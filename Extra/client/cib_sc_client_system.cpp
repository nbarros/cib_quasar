/*
 * cib_sc_client_system.cpp
 *
 *  Created on: Oct 15, 2024
 *      Author: Nuno Barros
 */


#define UA_LOGLEVEL 300



extern "C" {
#include <open62541.h>
};
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <cstring>
#include <fstream>
#include <json.hpp>
#include <spdlog/spdlog.h>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using json = nlohmann::json;

void parse_method_response_string(std::string &input)
{
  try
  {
    json jresp = json::parse(input);
    spdlog::info("Returned status : {0}",jresp["status"].get<std::string>());
    spdlog::info("Returned statuscode : {0}",jresp["statuscode"].get<uint32_t>());
    spdlog::info("Returned messages :");
    for (auto e : jresp.at("messages"))
    {
      spdlog::info("--> [{0}]",e.get<std::string>());
    }

  }
  catch(json::exception &e)
  {
    spdlog::critical("Caught a JSON exception : {0}",e.what());
  }
  catch(std::exception &e)
  {
    spdlog::critical("Caught an STL exception : {0}",e.what());
  }
  catch(...)
  {
    spdlog::critical("Caught an unknown exception");
  }

}

int main()
{
  spdlog::set_pattern("cib : [%^%L%$] %v");
  spdlog::set_level(spdlog::level::trace); // Set global log level to info

  UA_StatusCode retval = UA_STATUSCODE_GOOD;
  // CIB2 IP
  string server = "opc.tcp://10.73.137.148:4841";

  // create a new client, and use the default configuration options
  // for now this is fine
  // a new client. that's us
  UA_Client *client = UA_Client_new();
  UA_ClientConfig_setDefault(UA_Client_getConfig(client));

  /* Connect to the server */
  /* anonymous connect would be: retval = UA_Client_connect(client, "opc.tcp://localhost:4840"); */
  // retval = UA_Client_connectUsername(client, "opc.tcp://localhost:4840", "user1", "password");

  spdlog::info("Connecting to CIB server at [{0}]",server);

  retval = UA_Client_connect(client, server.c_str());

  if(retval != UA_STATUSCODE_GOOD)
  {
    spdlog::error("Failed with code {0}  name {1}",retval,UA_StatusCode_name(retval));
    // we can print a message why
    UA_Client_delete(client);
    return EXIT_FAILURE;
  } else
  {
    spdlog::info("Connected to server");
  }


  /**
   * Now we're in business. Let's rock!
   *
   *
   */
  spdlog::info("Let's try a complete sequence of operation.\n\n"
      "1.Load a configuration and send it to the server\n"
      "2.Check status of the various systems\n"
      "3.Check the state of the laser\n"
      "4.shutdown the system\n");

  std::ifstream fconf("iols_p2_config.json");

  json jconf = json::parse(fconf);

  spdlog::info("\n\nStage 1 : configure IOLS LS1.load_config\n\n");
  UA_UInt32 state;
  // pass our configuration argument
  UA_Variant input;
  size_t outputSize;
  UA_Variant *output;


  UA_Variant_init(&input);

  UA_String argString = UA_String_fromChars(jconf.dump().c_str());
  UA_Variant_setScalarCopy(&input, &argString, &UA_TYPES[UA_TYPES_STRING]);

  // prototype of the client call
  //    UA_StatusCode UA_Client_call(UA_Client * client,
  //                  const UA_NodeId objectId,
  //                  const UA_NodeId methodId,
  //                  size_t inputSize,
  //                  const UA_Variant *input,
  //                  size_t *outputSize,
  //                  UA_Variant **output)
  //
  retval = UA_Client_call(client,
                          UA_NODEID_STRING(2, "LS1"),
                          UA_NODEID_STRING(2, "LS1.load_config"),
                          1,
                          &input,
                          &outputSize,
                          &output);
  if(retval == UA_STATUSCODE_GOOD)
  {
    spdlog::info("Method call was successful. received {0} output values with the following output:",outputSize);
    // All methods in the CIB return a json string
    std::string response((char*)static_cast<UA_String*>(output[0].data)->data,(size_t)static_cast<UA_String*>(output[0].data)->length);
    spdlog::info("{0}",response);
    parse_method_response_string(response);
    //    json jresp = json::parse(response);
    //    spdlog::info("After parsing : {0}",jresp.dump().c_str());
    // if we want to be more picky:
    // clear the array
    UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
  }
  else
  {
    spdlog::error("Method execution failed with code {0} : {1}",retval,UA_StatusCode_name(retval));
  }
  // clear up the configuration allocated parts
  UA_Variant_clear(&input);
  fconf.close();

  spdlog::info("\n\nStage 2 : Check the individual node status\n\n");
  spdlog::info("Checking A1");

  UA_Variant *val = UA_Variant_new();
  retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(2, "LS1.A1.state"), val);
  if (retval == UA_STATUSCODE_GOOD)
  {
    if (UA_Variant_isScalar(val))
    {
      spdlog::debug("string reports as a scalar...interesting!");
    }
    // strings are arrays, therefore
    if (val->type == &UA_TYPES[UA_TYPES_STRING])
    {
      std::string value((char*)static_cast<UA_String*>(val->data)->data,(size_t)static_cast<UA_String*>(val->data)->length);
      spdlog::info("A1 state : {0}",value);
    }
    else
    {
      spdlog::error("Failed type check on val. Got {0}",(val->type)->typeName);
    }
  }
  else
  {
    spdlog::error("Failed to request value. Got error {0} : {1} ",retval,UA_StatusCode_name(retval));
  }
  // clear the variant from the query
  UA_Variant_delete(val);

  // repeat for the power meter
  spdlog::info("Checking PM1");
  val = UA_Variant_new();
  retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(2, "LS1.PM1.state"), val);
  if (retval == UA_STATUSCODE_GOOD)
  {
    // strings are arrays, therefore
    if (val->type == &UA_TYPES[UA_TYPES_STRING])
    {
      std::string value((char*)static_cast<UA_String*>(val->data)->data,(size_t)static_cast<UA_String*>(val->data)->length);
      spdlog::info("PM1 state : {0}",value);
    }
    else
    {
      spdlog::error("Failed type check on val. Got {0}",(val->type)->typeName);
    }
  }
  else
  {
    spdlog::error("Failed to request value. Got error {0} : {1} ",retval,UA_StatusCode_name(retval));
  }
  // clear the variant from the query
  UA_Variant_delete(val);

  // repeat for the power meter
  spdlog::info("Checking CIB1");
  val = UA_Variant_new();
  retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(2, "LS1.CIB1.state"), val);
  if (retval == UA_STATUSCODE_GOOD)
  {
    // strings are arrays, therefore
    if (val->type == &UA_TYPES[UA_TYPES_STRING])
    {
      std::string value((char*)static_cast<UA_String*>(val->data)->data,(size_t)static_cast<UA_String*>(val->data)->length);
      spdlog::info("CIB1 state : {0}",value);
    }
    else
    {
      spdlog::error("Failed type check on val. Got {0}",(val->type)->typeName);
    }
  }
  else
  {
    spdlog::error("Failed to request value. Got error {0} : {1} ",retval,UA_StatusCode_name(retval));
  }
  // clear the variant from the query
  UA_Variant_delete(val);

  // repeat for the laser
  spdlog::info("Checking L1");
  val = UA_Variant_new();
  retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(2, "LS1.L1.state"), val);
  if (retval == UA_STATUSCODE_GOOD)
  {
    // strings are arrays, therefore
    if (val->type == &UA_TYPES[UA_TYPES_STRING])
    {
      std::string value((char*)static_cast<UA_String*>(val->data)->data,(size_t)static_cast<UA_String*>(val->data)->length);
      spdlog::info("L1 state : {0}",value);
    }
    else
    {
      spdlog::error("Failed type check on val. Got {0}",(val->type)->typeName);
    }
  }
  else
  {
    spdlog::error("Failed to request value. Got error {0} : {1} ",retval,UA_StatusCode_name(retval));
  }
  // clear the variant from the query
  UA_Variant_delete(val);

  // And finally the motors

  spdlog::info("Checking RNN800");
  val = UA_Variant_new();
  retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(2, "LS1.RNN800.state"), val);
  if (retval == UA_STATUSCODE_GOOD)
  {
    // strings are arrays, therefore
    if (val->type == &UA_TYPES[UA_TYPES_STRING])
    {
      std::string value((char*)static_cast<UA_String*>(val->data)->data,(size_t)static_cast<UA_String*>(val->data)->length);
      spdlog::info("RNN800 state : {0}",value);
    }
    else
    {
      spdlog::error("Failed type check on val. Got {0}",(val->type)->typeName);
    }
  }
  else
  {
    spdlog::error("Failed to request value. Got error {0} : {1} ",retval,UA_StatusCode_name(retval));
  }
  // clear the variant from the query
  UA_Variant_delete(val);

  spdlog::info("Checking RNN600");
  val = UA_Variant_new();
  retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(2, "LS1.RNN600.state"), val);
  if (retval == UA_STATUSCODE_GOOD)
  {
    // strings are arrays, therefore
    if (val->type == &UA_TYPES[UA_TYPES_STRING])
    {
      std::string value((char*)static_cast<UA_String*>(val->data)->data,(size_t)static_cast<UA_String*>(val->data)->length);
      spdlog::info("RNN600 state : {0}",value);
    }
    else
    {
      spdlog::error("Failed type check on val. Got {0}",(val->type)->typeName);
    }
  }
  else
  {
    spdlog::error("Failed to request value. Got error {0} : {1} ",retval,UA_StatusCode_name(retval));
  }
  // clear the variant from the query
  UA_Variant_delete(val);

  spdlog::info("Checking LSTAGE");
  val = UA_Variant_new();
  retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(2, "LS1.LSTAGE.state"), val);
  if (retval == UA_STATUSCODE_GOOD)
  {
    // strings are arrays, therefore
    if (val->type == &UA_TYPES[UA_TYPES_STRING])
    {
      std::string value((char*)static_cast<UA_String*>(val->data)->data,(size_t)static_cast<UA_String*>(val->data)->length);
      spdlog::info("LSTAGE state : {0}",value);
    }
    else
    {
      spdlog::error("Failed type check on val. Got {0}",(val->type)->typeName);
    }
  }
  else
  {
    spdlog::error("Failed to request value. Got error {0} : {1} ",retval,UA_StatusCode_name(retval));
  }
  // clear the variant from the query
  UA_Variant_delete(val);

  // and finally the system as a whole
  spdlog::info("Checking LS1");
  val = UA_Variant_new();
  retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(2, "LS1.state"), val);
  if (retval == UA_STATUSCODE_GOOD)
  {
    // strings are arrays, therefore
    if (val->type == &UA_TYPES[UA_TYPES_STRING])
    {
      std::string value((char*)static_cast<UA_String*>(val->data)->data,(size_t)static_cast<UA_String*>(val->data)->length);
      spdlog::info("LS1 state : {0}",value);
    }
    else
    {
      spdlog::error("Failed type check on val. Got {0}",(val->type)->typeName);
    }
  }
  else
  {
    spdlog::error("Failed to request value. Got error {0} : {1} ",retval,UA_StatusCode_name(retval));
  }
  // clear the variant from the query
  UA_Variant_delete(val);

  spdlog::info("\n\nStage 3 : Query the security report of the laser\n\n");

  UA_UInt16 laser_status_code;
  val = UA_Variant_new();
  //UA_Variant * val_desc = UA_Variant_new();
  retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(2, "LS1.L1.laser_status_code"), val);
  if (retval != UA_STATUSCODE_GOOD)
  {
    spdlog::error("Failed to query for status code");
  }
  else
  {
    if (UA_Variant_isScalar(val))
    {
      if (val->type == &UA_TYPES[UA_TYPES_UINT16])
      {
        laser_status_code = *static_cast<UA_Int16*>(val->data);
        spdlog::info("L1 status code : {0}",laser_status_code);
      }
    }
    else
    {
      spdlog::error("Failed type check on val type. Got {0}",(val->type)->typeName);
    }
  }
  // -- an even better way (that also guarantees that there is a result
  // is to calldirectly the method
  UA_Variant_init(&input);
  retval = UA_Client_call(client, UA_NODEID_STRING(2, "LS1.L1"),
      UA_NODEID_STRING(2, "LS1.L1.check_laser_status"), 0, &input, &outputSize, &output);
  if(retval == UA_STATUSCODE_GOOD)
  {
    spdlog::debug("Method call was successful with {0} returned values (expect 2)",outputSize);
    // the first argument should be the status code
    // and the second should be the description
    laser_status_code = *static_cast<UA_Int16*>(output[0].data);
    std::string desc((char*)static_cast<UA_String*>(output[1].data)->data,(size_t)static_cast<UA_String*>(output[1].data)->length);
    spdlog::info("Response : {0} ({1})",laser_status_code,desc);
  }
  else
  {
    spdlog::error("Failed to request value. Got error {0} : {1} ",retval,UA_StatusCode_name(retval));
  }
  // clear the variant from the query
  UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
  UA_Variant_clear(&input);

  spdlog::info("\n\nStage 4 : Shut down the system\n\n");

  UA_Variant_init(&input);
  retval = UA_Client_call(client, UA_NODEID_STRING(2, "LS1"),
      UA_NODEID_STRING(2, "LS1.shutdown"), 0, &input, &outputSize, &output);
  if(retval == UA_STATUSCODE_GOOD)
  {
    spdlog::info("Method called successfully. Returned {0} arguments (none expected)",outputSize);
    // actually, this returns a typical response string
    std::string response((char*)static_cast<UA_String*>(output[0].data)->data,(size_t)static_cast<UA_String*>(output[0].data)->length);
    parse_method_response_string(response);
  }
  else
  {
    spdlog::error("Failed to execute method. Got error {0} : {1} ",retval,UA_StatusCode_name(retval));
  }
  UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
  UA_Variant_clear(&input);

  spdlog::info("All done. Shutting down client.");

  UA_Client_disconnect(client);
  UA_Client_delete(client);
  return EXIT_SUCCESS;

}
