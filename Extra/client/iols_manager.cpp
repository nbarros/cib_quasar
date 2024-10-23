/**
 * Created on: Oct 23, 2024
 *     Author: Nuno Barros <barros@lip.pt>
 */
#define UA_LOGLEVEL 30

extern "C"
{
  #include <open62541.h>
}

#include <spdlog/spdlog.h>
#include <json.hpp>
#include <vector>
#include <string>
#include <cstdint>

using json = nlohmann::json;
/**
 * Structures to help manage the system
 */
typedef struct motor_position_t
{
  int32_t motor;
  int32_t cib;
} motor_position_t;

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
      spdlog::info("{0}",e.get<std::string>());
    }
  }
  catch (const json::exception &e)
  {
    spdlog::error("Caught JSON exception : {0}", e.what());
  }
  catch (const std::exception &e)
  {
    spdlog::error("Caught STD exception : {0}",e.what());
  }
  catch(...)
  {
    spdlog::critical("Caught an unknown exception");
  }
}

// this method sets the stage to exit execution
void terminate_client(UA_Client *client)
{
  UA_Client_disconnect(client);
  UA_Client_delete(client);
}

template <typename T>
void query_variable(UA_Client *client, const std::string node, T &var)
{
  UA_Variant *val = UA_Variant_new();
  UA_StatusCode retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(2, const_cast<char *>(vname.c_str())), val);
  if (retval == UA_STATUSCODE_GOOD)
  {
    var = *static_cast<T*>(val->data);
  }
  else
  {
    spdlog::error("Failed to check on node [{0}]. Returned {1} ({2})",node,retval,UA_StatusCode_name(retval)));
  }
  UA_Variant_delete(val);
}
void get_motor_position(UA_Client *client, const std::string nodebase, motor_position_t &m)
{
  spdlog::debug("Checking motor position {0}", nodebase);
  assert(client!= nullptr);
  std::vector<std::string> nodes = {".current_position_motor", ".current_position_cib"};
  std::string node = nodebase + nodes.at(0);
  query_variable(client,node,m.motor);
  node = nodebase + nodes.at(1);
  query_variable(client, node, m.cib);
}
int connect_to_server(UA_Client *&client, const std::string server)
{
  client = UA_Client_new();
  UA_ClientConfig_setDefault(UA_Client_getConfig(client));
  spdlog::info("Connecting to CIB server at [{0}]", server);
  UA_StatusCode retval = UA_Client_connect(client, server.c_str());
  if (retval != UA_STATUSCODE_GOOD)
  {
    spdlog::error("Failed with code {0}  name {1}", retval, UA_StatusCode_name(retval));
    // we can print a message why
    UA_Client_delete(client);
    return EXIT_FAILURE;
  }
  else
  {
    spdlog::info("Connected to server");
  }
}

int run_command(int argc, char **argv)
{
  if (argc < 1)
  {
    return 1;
  }
  int ret = 0;
  std::string cmd(argv[0]);
  // check command request
  if (cmd == "exit")
  {
    return 255;
  }
  else if (cmd == "help")
  {
    print_help();
    return 0;
  }
  else if (cmd == "connect")
  {
    if (argc != 2)
    {
      spdlog::warn("Usage: connect <server>");
      spdlog::warn("    <server> can be one of:");
      spdlog::warn("        connect opc.tcp://1.1.1.1:2222 (replacing the IP and port to suit your needs)");
      spdlog::warn("        connect cib1 (to connect to server on CIB1)");
      spdlog::warn("        connect cib2 (to connect to server on CIB2)");
    }
    else
    {
      std::string server = argv[1];
      if (server == "cib1")
      {
        server = "opc.tcp://10.73.137.147:4841";
      }
      else if (server == "cib2")
      {
        server = "opc.tcp://10.73.137.148:4841"
      }
      UA_Client *client = nullptr;
      ret = connect_to_server(client, server);
      if (ret)
      {
        spdlog::critical("Failed to connect to server.");
        return 0;
      }      
    }
  }
  else if (cmd == "connect")
  {

  }
  else
  {
    spdlog::error("Unknown command");
    return 0;
  }
  return 0;
}

void print_help()
{
  spdlog::info("Available commands (note, commands without arguments print current settings):");
  // NFB: This command is no longer available for releases above v09-00
  //  spdlog::info("  dna");
  //  spdlog::info("    Obtain the DNA value from the FPGA silicon.");
  spdlog::info("  connect <server>");
  spdlog::info("    Connect to the slow control server. The format is: opc.tcp://<ip>:<port>");
  spdlog::info("    For ease of use, two aliases have been created: cib1, cib2");
}