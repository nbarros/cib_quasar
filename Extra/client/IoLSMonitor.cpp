#include "IoLSMonitor.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

IoLSMonitor::IoLSMonitor()
    : m_serverUrl(""), m_running(false), m_connected(false)
{
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

bool IoLSMonitor::connect(const std::string &serverUrl, FeedbackManager &feedback)
{
  try
  {
    m_serverUrl = serverUrl;
    m_connected = m_client.connect(m_serverUrl, feedback);
    if (m_connected)
    {
      m_running = true;
      m_monitor_thread = std::thread(&IoLSMonitor::monitor_loop, this);
      feedback.add_message(Severity::INFO, "Connected to server and started monitoring.");
      return true;
    }
    else
    {
      feedback.add_message(Severity::ERROR, "Failed to connect to server.");
      return false;
    }
  }
  catch (const std::exception &e)
  {
    feedback.add_message(Severity::ERROR, "Failed to connect to server: " + std::string(e.what()));
    return false;
  }
}

void IoLSMonitor::disconnect(FeedbackManager &feedback)
{
  m_running = false;
  if (m_monitor_thread.joinable())
  {
    m_monitor_thread.join();
  }
  m_client.disconnect(feedback);
  m_connected = false;
  feedback.add_message(Severity::INFO, "Disconnected from server.");
}

bool IoLSMonitor::is_connected() const
{
  return m_connected;
}

void IoLSMonitor::monitor_loop()
{
  while (m_running.load())
  {
    monitor_server();
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Adjust the sleep duration as needed
  }
}

void IoLSMonitor::monitor_server()
{
  for (auto &item : m_monitored_vars)
  {
    try
    {
      UA_Variant value;
      UA_Variant_init(&value);
      FeedbackManager feedback;
      m_client.read_variable(item.first, value, feedback);

      if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_STRING]))
      {
        UA_String *uaString = static_cast<UA_String *>(value.data);
        update_monitored_item(item.first, std::string(reinterpret_cast<char *>(uaString->data), uaString->length));
      }
      else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT32]))
      {
        update_monitored_item(item.first, *static_cast<UA_Int32 *>(value.data));
      }
      else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_UINT32]))
      {
        update_monitored_item(item.first, *static_cast<UA_UInt32 *>(value.data));
      }
      else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT16]))
      {
        update_monitored_item(item.first, *static_cast<UA_Int16 *>(value.data));
      }
      else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_UINT16]))
      {
        update_monitored_item(item.first, *static_cast<UA_UInt16 *>(value.data));
      }
      else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_DOUBLE]))
      {
        update_monitored_item(item.first, *static_cast<UA_Double *>(value.data));
      }
      else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_FLOAT]))
      {
        update_monitored_item(item.first, *static_cast<UA_Float *>(value.data));
      }
      else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_BOOLEAN]))
      {
        update_monitored_item(item.first, *static_cast<UA_Boolean *>(value.data));
      }

      UA_Variant_clear(&value);
    }
    catch (const std::exception &e)
    {
      FeedbackManager feedback;
      feedback.add_message(Severity::ERROR, std::string("Exception in monitor_server: ") + e.what());
    }
    catch (...)
    {
      FeedbackManager feedback;
      feedback.add_message(Severity::ERROR, "Unknown exception in monitor_server");
    }
  }
}

bool IoLSMonitor::config(const std::string &location, FeedbackManager &feedback)
{
  try
  {
    // Check that the client is connected
    if (!m_client.is_connected())
    {
      feedback.add_message(Severity::ERROR, "Client is not connected.");
      return false;
    }

    std::ifstream file(location);
    if (!file.is_open())
    {
      feedback.add_message(Severity::ERROR, "Failed to open file at [" + location + "]");
      return false;
    }

    json jconf = json::parse(file); // parse the file
    feedback.add_message(Severity::INFO, "Configuration file loaded successfully.");
    file.close();

    // now we have the json object, we can send it to the server
    std::string node_base = "LS1";
    std::string method = "LS1.load_config";

    // if the json config makes sense, need to put it into a UA_Variant
    UA_Variant configVariant;
    UA_Variant_init(&configVariant);
    //std::string configString = jconf.dump();
    UA_String uaConfigString = UA_String_fromChars(jconf.dump().c_str()); //UA_STRING_ALLOC(configString.c_str());
    UA_Variant_setScalarCopy(&configVariant, &uaConfigString, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_clear(&uaConfigString);
    std::vector<UA_Variant> outputArguments;
    try
    {
      m_client.call_method(node_base, method, {configVariant}, outputArguments, feedback);
    }
    catch (const std::exception &e)
    {
      feedback.add_message(Severity::ERROR, "Exception in call_method: " + std::string(e.what()));
      return false;
    }

    std::string reply;
    if (!outputArguments.empty() && UA_Variant_hasScalarType(&outputArguments[0], &UA_TYPES[UA_TYPES_STRING]))
    {
      UA_String *uaResponse = static_cast<UA_String *>(outputArguments[0].data);
      reply = std::string(reinterpret_cast<char *>(uaResponse->data), uaResponse->length);
      //feedback.add_message(Severity::INFO, "Reply from server: " + reply);
    }
    else
    {
      feedback.add_message(Severity::ERROR, "No valid response received from server.");
      return false;
    }

    json server_response = json::parse(reply);
    if (server_response.contains("messages"))
    {
      for (const auto &msg : server_response["messages"])
      {
        feedback.add_message(Severity::REPORT, msg);
      }
      if (server_response.contains("statuscode"))
      {
        feedback.set_global_status(static_cast<UA_StatusCode>(server_response["statuscode"].get<int>()));
      }
    }

    UA_Variant_clear(&configVariant);
    // for (auto &output : outputArguments)
    // {
    //   UA_Variant_clear(&output);
    // }

    return true;
  }
  catch (const json::exception &e)
  {
    feedback.add_message(Severity::ERROR, "JSON exception in config: " + std::string(e.what()));
    return false;
  }
  catch (const std::exception &e)
  {
    feedback.add_message(Severity::ERROR, "Exception in config: " + std::string(e.what()));
    return false;
  }
  catch (...)
  {
    feedback.add_message(Severity::ERROR, "Unknown exception in config");
    return false;
  }
}

bool IoLSMonitor::move_to_position(const std::string &position, const std::string &approach, FeedbackManager &feedback)
{
  try
  {
    // Check that the client is connected
    if (!m_client.is_connected())
    {
      feedback.add_message(Severity::ERROR, "Client is not connected");
      return false;
    }
    // Check that the position input has the format [x,y,z] with x, y, and z being integers
    std::regex position_regex(R"(\[\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*\])");
    std::smatch position_match;
    if (!std::regex_match(position, position_match, position_regex))
    {
      feedback.add_message(Severity::ERROR, "Invalid position format: " + position);
      return false;
    }

    if (approach.size() != 3 || !std::all_of(approach.begin(), approach.end(), [](char c)
                                             { return c == 'u' || c == 'd' || c == '-'; }))
    {
      feedback.add_message(Severity::ERROR, "Invalid approach format: " + approach);
      return false;
    }

    json jrequest;
    jrequest["target"] = json::array({std::stoi(position_match[1].str()), std::stoi(position_match[2].str()), std::stoi(position_match[3].str())});
    jrequest["approach"] = approach;

    UA_Variant requestVariant;
    UA_Variant_init(&requestVariant);
    std::string requestString = jrequest.dump();
    UA_String uaRequestString = UA_STRING_ALLOC(requestString.c_str());
    UA_Variant_setScalarCopy(&requestVariant, &uaRequestString, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_clear(&uaRequestString);

    std::vector<UA_Variant> outputArguments;
    m_client.call_method("LS1", "LS1.move_to_pos", {requestVariant}, outputArguments, feedback);

    // Convert the single output argument into a string and parse it into the response JSON variable
    if (!outputArguments.empty() && UA_Variant_hasScalarType(&outputArguments[0], &UA_TYPES[UA_TYPES_STRING]))
    {
      UA_String *uaResponse = static_cast<UA_String *>(outputArguments[0].data);
      std::string responseString(reinterpret_cast<char *>(uaResponse->data), uaResponse->length);
      json server_response = json::parse(responseString);

      if (server_response.contains("messages"))
      {
        for (const auto &msg : server_response["messages"])
        {
          feedback.add_message(Severity::REPORT, msg);
        }
      }
      if (server_response.contains("statuscode") && (server_response["statuscode"].get<int>() != UA_STATUSCODE_GOOD))
      {
        feedback.set_global_status(static_cast<UA_StatusCode>(server_response["statuscode"].get<int>()));
      }
    }
    else
    {
      feedback.add_message(Severity::ERROR, "No valid response received from server.");
      feedback.set_global_status(UA_STATUSCODE_BADUNKNOWNRESPONSE);
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
    feedback.add_message(Severity::ERROR, "JSON exception in move_to_position: " + std::string(e.what()));
    return false;
  }
  catch (const std::exception &e)
  {
    feedback.add_message(Severity::ERROR, "Exception in move_to_position: " + std::string(e.what()));
    return false;
  }
  catch (...)
  {
    feedback.add_message(Severity::ERROR, "Unknown exception in move_to_position");
    return false;
  }
}

bool IoLSMonitor::fire_at_position(const std::string &position, const std::string &approach, const uint32_t num_shots, FeedbackManager &feedback)
{
  try
  {
    // Check that the client is connected
    if (!m_client.is_connected())
    {
      feedback.add_message(Severity::ERROR, "Client is not connected.");
      return false;
    }

    // Check that the position input has the format [x,y,z], with x, y, and z being integer values
    std::regex position_regex(R"(\[\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*\])");
    std::smatch position_match;
    if (!std::regex_match(position, position_match, position_regex))
    {
      feedback.add_message(Severity::ERROR, "Invalid position format: " + position);
      return false;
    }

    if (approach.size() != 3 || !std::all_of(approach.begin(), approach.end(), [](char c)
                                             { return c == 'u' || c == 'd' || c == '-'; }))
    {
      feedback.add_message(Severity::ERROR, "Invalid approach format: " + approach);
      return false;
    }

    // Check that num_shots is a value above 0
    if (num_shots <= 0)
    {
      feedback.add_message(Severity::ERROR, "Invalid number of shots: " + std::to_string(num_shots));
      return false;
    }

    // Convert the two input strings into a JSON variable
    json jrequest;
    jrequest["target"] = json::array({std::stoi(position_match[1].str()), std::stoi(position_match[2].str()), std::stoi(position_match[3].str())});
    jrequest["approach"] = approach;
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
      m_client.call_method("LS1", "LS1.fire_at_position", {requestVariant}, outputArguments, feedback);
    }
    catch (const std::exception &e)
    {
    }

    // Convert the single output argument into a string and parse it into the response JSON variable
    if (!outputArguments.empty() && UA_Variant_hasScalarType(&outputArguments[0], &UA_TYPES[UA_TYPES_STRING]))
    {
      UA_String *uaResponse = static_cast<UA_String *>(outputArguments[0].data);
      std::string responseString(reinterpret_cast<char *>(uaResponse->data), uaResponse->length);
      json server_response = json::parse(responseString);

      // Merge the messages from the server response into the existing response
      Severity severity = Severity::REPORT;
      if (server_response.contains("messages"))
      {
        for (const auto &msg : server_response["messages"])
        {
          feedback.add_message(severity, msg);
        }
      }
      if (server_response.contains("statuscode"))
      {
        if (server_response["statuscode"].get<int>() != 0)
        {
          feedback.set_global_status(static_cast<UA_StatusCode>(server_response["statuscode"].get<int>()));
        }
      }
    }
    else
    {
      feedback.add_message(Severity::ERROR, "No valid response received from server.");
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
    feedback.add_message(Severity::ERROR, "JSON exception in fire_at_position: " + std::string(e.what()));
    return false;
  }
  catch (const std::exception &e)
  {
    feedback.add_message(Severity::ERROR, "Exception in fire_at_position: " + std::string(e.what()));
    return false;
  }
  catch (...)
  {
    feedback.add_message(Severity::ERROR, "Unknown exception in fire_at_position");
    return false;
  }
}

bool IoLSMonitor::fire_segment(const std::string &start_position, const std::string &end_position, FeedbackManager &feedback)
{
  try
  {
    // Check that the client is connected
    if (!m_client.is_connected())
    {
      feedback.add_message(Severity::ERROR, "Client is not connected.");
      return false;
    }
    // Check that the start_position and end_position inputs have the format [x,y,z], with x, y, and z being integer values
    std::regex position_regex(R"(\[\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*\])");
    std::smatch start_position_match, end_position_match;
    if (!std::regex_match(start_position, start_position_match, position_regex))
    {
      feedback.add_message(Severity::ERROR, "Invalid start position format: " + start_position);
      return false;
    }
    if (!std::regex_match(end_position, end_position_match, position_regex))
    {
      feedback.add_message(Severity::ERROR, "Invalid end position format: " + end_position);
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
      m_client.call_method("LS1", "LS1.fire_segment", {requestVariant}, outputArguments,feedback);
    }
    catch (const std::exception &e)
    {
      feedback.add_message(Severity::ERROR, "Exception in fire_segment: " + std::string(e.what()));
    }

    // Convert the single output argument into a string and parse it into the response JSON variable
    std::string reply;
    if (!outputArguments.empty() && UA_Variant_hasScalarType(&outputArguments[0], &UA_TYPES[UA_TYPES_STRING]))
    {
      UA_String *uaResponse = static_cast<UA_String *>(outputArguments[0].data);
      std::string responseString(reinterpret_cast<char *>(uaResponse->data), uaResponse->length);

      // Merge the messages from the server response into the existing response
      json server_response = json::parse(responseString);
      Severity severity = Severity::REPORT;
      if (server_response.contains("messages"))
      {
        for (const auto &msg : server_response["messages"])
        {
          feedback.add_message(severity, msg);
        }
      }
      if (server_response.contains("statuscode")  && (server_response["statuscode"].get<int>() != 0))

      {
        feedback.set_global_status(static_cast<UA_StatusCode>(server_response["statuscode"].get<int>()));
      }
    }
    else
    {
      feedback.add_message(Severity::ERROR,"No valid response received from server.");
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
    feedback.add_message(Severity::ERROR, "JSON exception in fire_segment: " + std::string(e.what()));
    return false;
  }
  catch (const std::exception &e)
  {
    feedback.add_message(Severity::ERROR, "Exception in fire_segment: " + std::string(e.what()));
    return false;
  }
  catch (...)
  {
    feedback.add_message(Severity::ERROR, "Unknown exception in fire_segment");
    return false;
  }
}

bool IoLSMonitor::execute_scan(const std::string &run_plan, FeedbackManager &feedback)
{
  try
  {
    // Check that the client is connected
    if (!m_client.is_connected())
    {
      feedback.add_message(Severity::ERROR, "Client is not connected.");
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
      feedback.add_message(Severity::ERROR, "Invalid JSON format: " + std::string(e.what()) + " | Input: " + run_plan);
      return false;
    }

    // Check that the run_plan variable is valid JSON with the required structure
    if (!jrun_plan.contains("scan_plan") || !jrun_plan["scan_plan"].is_array())
    {
      feedback.add_message(Severity::ERROR, "Invalid scan plan structure. Expected a JSON object with a 'scan_plan' array.");
      return false;
    }

    std::regex position_regex(R"(\[\s*(-?\d+),\s*(-?\d+),\s*(-?\d+)\s*\])");
    for (const auto &item : jrun_plan["scan_plan"])
    {
      if (!item.contains("start") || !item.contains("end"))
      {
        feedback.add_message(Severity::ERROR, "Missing 'start' or 'end' key in scan plan item: " + item.dump());
        return false;
      }

      std::string start_str = item["start"].dump();
      std::string end_str = item["end"].dump();
      std::smatch start_match, end_match;
      if (!std::regex_match(start_str, start_match, position_regex) || !std::regex_match(end_str, end_match, position_regex))
      {
        feedback.add_message(Severity::ERROR, "Invalid position format in scan plan item. Expected format [x,y,z]. Item: " + item.dump());
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
      m_client.call_method("LS1", "LS1.execute_scan", {requestVariant}, outputArguments, feedback);
    }
    catch (const std::exception &e)
    {
      feedback.add_message(Severity::ERROR, "Exception in fire_segment: " + std::string(e.what()));
    }

    // Convert the single output argument into a string and parse it into the response JSON variable
    if (!outputArguments.empty() && UA_Variant_hasScalarType(&outputArguments[0], &UA_TYPES[UA_TYPES_STRING]))
    {
      UA_String *uaResponse = static_cast<UA_String *>(outputArguments[0].data);
      std::string responseString(reinterpret_cast<char *>(uaResponse->data), uaResponse->length);

      // Merge the messages from the server response into the existing response
      json server_response = json::parse(responseString);
      Severity severity = Severity::REPORT;
      if (server_response.contains("messages"))
      {
        for (const auto &msg : server_response["messages"])
        {
          feedback.add_message(severity, msg);
        }
      }
      if (server_response.contains("statuscode") && (server_response["statuscode"].get<int>() != UA_STATUSCODE_GOOD))
      {
        feedback.set_global_status(static_cast<UA_StatusCode>(server_response["statuscode"].get<int>()));
      }
    }
    else
    {
      feedback.add_message(Severity::ERROR, "No valid response received from server.");
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
    feedback.add_message(Severity::ERROR, "JSON exception in execute_scan: " + std::string(e.what()));
    return false;
  }
  catch (const std::exception &e)
  {
    feedback.add_message(Severity::ERROR, "Exception in execute_scan: " + std::string(e.what()));
    return false;
  }
  catch (...)
  {
    feedback.add_message(Severity::ERROR, "Unknown exception in execute_scan");
    return false;
  }
}

bool IoLSMonitor::execute_grid_scan(const std::string &run_plan, FeedbackManager &feedback)
{
  try
  {
    // Check that the client is connected
    if (!m_client.is_connected())
    {
      feedback.add_message(Severity::ERROR, "Client is not connected.");
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
      feedback.add_message(Severity::ERROR, "Invalid JSON format: " + std::string(e.what()) + " | Input: " + run_plan);
      return false;
    }

    // Check that the run_plan variable is valid JSON with the required structure
    if (!jrun_plan.contains("center") || !jrun_plan["center"].is_array() || jrun_plan["center"].size() != 3)
    {
      feedback.add_message(Severity::ERROR, "Invalid or missing 'center' key. Expected an array of 3 elements.");
      return false;
    }

    if (!jrun_plan.contains("range") || !jrun_plan["range"].is_array() || jrun_plan["range"].size() != 3)
    {
      feedback.add_message(Severity::ERROR, "Invalid or missing 'range' key. Expected an array of 3 elements.");
      return false;
    }

    if (!jrun_plan.contains("step") || !jrun_plan["step"].is_array() || jrun_plan["step"].size() != 3)
    {
      feedback.add_message(Severity::ERROR, "Invalid or missing 'step' key. Expected an array of 3 elements.");
      return false;
    }

    if (!jrun_plan.contains("approach") || !jrun_plan["approach"].is_string() || jrun_plan["approach"].get<std::string>().size() != 3)
    {
      feedback.add_message(Severity::ERROR, "Invalid or missing 'approach' key. Expected a string of 3 characters.");
      return false;
    }

    if (!jrun_plan.contains("scan_axis") || !jrun_plan["scan_axis"].is_number_unsigned() || jrun_plan["scan_axis"].get<std::string>().size() > 2)
    {
      feedback.add_message(Severity::ERROR, "Invalid or missing 'scan_axis' key. Expected an unsigned integer < 2.");
      return false;
    }

    // Convert the JSON variable to a UA_Variant
    UA_Variant requestVariant;
    UA_Variant_init(&requestVariant);
    std::string requestString = jrun_plan.dump();
    UA_String uaRequestString = UA_STRING_ALLOC(requestString.c_str());
    UA_Variant_setScalarCopy(&requestVariant, &uaRequestString, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_clear(&uaRequestString);

    // Call the method under node "LS1.execute_grid_scan"
    std::vector<UA_Variant> outputArguments;
    try
    {
      m_client.call_method("LS1", "LS1.execute_grid_scan", {requestVariant}, outputArguments, feedback);
    }
    catch (const std::exception &e)
    {
      feedback.add_message(Severity::ERROR, "Exception in execute_grid_scan: " + std::string(e.what()));
      return false;
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
          feedback.add_message(Severity::REPORT, msg);
        }
      }
      if (server_response.contains("statuscode") && (server_response["statuscode"].get<int>() != UA_STATUSCODE_GOOD))
      {
        feedback.set_global_status(static_cast<UA_StatusCode>(server_response["statuscode"].get<int>()));
      }
    }

    feedback.add_message(Severity::INFO, "Grid scan executed successfully.");
    return true;
  }
  catch (const std::exception &e)
  {
    feedback.add_message(Severity::ERROR, "Exception in execute_grid_scan: " + std::string(e.what()));
    return false;
  }
  catch (...)
  {
    feedback.add_message(Severity::ERROR, "Unknown exception in execute_grid_scan");
    return false;
  }
}
bool IoLSMonitor::exec_method_simple(const std::string &method_node, FeedbackManager &feedback)
{
  try
  {
    // Check that the client is connected
    if (!m_client.is_connected())
    {
      feedback.add_message(Severity::ERROR, "Client is not connected.");
      return false;
    }

    // Prepare the request variant (no specific input needed for simple methods)
    // UA_Variant requestVariant;
    // UA_Variant_init(&requestVariant);

    // Call the method under the specified node
    std::vector<UA_Variant> outputArguments;
    try
    {
      m_client.call_method("LS1", method_node, {}, outputArguments, feedback);
    }
    catch (const std::exception &e)
    {
      feedback.add_message(Severity::ERROR, "Exception in exec_simple: " + std::string(e.what()));
    }

    // Convert the single output argument into a string and parse it into the response JSON variable
    if (!outputArguments.empty() && UA_Variant_hasScalarType(&outputArguments[0], &UA_TYPES[UA_TYPES_STRING]))
    {
      UA_String *uaResponse = static_cast<UA_String *>(outputArguments[0].data);
      std::string responseString(reinterpret_cast<char *>(uaResponse->data), uaResponse->length);

      // Merge the messages from the server response into the existing response
      json server_response = json::parse(responseString);
      Severity severity = Severity::REPORT;
      if (server_response.contains("messages"))
      {
        for (const auto &msg : server_response["messages"])
        {
          feedback.add_message(severity, msg);
        }
      }
      if (server_response.contains("statuscode") && (server_response["statuscode"].get<int>() != UA_STATUSCODE_GOOD))
      {
        feedback.set_global_status(static_cast<UA_StatusCode>(server_response["statuscode"].get<int>()));
      }
    }
    else
    {
      feedback.add_message(Severity::ERROR, "No valid response received from server.");
      // Clean up the UA_Variant
      // UA_Variant_clear(&requestVariant);
      for (auto &output : outputArguments)
      {
        UA_Variant_clear(&output);
      }

      return false;
    }

    // Clean up the UA_Variant
    // UA_Variant_clear(&requestVariant);
    for (auto &output : outputArguments)
    {
      UA_Variant_clear(&output);
    }

    return true;
  }
  catch (const json::exception &e)
  {
    feedback.add_message(Severity::ERROR, "JSON exception in " + method_node + ": " + std::string(e.what()));
    return false;
  }
  catch (const std::exception &e)
  {
    feedback.add_message(Severity::ERROR, "Exception in " + method_node + ": " + std::string(e.what()));
    return false;
  }
  catch (...)
  {
    feedback.add_message(Severity::ERROR, "Unknown exception in " + method_node);
    return false;
  }
}

bool IoLSMonitor::exec_method_arg(const std::string &method_node, const UA_Variant& val, FeedbackManager &feedback)
{
  try
  {
    // Check that the client is connected
    if (!m_client.is_connected())
    {
      feedback.add_message(Severity::ERROR, "Client is not connected.");
      return false;
    }

    // Call the method under the specified node
    std::vector<UA_Variant> outputArguments;
    try
    {
      // the node basically just a trim out of the method node
      std::string parent_node = method_node.substr(0, method_node.find_last_of('.')); 
      m_client.call_method(parent_node, method_node, {val}, outputArguments, feedback);
    }
    catch (const std::exception &e)
    {
      feedback.add_message(Severity::ERROR, "Exception in exec_arg: " + std::string(e.what()));
    }

    // Convert the single output argument into a string and parse it into the response JSON variable
    if (!outputArguments.empty() && UA_Variant_hasScalarType(&outputArguments[0], &UA_TYPES[UA_TYPES_STRING]))
    {
      UA_String *uaResponse = static_cast<UA_String *>(outputArguments[0].data);
      std::string responseString(reinterpret_cast<char *>(uaResponse->data), uaResponse->length);

      // Merge the messages from the server response into the existing response
      json server_response = json::parse(responseString);
      Severity severity = Severity::REPORT;
      if (server_response.contains("messages"))
      {
        for (const auto &msg : server_response["messages"])
        {
          feedback.add_message(severity, msg);
        }
      }
      if (server_response.contains("statuscode") && (server_response["statuscode"].get<int>() != UA_STATUSCODE_GOOD))
      {
        feedback.set_global_status(static_cast<UA_StatusCode>(server_response["statuscode"].get<int>()));
      }
    }
    else
    {
      feedback.add_message(Severity::ERROR, "No valid response received from server.");
      // Clean up the UA_Variant
      // UA_Variant_clear(&requestVariant);
      for (auto &output : outputArguments)
      {
        UA_Variant_clear(&output);
      }

      return false;
    }

    // Clean up the UA_Variant
    // UA_Variant_clear(&requestVariant);
    for (auto &output : outputArguments)
    {
      UA_Variant_clear(&output);
    }
    return true;
  }
  catch (const json::exception &e)
  {
    feedback.add_message(Severity::ERROR, "JSON exception in " + method_node + ": " + std::string(e.what()));
    return false;
  }
  catch (const std::exception &e)
  {
    feedback.add_message(Severity::ERROR, "Exception in " + method_node + ": " + std::string(e.what()));
    return false;
  }
  catch (...)
  {
    feedback.add_message(Severity::ERROR, "Unknown exception in " + method_node);
    return false;
  }
}

bool IoLSMonitor::set_pm_range(const int16_t &selection, FeedbackManager &feedback)
{
  UA_Variant input;
  UA_Variant_init(&input);
  UA_Variant_setScalarCopy(&input, &selection, &UA_TYPES[UA_TYPES_INT16]);
  bool success = exec_method_arg("LS1.PM1.set_range", input, feedback);
  UA_Variant_clear(&input);
  return success;
}

bool IoLSMonitor::set_pm_threshold(const uint16_t &selection, FeedbackManager &feedback)
{
  UA_Variant input;
  UA_Variant_init(&input);
  UA_Variant_setScalarCopy(&input, &selection, &UA_TYPES[UA_TYPES_UINT16]);
  bool success = exec_method_arg("LS1.PM1.set_threshold", input, feedback);
  UA_Variant_clear(&input);
  return success;
}

bool IoLSMonitor::set_dac_threshold(const uint16_t &selection, FeedbackManager &feedback)
{
  UA_Variant input;
  UA_Variant_init(&input);
  UA_Variant_setScalarCopy(&input, &selection, &UA_TYPES[UA_TYPES_UINT16]);
  bool success = exec_method_arg("LS1.CIB1.set_dac_threshold", input, feedback);
  UA_Variant_clear(&input);
  return success;
}
bool IoLSMonitor::set_att_pos(const uint32_t &value, FeedbackManager &feedback)
{
  UA_Variant input;
  UA_Variant_init(&input);
  UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_UINT32]);
  bool success = exec_method_arg("LS1.A1.set_position", input, feedback);
  UA_Variant_clear(&input);
  return success;
}

bool IoLSMonitor::pause(FeedbackManager &feedback)
{
  return exec_method_simple("LS1.pause", feedback);
}

bool IoLSMonitor::standby(FeedbackManager &feedback)
{
  return exec_method_simple("LS1.standby", feedback);
}

bool IoLSMonitor::resume(FeedbackManager &feedback)
{
  return exec_method_simple("LS1.resume", feedback);
}

bool IoLSMonitor::warmup(FeedbackManager &feedback)
{
  return exec_method_simple("LS1.warmup_laser", feedback);
}

bool IoLSMonitor::shutdown(FeedbackManager &feedback)
{
  return exec_method_simple("LS1.shutdown", feedback);
}
bool IoLSMonitor::clear_error(FeedbackManager &feedback)
{
  return exec_method_simple("LS1.clear_error", feedback);
}
bool IoLSMonitor::stop(FeedbackManager &feedback)
{
  return exec_method_simple("LS1.stop", feedback);
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

bool IoLSMonitor::write_variable(const std::string &variable, std::string &arg, std::string &type, FeedbackManager &feedback)
{
  try
  {
    // Check that the client is connected
    if (!m_client.is_connected())
    {
      feedback.add_message(Severity::ERROR, "Client is not connected.");
      return false;
    }

    // Initialize the input argument variant
    UA_Variant input;
    UA_Variant_init(&input);

    // Set the input argument based on the type
    if (type == "i")
    {
      int value = std::stoi(arg);
      UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_INT32]);
    }
    else if (type == "d")
    {
      double value = std::stod(arg);
      UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_DOUBLE]);
    }
    else if (type == "f")
    {
      float value = std::stof(arg);
      UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_FLOAT]);
    }
    else if (type == "s")
    {
      UA_String value = UA_STRING_ALLOC(arg.c_str());
      UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_STRING]);
      UA_String_clear(&value);
    }
    else if (type == "i32")
    {
      int32_t value = std::stoi(arg);
      UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_INT32]);
    }
    else if (type == "u32")
    {
      uint32_t value = std::stoul(arg);
      UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_UINT32]);
    }
    else if (type == "u16")
    {
      uint16_t value = static_cast<uint16_t>(std::stoul(arg));
      UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_UINT16]);
    }
    else if (type == "i16")
    {
      int16_t value = static_cast<int16_t>(std::stoi(arg));
      UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_INT16]);
    }
    else if (type == "b")
    {
      bool value = (arg == "true" || arg == "1");
      UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    else
    {
      feedback.add_message(Severity::ERROR, "Unsupported argument type: " + type);
      return false;
    }

    // Read the variable
    bool success = false;
    try
    {
      success = m_client.write_variable(variable, input, feedback);
    }
    catch (const std::exception &e)
    {
      feedback.add_message(Severity::ERROR, std::string("Exception in write_variable: ") + e.what());
      return false;
    }
    // Clear the input variant
    UA_Variant_clear(&input);

    if (!success)
    {
      feedback.add_message(Severity::ERROR, "Failed to write variable: " + variable);
      return false;
    }
    else
    {
      feedback.add_message(Severity::INFO, "Successfully write variable: " + variable);
      return true;
    }
  }
  catch (const std::exception &e)
  {
    feedback.add_message(Severity::ERROR, std::string("Exception in write_variable: ") + e.what());
    return false;
  }
  catch (...)
  {
    feedback.add_message(Severity::ERROR, "Unknown exception in write_variable");
    return false;
  }
}
bool IoLSMonitor::read_variable(const std::string &variable, UA_Variant &value, FeedbackManager &feedback)
{
  try
  {
    // Check that the client is connected
    if (!m_client.is_connected())
    {
      feedback.add_message(Severity::ERROR, "Client is not connected.");
      return false;
    }

    // Read the variable
    bool success = false;
    try
    {
      success = m_client.read_variable(variable, value, feedback);
    }
    catch(const std::exception& e)
    {
      feedback.add_message(Severity::ERROR, std::string("Exception in read_variable: ") + e.what());
      return false;
    }
    if (!success)
    {
      feedback.add_message(Severity::ERROR, "Failed to read variable: " + variable);
      return false;
    }
    else
    {
      feedback.add_message(Severity::INFO, "Successfully read variable: " + variable);
      return true;
    }    
  }
  catch (const std::exception &e)
  {
    feedback.add_message(Severity::ERROR, std::string("Exception in read_variable: ") + e.what());
    return false;
  }
  catch (...)
  {
    feedback.add_message(Severity::ERROR, "Unknown exception in read_variable");
    return false;
  }
}
bool IoLSMonitor::call_method(const std::string &method_name, const std::string &arg, const std::string &type, FeedbackManager &feedback)
{
  try
  {

    // Initialize the input argument variant
    UA_Variant input;
    UA_Variant_init(&input);

    // Set the input argument based on the type
    if (type == "i")
    {
      int value = std::stoi(arg);
      UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_INT32]);
    }
    else if (type == "d")
    {
      double value = std::stod(arg);
      UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_DOUBLE]);
    }
    else if (type == "f")
    {
      float value = std::stof(arg);
      UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_FLOAT]);
    }
    else if (type == "s")
    {
      UA_String value = UA_STRING_ALLOC(arg.c_str());
      UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_STRING]);
      UA_String_clear(&value);
    }
    else if (type == "i32")
    {
      int32_t value = std::stoi(arg);
      UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_INT32]);
    }
    else if (type == "u32")
    {
      uint32_t value = std::stoul(arg);
      UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_UINT32]);
    }
    else if (type == "u16")
    {
      uint16_t value = static_cast<uint16_t>(std::stoul(arg));
      UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_UINT16]);
    }
    else if (type == "i16")
    {
      int16_t value = static_cast<int16_t>(std::stoi(arg));
      UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_INT16]);
    }
    else if (type == "b")
    {
      bool value = (arg == "true" || arg == "1");
      UA_Variant_setScalarCopy(&input, &value, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    else
    {
      feedback.add_message(Severity::ERROR, "Unsupported argument type: " + type);
      return false;
    }

    bool success = exec_method_arg(method_name, input, feedback);
    // Clear the input variant
    UA_Variant_clear(&input);

    if (!success)
    {
      feedback.add_message(Severity::ERROR, "Failed to call method: " + method_name);
      return false;
    }
    else
    {
      feedback.add_message(Severity::INFO, "Method called successfully.");
      return true;
    }
  }
  catch (const std::exception &e)
  {
    feedback.add_message(Severity::ERROR, "Exception in call_method: " + std::string(e.what()));
    return false;
  }
  catch (...)
  {
    feedback.add_message(Severity::ERROR, "Unknown exception in call_method");
    return false;
  }
}