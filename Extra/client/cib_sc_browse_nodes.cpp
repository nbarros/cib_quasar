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

static UA_StatusCode
nodeIter(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle) {
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NodeId *parent = (UA_NodeId *)handle;
//    spdlog::info("{0},{1} --- {2} ---> {3},{4}",
//                 parent->namespaceIndex, parent->identifier.string.data,
//                 referenceTypeId.identifier.string.data, childId.namespaceIndex,
//                 childId.identifier.string.data);

//    // alternatively, we can maybe do it this way:
//    spdlog::info("{0},{1} --- {2} ---> {3},{4}",
//
//    if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
//        printf("%-9d %-16d %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
//               ref->nodeId.nodeId.identifier.numeric, (int)ref->browseName.name.length,
//               ref->browseName.name.data, (int)ref->displayName.text.length,
//               ref->displayName.text.data);
//    } else if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
//        printf("%-9d %-16.*s %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
//               (int)ref->nodeId.nodeId.identifier.string.length,
//               ref->nodeId.nodeId.identifier.string.data,
//               (int)ref->browseName.name.length, ref->browseName.name.data,
//               (int)ref->displayName.text.length, ref->displayName.text.data);
//    }

//    printf("%d, %d --- %d ---> NodeId %d, %d\n",
//           parent->namespaceIndex, parent->identifier.numeric,
//           referenceTypeId.identifier.numeric, childId.namespaceIndex,
//           childId.identifier.numeric);
    return UA_STATUSCODE_GOOD;
}

void browse_nodes_iter(UA_Client *client)
{
  /* Same thing, this time using the node iterator... */
  UA_NodeId *parent = UA_NodeId_new();
  *parent = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  spdlog::info("Parent ID --- reference type -- node id");

  UA_Client_forEachChildNodeCall(client, UA_NODEID_NUMERIC(2, UA_NS0ID_OBJECTSFOLDER),
                                 nodeIter, (void *) parent);
  UA_NodeId_delete(parent);
}

void browse_nodes_scan(UA_Client *client)
{
  printf("Browsing nodes in objects folder:\n");
  UA_BrowseRequest bReq;
  UA_BrowseRequest_init(&bReq);
  bReq.requestedMaxReferencesPerNode = 0;
  bReq.nodesToBrowse = UA_BrowseDescription_new();
  bReq.nodesToBrowseSize = 1;
  bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); /* browse objects folder */
  bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;                  /* return everything */
  UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
  printf("%-9s %-16s %-16s %-16s\n", "NAMESPACE", "NODEID", "BROWSE NAME", "DISPLAY NAME");
  for (size_t i = 0; i < bResp.resultsSize; ++i)
  {
    for (size_t j = 0; j < bResp.results[i].referencesSize; ++j)
    {
      UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
      if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC)
      {
        printf("%-9d %-16d %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
               ref->nodeId.nodeId.identifier.numeric, (int)ref->browseName.name.length,
               ref->browseName.name.data, (int)ref->displayName.text.length,
               ref->displayName.text.data);
      }
      else if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING)
      {
        printf("%-9d %-16.*s %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
               (int)ref->nodeId.nodeId.identifier.string.length,
               ref->nodeId.nodeId.identifier.string.data,
               (int)ref->browseName.name.length, ref->browseName.name.data,
               (int)ref->displayName.text.length, ref->displayName.text.data);
      }
      else
      {
        printf("Unknown node id type\n");
      }
      /* TODO: distinguish further types */
    }
  }
  UA_BrowseRequest_clear(&bReq);
  UA_BrowseResponse_clear(&bResp);
}
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
    spdlog::error("Failed to connect with code {0}  name {1}",retval,UA_StatusCode_name(retval));
    // we can print a message why
    UA_Client_delete(client);
    return EXIT_FAILURE;
  } 
  else
  {
    spdlog::info("Connected to server");
  }

  // -- First browse all nodes that are available
  browse_nodes_scan(client);


  UA_Client_disconnect(client);
  UA_Client_delete(client);
  return EXIT_SUCCESS;

}
