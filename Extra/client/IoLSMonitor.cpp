#include "IoLSMonitor.h"
#include <json.hpp>
#include <fstream>
#include <regex>
#include <chrono>
using json = nlohmann::json;

IoLSMonitor::IoLSMonitor(const std::string &serverUrl)
    : m_serverUrl(serverUrl), m_running(false), m_connected(false)
{
  // -- fill the list of variables 
}

IoLSMonitor::~IoLSMonitor()
{
  if (m_running)
  {
    m_running = false;
    if (m_monitor_thread.joinable())
    {
      m_monitor_thread.join();
    }
  }
}

bool IoLSMonitor::connect()
{
  try
  {
    m_connected = m_client.connect(m_serverUrl);
    if (m_connected)
    {
      m_running = true;
      m_monitor_thread = std::thread(&IoLSMonitor::monitor_loop, this);
      return true;
    }
    else
    {
      m_feedback_messages.push_back("Failed to connect to server.");
      return false;
    }
  }
  catch (const std::exception &e)
  {
    m_feedback_messages.push_back("Failed to connect to server : " + std::string(e.what()));
    return false;
  }
}

void IoLSMonitor::disconnect()
{
  m_running = false;
  if (m_monitor_thread.joinable())
  {
    m_monitor_thread.join();
  }
  m_client.disconnect();
  m_connected = false;
  m_feedback_messages.push_back("Disconnected from server.");
}

bool IoLSMonitor::is_connected() const
{
  return m_connected;
}

bool IoLSMonitor::config(std::string location, json &response)
{
  try
  {
    // Check that the client is connected
    if (!m_client.is_connected())
    {
      response["messages"].push_back("Client is not connected.");
      return false;
    }
    // Initialize the response JSON with a messages array
    response["messages"] = json::array();

    std::ifstream file(location);
    if (!file.is_open())
    {
      response["messages"].push_back("Failed to open file at [" + location + "]");
      return false;
    }

    json jconf = json::parse(file); // parse the file
    response["messages"].push_back("Configuration file loaded successfully.");
    file.close();

    // now we have the json object, we can send it to the server
    std::string node_base = "LS1";
    std::string method = "LS1.load_config";

    // if the json config makes sense, need to put it into a UA_Variant
    UA_Variant configVariant;
    UA_Variant_init(&configVariant);
    std::string configString = jconf.dump();
    UA_String uaConfigString = UA_STRING_ALLOC(configString.c_str());
    UA_Variant_setScalarCopy(&configVariant, &uaConfigString, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_clear(&uaConfigString);
    std::vector<UA_Variant> outputArguments;

    try
    {
      m_client.call_method(node_base, method, {configVariant}, outputArguments);
    }
    catch(const std::exception& e)
    {
      // the method somehow may have failed. 
      // append the feedback to the ongoing response
    }
    auto feedback_messages = m_client.get_feedback_messages();
    for (const auto &msg : feedback_messages)
    {
      response["messages"].push_back(msg);
    }

    std::string reply;
    if (!outputArguments.empty() && UA_Variant_hasScalarType(&outputArguments[0], &UA_TYPES[UA_TYPES_STRING]))
    {
      UA_String *uaResponse = static_cast<UA_String *>(outputArguments[0].data);
      reply = std::string(reinterpret_cast<char *>(uaResponse->data), uaResponse->length);
      //response["messages"].push_back("Reply from server: " + reply);
    }
    else
    {
      response["messages"].push_back("No valid response received from server.");
      // Do not clear the response
    }

    // Parse the server's reply and merge the messages and other keys
    json server_response = json::parse(reply);
    if (server_response.contains("messages"))
    {
      for (const auto &msg : server_response["messages"])
      {
        response["messages"].push_back(msg);
      }
    }
    for (auto it = server_response.begin(); it != server_response.end(); ++it)
    {
      if (it.key() != "messages")
      {
        response[it.key()] = it.value();
      }
    }

    // Clean up the UA_Variant
    UA_Variant_clear(&configVariant);
    for (auto &output : outputArguments)
    {
      UA_Variant_clear(&output);
    }

    return true;
  }
  catch (const json::exception &e)
  {
    response["messages"].push_back("JSON exception in config: " + std::string(e.what()));
    return false;
  }
  catch (const std::exception &e)
  {
    response["messages"].push_back("Exception in config: " + std::string(e.what()));
    return false;
  }
  catch (...)
  {
    response["messages"].push_back("Unknown exception in config");
    return false;
  }
}

void IoLSMonitor::monitor_server()
{
  try
  {
    while (m_running)
    {
      for (auto &item : m_monitored_vars)
      {
        try
        {
          std::unique_ptr<UA_Variant, void(*)(UA_Variant*)> value(new UA_Variant, [](UA_Variant* v) { UA_Variant_clear(v); delete v; });
          UA_Variant_init(value.get());
          m_client.read_variable(item.first, *value);
          // grab the feedback...and forget it
          m_client.get_feedback_messages();
          if (UA_Variant_hasScalarType(value.get(), &UA_TYPES[UA_TYPES_STRING]))
          {
            UA_String *uaString = static_cast<UA_String *>(value->data);
            m_monitored_items[item.first] = std::string(reinterpret_cast<char *>(uaString->data), uaString->length);
          }
          else if (UA_Variant_hasScalarType(value.get(), &UA_TYPES[UA_TYPES_INT32]))
          {
            m_monitored_items[item.first] = *static_cast<UA_Int32 *>(value->data);
          }
          else if (UA_Variant_hasScalarType(value.get(), &UA_TYPES[UA_TYPES_DOUBLE]))
          {
            m_monitored_items[item.first] = *static_cast<UA_Double *>(value->data);
          }
        }
        catch (const std::exception &e)
        {
          // don't print anything here, just silently ignore the call
          m_client.get_feedback_messages();
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
  catch (const std::exception &e)
  {
    m_feedback_messages.push_back(std::string("Exception in monitor_server: ") + e.what());
  }
  catch (...)
  {
    m_feedback_messages.push_back("Unknown exception in monitor_server");
  }
}

void IoLSMonitor::monitor_loop()
{
  monitor_server();
}

bool IoLSMonitor::move_to_position(const std::string &position, const std::string &approach, json &response)
{
  try
  {
    // Check that the client is connected
    if (!m_client.is_connected())
    {
      response["messages"].push_back("Client is not connected.");
      return false;
    }
    // Initialize the response JSON with a messages array
    response["messages"] = json::array();

    // Check that the position input has the format [x,y,z] with x, y, and z being integers
    std::regex position_regex(R"(\[\s*(-?\d+),\s*(-?\d+),\s*(-?\d+)\s*\])");
    std::smatch position_match;
    if (!std::regex_match(position, position_match, position_regex))
    {
      response["messages"].push_back("Invalid position format: " + position);
      return false;
    }

    // Check that the approach input is a string with three characters and all of them are one of 'u', 'd', '-'
    if (approach.size() != 3 || !std::all_of(approach.begin(), approach.end(), [](char c)
                                             { return c == 'u' || c == 'd' || c == '-'; }))
    {
      response["messages"].push_back("Invalid approach format: " + approach);
      return false;
    }

    // Convert the two input strings into a JSON variable
    json jrequest;
    jrequest["target"] = json::array({std::stoi(position_match[1].str()), std::stoi(position_match[2].str()), std::stoi(position_match[3].str())});
    jrequest["approach"] = approach;

    // Convert the JSON variable to a UA_Variant
    UA_Variant requestVariant;
    UA_Variant_init(&requestVariant);
    std::string requestString = jrequest.dump();
    UA_String uaRequestString = UA_STRING_ALLOC(requestString.c_str());
    UA_Variant_setScalarCopy(&requestVariant, &uaRequestString, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_clear(&uaRequestString);

    // Call the method under node "LS1.move_to_pos"
    std::vector<UA_Variant> outputArguments;
    try
    {
      m_client.call_method("LS1", "LS1.move_to_pos", {requestVariant}, outputArguments);
    }
    catch (const std::exception &e)
    {
      response["messages"].push_back("Exception in move_to_position: " + std::string(e.what()));
      return false;
    }
    auto feedback_messages = m_client.get_feedback_messages();
    for (const auto &msg : feedback_messages)
    {
      response["messages"].push_back(msg);
    }

    m_client.call_method("LS1", "LS1.move_to_pos", {requestVariant}, outputArguments);

    // Convert the single output argument into a string and parse it into the response JSON variable
    std::string reply;
    if (!outputArguments.empty() && UA_Variant_hasScalarType(&outputArguments[0], &UA_TYPES[UA_TYPES_STRING]))
    {
      UA_String *uaResponse = static_cast<UA_String *>(outputArguments[0].data);
      std::string responseString(reinterpret_cast<char *>(uaResponse->data), uaResponse->length);
      reply = json::parse(responseString);
    }
    else
    {
      response["messages"].push_back("No valid response received from server.");
    }

    // Parse the server's reply and merge the messages and other keys
    json server_response = json::parse(reply);
    if (server_response.contains("messages"))
    {
      for (const auto &msg : server_response["messages"])
      {
        response["messages"].push_back(msg);
      }
    }
    for (auto it = server_response.begin(); it != server_response.end(); ++it)
    {
      if (it.key() != "messages")
      {
        response[it.key()] = it.value();
      }
    }

    // Clean up the UA_Variant
    UA_Variant_clear(&requestVariant);
    for (auto &output : outputArguments)
    {
      UA_Variant_clear(&output);
    }

    return true;
  }
  catch (const json::exception &e)
  {
    response["messages"].push_back("JSON exception in move_to_position: " + std::string(e.what()));
    return false;
  }
  catch (const std::exception &e)
  {
    response["messages"].push_back("Exception in move_to_position: " + std::string(e.what()));
    return false;
  }
  catch (...)
  {
    response["messages"].push_back("Unknown exception in move_to_position");
    return false;
  }
}

bool IoLSMonitor::fire_at_position(const std::string position, const uint32_t num_shots, json &response)
{
  try
  {
    // Check that the client is connected
    if (!m_client.is_connected())
    {
      response["messages"].push_back("Client is not connected.");
      return false;
    }
    // Initialize the response JSON with a messages array
    response["messages"] = json::array();

    // Check that the position input has the format [x,y,z], with x, y, and z being integer values
    std::regex position_regex(R"(\[\s*(-?\d+),\s*(-?\d+),\s*(-?\d+)\s*\])");
    std::smatch position_match;
    if (!std::regex_match(position, position_match, position_regex))
    {
      response["messages"].push_back("Invalid position format: " + position);
      return false;
    }

    // Check that num_shots is a value above 0
    if (num_shots <= 0)
    {
      response["messages"].push_back("Invalid number of shots: " + std::to_string(num_shots));
      return false;
    }

    // Convert the two input strings into a JSON variable
    json jrequest;
    jrequest["target"] = json::array({std::stoi(position_match[1].str()), std::stoi(position_match[2].str()), std::stoi(position_match[3].str())});
    jrequest["num_pulses"] = num_shots;
    jrequest["lbls"] = false;

    // Convert the JSON variable to a UA_Variant
    UA_Variant requestVariant;
    UA_Variant_init(&requestVariant);
    std::string requestString = jrequest.dump();
    UA_String uaRequestString = UA_STRING_ALLOC(requestString.c_str());
    UA_Variant_setScalarCopy(&requestVariant, &uaRequestString, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_clear(&uaRequestString);

    // Call the method under node "LS1.fire_at_position"
    std::vector<UA_Variant> outputArguments;
    try
    {
      m_client.call_method("LS1", "LS1.fire_at_position", {requestVariant}, outputArguments);
    }
    catch (const std::exception &e)
    {
      response["messages"].push_back("Exception in fire_at_position: " + std::string(e.what()));
    }
    // feed any feedback messages into the list
    auto feedback_messages = m_client.get_feedback_messages();
    for (const auto &msg : feedback_messages)
    {
      response["messages"].push_back(msg);
    }

    // Convert the single output argument into a string and parse it into the response JSON variable
    if (!outputArguments.empty() && UA_Variant_hasScalarType(&outputArguments[0], &UA_TYPES[UA_TYPES_STRING]))
    {
      UA_String *uaResponse = static_cast<UA_String *>(outputArguments[0].data);
      std::string responseString(reinterpret_cast<char *>(uaResponse->data), uaResponse->length);
      json server_response = json::parse(responseString);

      // Merge the messages from the server response into the existing response
      if (server_response.contains("messages"))
      {
      for (const auto &msg : server_response["messages"])
      {
        response["messages"].push_back(msg);
      }
      }

      // Add any missing keys from the server response to the existing response
      for (auto it = server_response.begin(); it != server_response.end(); ++it)
      {
      if (it.key() != "messages")
      {
        response[it.key()] = it.value();
      }
      }
    }
    else
    {
      response["messages"].push_back("No valid response received from server.");
      // Clean up the UA_Variant
      UA_Variant_clear(&requestVariant);
      for (auto &output : outputArguments)
      {
        UA_Variant_clear(&output);
      }

      return false;
    }

    // Clean up the UA_Variant
    UA_Variant_clear(&requestVariant);
    for (auto &output : outputArguments)
    {
      UA_Variant_clear(&output);
    }

    return true;
  }
  catch (const json::exception &e)
  {
    response["messages"].push_back("JSON exception in fire_at_position: " + std::string(e.what()));
    return false;
  }
  catch (const std::exception &e)
  {
    response["messages"].push_back("Exception in fire_at_position: " + std::string(e.what()));
    return false;
  }
  catch (...)
  {
    response["messages"].push_back("Unknown exception in fire_at_position");
    return false;
  }
}

bool IoLSMonitor::fire_segment(const std::string start_position, const std::string end_position, json &response)
{
  try
  {
    // Check that the client is connected
    if (!m_client.is_connected())
    {
      response["messages"].push_back("Client is not connected.");
      return false;
    }
    // Initialize the response JSON with a messages array
    response["messages"] = json::array();

    // Check that the start_position and end_position inputs have the format [x,y,z], with x, y, and z being integer values
    std::regex position_regex(R"(\[\s*(-?\d+),\s*(-?\d+),\s*(-?\d+)\s*\])");
    std::smatch start_position_match, end_position_match;
    if (!std::regex_match(start_position, start_position_match, position_regex))
    {
      response["messages"].push_back("Invalid start position format: " + start_position);
      return false;
    }
    if (!std::regex_match(end_position, end_position_match, position_regex))
    {
      response["messages"].push_back("Invalid end position format: " + end_position);
      return false;
    }

    // Convert the input strings into a JSON variable
    json jrequest;
    jrequest["start_position"] = json::array({std::stoi(start_position_match[1].str()), std::stoi(start_position_match[2].str()), std::stoi(start_position_match[3].str())});
    jrequest["end_position"] = json::array({std::stoi(end_position_match[1].str()), std::stoi(end_position_match[2].str()), std::stoi(end_position_match[3].str())});
    jrequest["lbls"] = false;

    // Convert the JSON variable to a UA_Variant
    UA_Variant requestVariant;
    UA_Variant_init(&requestVariant);
    std::string requestString = jrequest.dump();
    UA_String uaRequestString = UA_STRING_ALLOC(requestString.c_str());
    UA_Variant_setScalarCopy(&requestVariant, &uaRequestString, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_clear(&uaRequestString);

    // Call the method under node "LS1.fire_segment"
    std::vector<UA_Variant> outputArguments;
    try
    {
      m_client.call_method("LS1", "LS1.fire_segment", {requestVariant}, outputArguments);
    }
    catch (const std::exception &e)
    {
      response["messages"].push_back("Exception in fire_segment: " + std::string(e.what()));
    }
    // feed any feedback messages into the list
    auto feedback_messages = m_client.get_feedback_messages();
    for (const auto &msg : feedback_messages)
    {
      response["messages"].push_back(msg);
    }

    // Convert the single output argument into a string and parse it into the response JSON variable
    std::string reply;
    if (!outputArguments.empty() && UA_Variant_hasScalarType(&outputArguments[0], &UA_TYPES[UA_TYPES_STRING]))
    {
      UA_String *uaResponse = static_cast<UA_String *>(outputArguments[0].data);
      std::string responseString(reinterpret_cast<char *>(uaResponse->data), uaResponse->length);

      // Merge the messages from the server response into the existing response
      json server_response = json::parse(responseString);
      if (server_response.contains("messages"))
      {
        for (const auto &msg : server_response["messages"])
        {
          response["messages"].push_back(msg);
        }
      }
      // Add any missing keys from the server response to the existing response
      for (auto it = server_response.begin(); it != server_response.end(); ++it)
      {
        if (it.key() != "messages")
        {
          response[it.key()] = it.value();
        }
      }
    }
    else
    {
      response["messages"].push_back("No valid response received from server.");
      // Clean up the UA_Variant
      UA_Variant_clear(&requestVariant);
      for (auto &output : outputArguments)
      {
        UA_Variant_clear(&output);
      }

      return false;
    }

    // Clean up the UA_Variant
    UA_Variant_clear(&requestVariant);
    for (auto &output : outputArguments)
    {
      UA_Variant_clear(&output);
    }

    return true;
  }
  catch (const json::exception &e)
  {
    response["messages"].push_back("JSON exception in fire_segment: " + std::string(e.what()));
    return false;
  }
  catch (const std::exception &e)
  {
    response["messages"].push_back("Exception in fire_segment: " + std::string(e.what()));
    return false;
  }
  catch (...)
  {
    response["messages"].push_back("Unknown exception in fire_segment");
    return false;
  }
}

bool IoLSMonitor::execute_scan(const std::string run_plan, json &response)
{
  try
  {
    // Check that the client is connected
    if (!m_client.is_connected())
    {
      response["messages"].push_back("Client is not connected.");
      return false;
    }
    // Initialize the response JSON with a messages array
    response["messages"] = json::array();

    // Check that the client is connected
    if (!m_client.is_connected())
    {
      response["messages"].push_back("Client is not connected.");
      return false;
    }

    // Parse the run_plan to JSON
    json jrun_plan;
    try
    {
      jrun_plan = json::parse(run_plan);
    }
    catch (const json::exception &e)
    {
      response["messages"].push_back("Invalid JSON format: " + std::string(e.what()) + " | Input: " + run_plan);
      return false;
    }

    // Check that the run_plan variable is valid JSON with the required structure
    if (!jrun_plan.contains("scan_plan") || !jrun_plan["scan_plan"].is_array())
    {
      response["messages"].push_back("Invalid scan plan structure. Expected a JSON object with a 'scan_plan' array.");
      return false;
    }

    std::regex position_regex(R"(\[\s*(-?\d+),\s*(-?\d+),\s*(-?\d+)\s*\])");
    for (const auto &item : jrun_plan["scan_plan"])
    {
      if (!item.contains("start") || !item.contains("end"))
      {
        response["messages"].push_back("Missing 'start' or 'end' key in scan plan item: " + item.dump());
        return false;
      }

      std::string start_str = item["start"].dump();
      std::string end_str = item["end"].dump();
      std::smatch start_match, end_match;
      if (!std::regex_match(start_str, start_match, position_regex) || !std::regex_match(end_str, end_match, position_regex))
      {
        response["messages"].push_back("Invalid position format in scan plan item. Expected format [x,y,z]. Item: " + item.dump());
        return false;
      }
    }

    // Convert the JSON variable to a UA_Variant
    UA_Variant requestVariant;
    UA_Variant_init(&requestVariant);
    std::string requestString = jrun_plan.dump();
    UA_String uaRequestString = UA_STRING_ALLOC(requestString.c_str());
    UA_Variant_setScalarCopy(&requestVariant, &uaRequestString, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_clear(&uaRequestString);

    // Call the method under node "LS1.execute_scan"
    std::vector<UA_Variant> outputArguments;
    try
    {
      m_client.call_method("LS1", "LS1.execute_scan", {requestVariant}, outputArguments);
    }
    catch (const std::exception &e)
    {
      response["messages"].push_back("Exception in fire_segment: " + std::string(e.what()));
    }
    // feed any feedback messages into the list
    auto feedback_messages = m_client.get_feedback_messages();
    for (const auto &msg : feedback_messages)
    {
      response["messages"].push_back(msg);
    }

    // Convert the single output argument into a string and parse it into the response JSON variable
    if (!outputArguments.empty() && UA_Variant_hasScalarType(&outputArguments[0], &UA_TYPES[UA_TYPES_STRING]))
    {
      UA_String *uaResponse = static_cast<UA_String *>(outputArguments[0].data);
      std::string responseString(reinterpret_cast<char *>(uaResponse->data), uaResponse->length);

      // Merge the messages from the server response into the existing response
      json server_response = json::parse(responseString);
      if (server_response.contains("messages"))
      {
        for (const auto &msg : server_response["messages"])
        {
          response["messages"].push_back(msg);
        }
      }
      // Add any missing keys from the server response to the existing response
      for (auto it = server_response.begin(); it != server_response.end(); ++it)
      {
        if (it.key() != "messages")
        {
          response[it.key()] = it.value();
        }
      }
    }
    else
    {
      response["messages"].push_back("No valid response received from server.");
      // Clean up the UA_Variant
      UA_Variant_clear(&requestVariant);
      for (auto &output : outputArguments)
      {
        UA_Variant_clear(&output);
      }

      return false;
    }

    // Clean up the UA_Variant
    UA_Variant_clear(&requestVariant);
    for (auto &output : outputArguments)
    {
      UA_Variant_clear(&output);
    }

    return true;
  }
  catch (const json::exception &e)
  {
    response["messages"].push_back("JSON exception in execute_scan: " + std::string(e.what()));
    return false;
  }
  catch (const std::exception &e)
  {
    response["messages"].push_back("Exception in execute_scan: " + std::string(e.what()));
    return false;
  }
  catch (...)
  {
    response["messages"].push_back("Unknown exception in execute_scan");
    return false;
  }
}

bool IoLSMonitor::exec_method_simple(const std::string &method_node, json &response)
{
  try
  {
    // Check that the client is connected
    if (!m_client.is_connected())
    {
      response["messages"].push_back("Client is not connected.");
      return false;
    }
    // Initialize the response JSON with a messages array
    response["messages"] = json::array();

    // Check that the client is connected
    if (!m_client.is_connected())
    {
      response["messages"].push_back("Client is not connected.");
      return false;
    }

    // Prepare the request variant (no specific input needed for simple methods)
    UA_Variant requestVariant;
    UA_Variant_init(&requestVariant);

    // Call the method under the specified node
    std::vector<UA_Variant> outputArguments;
    try
    {
      m_client.call_method("LS1", method_node, {requestVariant}, outputArguments);
    }
    catch (const std::exception &e)
    {
      response["messages"].push_back("Exception in fire_segment: " + std::string(e.what()));
    }
    // feed any feedback messages into the list
    auto feedback_messages = m_client.get_feedback_messages();
    for (const auto &msg : feedback_messages)
    {
      response["messages"].push_back(msg);
    }

    // Convert the single output argument into a string and parse it into the response JSON variable
    if (!outputArguments.empty() && UA_Variant_hasScalarType(&outputArguments[0], &UA_TYPES[UA_TYPES_STRING]))
    {
      UA_String *uaResponse = static_cast<UA_String *>(outputArguments[0].data);
      std::string responseString(reinterpret_cast<char *>(uaResponse->data), uaResponse->length);

      // Merge the messages from the server response into the existing response
      json server_response = json::parse(responseString);
      if (server_response.contains("messages"))
      {
        for (const auto &msg : server_response["messages"])
        {
          response["messages"].push_back(msg);
        }
      }
      // Add any missing keys from the server response to the existing response
      for (auto it = server_response.begin(); it != server_response.end(); ++it)
      {
        if (it.key() != "messages")
        {
          response[it.key()] = it.value();
        }
      }
    }
    else
    {
      response["messages"].push_back("No valid response received from server.");
      // Clean up the UA_Variant
      UA_Variant_clear(&requestVariant);
      for (auto &output : outputArguments)
      {
        UA_Variant_clear(&output);
      }

      return false;
    }

    // Clean up the UA_Variant
    UA_Variant_clear(&requestVariant);
    for (auto &output : outputArguments)
    {
      UA_Variant_clear(&output);
    }

    return true;
  }
  catch (const json::exception &e)
  {
    response["messages"].push_back("JSON exception in " + method_node + ": " + std::string(e.what()));
    return false;
  }
  catch (const std::exception &e)
  {
    response["messages"].push_back("Exception in " + method_node + ": " + std::string(e.what()));
    return false;
  }
  catch (...)
  {
    response["messages"].push_back("Unknown exception in " + method_node);
    return false;
  }
}

bool IoLSMonitor::pause(json &response)
{
  return exec_method_simple("LS1.pause", response);
}

bool IoLSMonitor::standby(json &response)
{
  return exec_method_simple("LS1.standby", response);
}

bool IoLSMonitor::resume(json &response)
{
  return exec_method_simple("LS1.resume", response);
}

bool IoLSMonitor::warmup(json &response)
{
  return exec_method_simple("LS1.warmup", response);
}

bool IoLSMonitor::shutdown(json &response)
{
  return exec_method_simple("LS1.shutdown", response);
}

bool IoLSMonitor::stop(json &response)
{
  return exec_method_simple("LS1.stop", response);
}
void IoLSMonitor::update_monitored_item(const std::string &key, const iols_opc_variant_t &value)
{
  m_monitored_items[key] = value;
}

void IoLSMonitor::set_monitored_vars(const std::vector<std::string> &var_names)
{
  for (const auto &var_name : var_names)
  {
    UA_Variant value;
    UA_Variant_init(&value);
    m_monitored_vars[var_name] = value;
  }
}
std::deque<std::string> IoLSMonitor::get_feedback_messages()
{
  std::deque<std::string> messages = std::move(m_feedback_messages);
  m_feedback_messages.clear();
  return messages;
}

bool IoLSMonitor::read_variable(const std::string &variable, UA_Variant &value)
{
  try
  {
    // Check that the client is connected
    if (!m_client.is_connected())
    {
      m_feedback_messages.push_back("Client is not connected.");
      return false;
    }

    // Read the variable
    m_client.read_variable(variable, value);

    // Get feedback messages from the client and append them to m_feedback_messages
    auto client_feedback = m_client.get_feedback_messages();
    m_feedback_messages.insert(m_feedback_messages.end(), client_feedback.begin(), client_feedback.end());

    m_feedback_messages.push_back("Successfully read variable: " + variable);
    return true;
  }
  catch (const std::exception &e)
  {
    m_feedback_messages.push_back(std::string("Exception in read_variable: ") + e.what());
    return false;
  }
  catch (...)
  {
    m_feedback_messages.push_back("Unknown exception in read_variable");
    return false;
  }
}