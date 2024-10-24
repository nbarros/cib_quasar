#include "IoLSMonitor.h"
#include <spdlog/spdlog.h>
#include <json.hpp>
#include <fstream>
#include <regex>

using json = nlohmann::json;

IoLSMonitor::IoLSMonitor(const std::string &serverUrl)
  : m_serverUrl(serverUrl), m_running(false)
{
}

IoLSMonitor::~IoLSMonitor()
{
  disconnect();
}

bool IoLSMonitor::connect()
{
  try
  {
    m_client.connect(m_serverUrl);
    m_running = true;
    m_monitor_thread = std::thread(&IoLSMonitor::monitor_loop, this);
    return true;
  }
  catch (const std::exception &e)
  {
    spdlog::critical("Failed to connect to server: {}", e.what());
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
}

bool IoLSMonitor::config(std::string location, json &response)
{
  try
  {
    std::ifstream file(location);
    if (!file.is_open())
    {
      spdlog::error("Failed to open file at [{0}]", location);
      return EXIT_FAILURE;
    }
    json jconf = json::parse(file); // parse the file
    spdlog::trace("Configuration file [{0}]", jconf.dump());
    // now we have the json object, we can send it to the server
    //
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

    m_client.call_method(node_base, method, {configVariant}, outputArguments);

    std::string reply;
    if (!outputArguments.empty() && UA_Variant_hasScalarType(&outputArguments[0], &UA_TYPES[UA_TYPES_STRING]))
    {
      UA_String *uaResponse = static_cast<UA_String *>(outputArguments[0].data);
      reply = std::string(reinterpret_cast<char *>(uaResponse->data), uaResponse->length);
      spdlog::trace("Reply from server: {}", reply);
    }
    else
    {
      spdlog::error("No valid response received from server.");
    }

    response = json::parse(reply);

    // Clean up the UA_Variant
    UA_Variant_clear(&configVariant);
    for (auto &output : outputArguments)
    {
      UA_Variant_clear(&output);
    }

    return true;
  }
  catch (const std::exception &e)
  {
    spdlog::critical("Failed to read variable: {}", e.what());
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
        UA_Variant value;
        UA_Variant_init(&value);
        m_client.read_variable(item.first, value);
        item.second = value;
        UA_Variant_clear(&value);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
  catch (const json::exception &e)
  {
    spdlog::critical("JSON exception occurred: {}", e.what());
  }
  catch (const std::exception &e)
  {
    spdlog::critical("Failed to monitor server: {}", e.what());
  }
}

void IoLSMonitor::monitor_loop()
{
  monitor_server();
}

bool IoLSMonitor::move_to_position(std::string position, std::string approach,json &response)
{
  try
  {
    // Check that the position input has the format [x,y,z]
    std::regex position_regex(R"(\[\s*(-?\d+(\.\d+)?),\s*(-?\d+(\.\d+)?),\s*(-?\d+(\.\d+)?)\s*\])");
    std::smatch position_match;
    if (!std::regex_match(position, position_match, position_regex))
    {
      spdlog::error("Invalid position format: {}", position);
      return false;
    }

    // Check that the approach input is a string with three characters and all of them are one of 'u', 'd', '-'
    if (approach.size() != 3 || !std::all_of(approach.begin(), approach.end(), [](char c)
                                             { return c == 'u' || c == 'd' || c == '-'; }))
    {
      spdlog::error("Invalid approach format: {}", approach);
      return false;
    }

    // Convert the two input strings into a JSON variable
    json jrequest;
    jrequest["target"] = json::array({std::stod(position_match[1]), std::stod(position_match[3]), std::stod(position_match[5])});
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
    m_client.call_method("LS1", "LS1.move_to_pos", {requestVariant}, outputArguments);

    // Convert the single output argument into a string and parse it into the response JSON variable
    if (!outputArguments.empty() && UA_Variant_hasScalarType(&outputArguments[0], &UA_TYPES[UA_TYPES_STRING]))
    {
      UA_String *uaResponse = static_cast<UA_String *>(outputArguments[0].data);
      std::string responseString(reinterpret_cast<char *>(uaResponse->data), uaResponse->length);
      response = json::parse(responseString);
      spdlog::trace("Response from server: {}", response.dump());
    }
    else
    {
      spdlog::error("No valid response received from server.");
      response.clear();
    }

    // Clean up the UA_Variant
    UA_Variant_clear(&requestVariant);
    for (auto &output : outputArguments)
    {
      UA_Variant_clear(&output);
    }

    return true;
  }
  catch (const std::exception &e)
  {
    spdlog::critical("Exception in move_to_position: {}", e.what());
    return false;
  }
}

bool IoLSMonitor::fire_at_position(const std::string position, const uint32_t num_shots, json &response)
{

}

bool IoLSMonitor::fire_segment(const std::string start_position, const std::string end_position, json &response)
{

}

bool IoLSMonitor::execute_scan(const std::string run_plan, json &response)
{

}

bool IoLSMonitor::pause(json &response)
{

}

bool IoLSMonitor::standby(json &response)
{

}

bool IoLSMonitor::resume(json &response)
{

}

bool IoLSMonitor::warmup(json &response)
{

}

bool IoLSMonitor::shutdown(json &response)
{

}

bool IoLSMonitor::stop(json &response)
{

}
