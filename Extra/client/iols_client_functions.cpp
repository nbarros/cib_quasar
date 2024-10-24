#include "iols_client_functions.h"
#include <fstream>

// this method sets the stage to exit execution
void terminate_client()
{
  UA_Client_disconnect(g_client);
  UA_Client_delete(g_client);
}
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
template <typename T>
void query_variable(const std::string node, T &var)
{
  UA_Variant *val = UA_Variant_new();
  UA_StatusCode retval = UA_Client_readValueAttribute(g_client, UA_NODEID_STRING(2, const_cast<char *>(node.c_str())), val);
  if (retval == UA_STATUSCODE_GOOD)
  {
    var = *static_cast<T *>(val->data);
  }
  else
  {
    spdlog::error("Failed to check on node [{0}]. Returned {1} ({2})",node,retval,UA_StatusCode_name(retval));
  }
  UA_Variant_delete(val);
}
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
void get_motor_position(UA_Client *client, const std::string nodebase, motor_position_t &m)
{
  spdlog::debug("Checking motor position {0}", nodebase);
  assert(g_client != nullptr);
  std::vector<std::string> nodes = {".current_position_motor", ".current_position_cib"};
  std::string node = nodebase + nodes.at(0);
  query_variable( node, m.motor);
  node = nodebase + nodes.at(1);
  query_variable(node, m.cib);
}
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
int connect_to_server(const std::string server)
{
  assert(g_client == nullptr); // make sure that this is the first connection
  g_client = UA_Client_new();
  UA_ClientConfig_setDefault(UA_Client_getConfig(g_client));
  spdlog::info("Connecting to CIB server at [{0}]", server);
  UA_StatusCode retval = UA_Client_connect(g_client, server.c_str());
  if (retval != UA_STATUSCODE_GOOD)
  {
    spdlog::error("Failed with code {0}  name {1}", retval, UA_StatusCode_name(retval));
    // we can print a message why
    UA_Client_delete(g_client);
    g_client = nullptr;
    return EXIT_FAILURE;
  }
  else
  {
    spdlog::info("Connected to server");
  }
  return 0;
}
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
void parse_method_response(const std::string response)
{
  try
  {
    json jresp = json::parse(response);
    spdlog::trace("Returned [{0}]", jresp.dump());
    spdlog::info("Returned status : {0}", jresp["status"].get<std::string>());
    spdlog::info("Returned statuscode : {0}", jresp["statuscode"].get<uint32_t>());
    spdlog::info("Returned messages :");
    for (auto e : jresp.at("messages"))
    {
      spdlog::info("{0}", e.get<std::string>());
    }
  }
  catch (const json::exception &e)
  {
    spdlog::error("Caught JSON exception : {0}", e.what());
  }
  catch (const std::exception &e)
  {
    spdlog::error("Caught STD exception : {0}", e.what());
  }
  catch (...)
  {
    spdlog::critical("Caught an unknown exception");
  }
}
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
int config(const std::string location)
{
  // load the file at location and parse it as a json object
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
  std::string method = "config";
  std::string request = "{\"method\":\"" + method + "\",\"location\":\"" + location + "\"}";
  spdlog::trace("Requesting [{0}]", request);
  UA_Variant input;
  UA_Variant_init(&input);
  UA_Variant_setScalarCopy(&input, &request, &UA_TYPES[UA_TYPES_STRING]);
  UA_Variant *output = nullptr;
  UA_NodeId methodId = UA_NODEID_STRING(2, const_cast<char *>(method.c_str()));
  size_t inputSize = 1;
  size_t outputSize = 1;

  const UA_Variant *inputArray = &input;
  UA_Variant *outputArray;
  UA_StatusCode retval = UA_Client_call(g_client, UA_NODEID_NULL, methodId, inputSize, inputArray, &outputSize, &outputArray);
  if (retval == UA_STATUSCODE_GOOD)
  {
    std::string response = *static_cast<std::string *>(output->data);
    parse_method_response(response);
    UA_Variant_delete(output);
  }
  else
  {
    spdlog::error("Failed to call method [{0}]. Returned {1} ({2})", method, retval, UA_StatusCode_name(retval));
  }
  return retval;
}