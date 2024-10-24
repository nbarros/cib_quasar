/**
 * Created on: Oct 23, 2024
 *     Author: Nuno Barros <barros@lip.pt>
 */
#include "iols_client_functions.h"


/**
 * Global variables that are used in this client
 */


void print_help()
{
  spdlog::info("config <file>");
  spdlog::info("\t\tLoad configuration into the IoLS server");
  spdlog::info("help");
  spdlog::info("\t\tPrint this help");
  spdlog::info("exit");
  spdlog::info("\t\tExit execution. Will automatically disconnect from the server, but won't shutdown the system.");
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

int main(int argc, char** argv)
{
  // initialize globals
  g_client = nullptr;

  // all good, time to leave
  return 0;
}