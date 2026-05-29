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
#include <cctype>
#include <fstream>
#include <json.hpp>
#include <spdlog/spdlog.h>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using json = nlohmann::json;

enum class NodeReportType
{
  all,
  variable,
  method
};

enum class NodeKind
{
  variable,
  method
};

struct BrowseReport
{
  std::vector<std::string> in_order;
  std::vector<std::string> variables;
  std::vector<std::string> methods;
};

static bool should_report(NodeKind kind, NodeReportType filter)
{
  if (filter == NodeReportType::all)
  {
    return true;
  }
  if (filter == NodeReportType::variable)
  {
    return kind == NodeKind::variable;
  }
  return kind == NodeKind::method;
}

static void add_report_line(BrowseReport &report, NodeKind kind, const std::string &line)
{
  report.in_order.push_back(line);
  if (kind == NodeKind::variable)
  {
    report.variables.push_back(line);
  }
  else
  {
    report.methods.push_back(line);
  }
}

static std::string to_lower_copy(std::string value)
{
  for (char &ch : value)
  {
    ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
  }
  return value;
}

static bool parse_type_filter(const std::string &value, NodeReportType &out_filter)
{
  const std::string normalized = to_lower_copy(value);
  if (normalized == "all")
  {
    out_filter = NodeReportType::all;
    return true;
  }
  if (normalized == "variable")
  {
    out_filter = NodeReportType::variable;
    return true;
  }
  if (normalized == "method")
  {
    out_filter = NodeReportType::method;
    return true;
  }
  return false;
}

static void print_usage(const char *program)
{
  std::cout << "Usage: " << program << " [options]\n"
            << "Options:\n"
            << "  -s, --sort           Group output by node type\n"
            << "  -t, --type <type>    Node type filter: variable | method | all\n"
            << "  -h, --help           Show this help message\n";
}

static void print_report(const BrowseReport &report, bool sort_by_type)
{
  if (!sort_by_type)
  {
    for (const auto &line : report.in_order)
    {
      std::cout << line << std::endl;
    }
    return;
  }

  std::cout << "[VARIABLES]" << std::endl;
  for (const auto &line : report.variables)
  {
    std::cout << line << std::endl;
  }

  std::cout << "[METHODS]" << std::endl;
  for (const auto &line : report.methods)
  {
    std::cout << line << std::endl;
  }
}

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

void browse_nodes_scan(UA_Client *client, UA_NodeId *node)
{
  printf("Browsing nodes in objects folder:\n");
  UA_BrowseRequest bReq;
  UA_BrowseRequest_init(&bReq);
  bReq.requestedMaxReferencesPerNode = 0;
  bReq.nodesToBrowse = UA_BrowseDescription_new();
  bReq.nodesToBrowseSize = 1;
  if (node == nullptr)
  {
    bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); /* browse objects folder */
  }
  else
  {
    bReq.nodesToBrowse[0].nodeId = *node; /* browse objects folder */
  }
  bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;                  /* return everything */
  UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
  for (size_t i = 0; i < bResp.resultsSize; ++i)
  {
    for (size_t j = 0; j < bResp.results[i].referencesSize; ++j)
    {
      UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
      // we only care for nodes with namespace 2
//      if (ref->nodeId.nodeId.namespaceIndex == 2)
//      {
//        printf("%-16.*s %-16.*s\n",
//               (int)ref->nodeId.nodeId.identifier.string.length,
//               ref->nodeId.nodeId.identifier.string.data,
//               (int)ref->browseName.name.length, ref->browseName.name.data
//                );
//      }

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
        // repeat for this node
        std::string nodeid_str = std::string(reinterpret_cast<char*>(ref->nodeId.nodeId.identifier.string.data));
        if (nodeid_str.find("LS1") != std::string::npos)
        {
          browse_nodes_scan(client,&(ref->nodeId.nodeId));
        }
      }
      else
      {
        printf("Unknown node id type\n");
      }
      /* TODO: distinguish further types */
    }
  }
  printf("Clearing\n");
  UA_BrowseRequest_clear(&bReq);
  //UA_BrowseResponse_clear(&bResp);
  printf("Done clearing\n");
}
void browse_nodes_scan_cib(UA_Client *client, UA_NodeId *node, NodeReportType filter, BrowseReport &report)
{
  //printf("Browsing nodes in objects folder:\n");
  UA_StatusCode retval = UA_STATUSCODE_GOOD;
  UA_BrowseRequest bReq;
  UA_BrowseRequest_init(&bReq);
  bReq.requestedMaxReferencesPerNode = 0;
  bReq.nodesToBrowse = UA_BrowseDescription_new();
  bReq.nodesToBrowseSize = 1;
  // if no parent is specified, start from the root folder
  if (node == nullptr)
  {
    //bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); /* browse objects folder */
    bReq.nodesToBrowse[0].nodeId = UA_NODEID_STRING(2, "LS1"); /* browse objects folder */
  }
  else
  {
    bReq.nodesToBrowse[0].nodeId = *node; /* browse objects folder */
  }
  bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;                  /* return everything */
  UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
  // now loop the results
  for (size_t i = 0; i < bResp.resultsSize; ++i)
  {
    for (size_t j = 0; j < bResp.results[i].referencesSize; ++j)
    {
      // grab a reference
      UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
      // if the node is in namespace 2, it is something we care.
      // if it is in namespace 0, it is a descriptor of the previous node
      if (ref->nodeClass == UA_NODECLASS_VARIABLE)
      {
        UA_Variant output;
        UA_Variant_init(&output);

        retval = UA_Client_readValueAttribute(client, ref->nodeId.nodeId, &output);
        if (retval != UA_STATUSCODE_GOOD)
        {
          UA_Variant_clear(&output);
          continue;
        }
        if (output.type == &UA_TYPES[UA_TYPES_EXTENSIONOBJECT])
        {
          // this is an argument
          // try to fetch its data type exactly
          if (should_report(NodeKind::variable, filter))
          {
            char line[256];
            std::snprintf(line, sizeof(line), ",,%-16.*s",
                          (int)ref->browseName.name.length, ref->browseName.name.data);
            add_report_line(report, NodeKind::variable, std::string(line));
          }
          //(int)ref->browseName.name.length, ref->browseName.name.data,
        }
        else
        {
          // query the data type
          // UA_Client_readValueAttribute(UA_Client *client, const UA_NodeId nodeId, UA_Variant *outValue)
          if (should_report(NodeKind::variable, filter))
          {
            char line[256];
            std::snprintf(line, sizeof(line), "%-16.*s,VAR,%s",
                          (int)ref->nodeId.nodeId.identifier.string.length,
                          ref->nodeId.nodeId.identifier.string.data,
                          output.type->typeName);
            add_report_line(report, NodeKind::variable, std::string(line));
          }

          //UA_Datatype *tp = &UA_TYPES[output.type]
        }
        UA_Variant_clear(&output);

        // there is a special case here. method arguments also report as variables


        // do we need to go further?
      }
      else if (ref->nodeClass == UA_NODECLASS_OBJECTTYPE)
      {
//        printf("\t\t\t\t%-16.*s --> %-16.*s\n",
//               (int)ref->browseName.name.length,
//               ref->browseName.name.data,
//               (int)ref->nodeId.nodeId.identifier.string.length,
//               ref->nodeId.nodeId.identifier.string.data
//        );
      }
      else if (ref->nodeClass == UA_NODECLASS_METHOD)
      {
        // in this case we need to figure out whether they have arguments
        // so we should also fetch their arguments
        if (should_report(NodeKind::method, filter))
        {
          char line[256];
          std::snprintf(line, sizeof(line), "%-16.*s,METHOD",
                        (int)ref->nodeId.nodeId.identifier.string.length,
                        ref->nodeId.nodeId.identifier.string.data);
          add_report_line(report, NodeKind::method, std::string(line));
        }

        browse_nodes_scan_cib(client, &(ref->nodeId.nodeId), filter, report);

      }
      else if (ref->nodeClass == UA_NODECLASS_OBJECT)
      {
        // browse one further
        browse_nodes_scan_cib(client, &(ref->nodeId.nodeId), filter, report);
      }
      else
      {
        printf("%-16.*s,%d\n",
               (int)ref->nodeId.nodeId.identifier.string.length,
               ref->nodeId.nodeId.identifier.string.data,
               (int)ref->nodeClass
               );

      }
    }
  }
  //printf("Clearing\n");
  UA_BrowseRequest_clear(&bReq);
  //UA_BrowseResponse_clear(&bResp);
  //printf("Done clearing\n");
}

void browse_node_path(UA_Client *client )
{
  std::vector<string> browse_path;

  UA_BrowsePath browsePath;
  UA_BrowsePath_init (&browsePath);
  browsePath.startingNode = UA_NODEID_NUMERIC (0, UA_NS0ID_OBJECTSFOLDER);
  browsePath.relativePath.elements = (UA_RelativePathElement *)UA_Array_new(browse_path.size (), &UA_TYPES[UA_TYPES_RELATIVEPATHELEMENT]);
  browsePath.relativePath.elementsSize = browse_path.size ();

  for (int i = 0; i < browse_path.size (); i++)
  {
    UA_RelativePathElement *elem = &browsePath.relativePath.elements[i];
    elem->targetName = UA_QUALIFIEDNAME_ALLOC (1, browse_path.at (i).c_str ()); // Create in server namespace (1), not UA namespace (0)!
  }
  UA_TranslateBrowsePathsToNodeIdsRequest request;
  UA_TranslateBrowsePathsToNodeIdsRequest_init (&request);
  request.browsePaths = &browsePath;
  request.browsePathsSize = 1;

  UA_TranslateBrowsePathsToNodeIdsResponse response;
  response = UA_Client_Service_translateBrowsePathsToNodeIds (client, request);
  if (UA_STATUSCODE_GOOD == response.responseHeader.serviceResult && response.resultsSize > 0)
  {
    UA_BrowsePathResult *first = response.results;
    if (first->targetsSize >= 1)
    {
      UA_NodeId id = first->targets[0].targetId.nodeId;
      std::cout << "Found ID";
    }
    else
    {
      std::cout << "OK response but no results";
    }
  }
  else
  {
    std::cout << "Error in translate browsename";
  }

  UA_BrowsePath_deleteMembers (&browsePath); // Marked as deprecated, but UA_BrowsePath_delete() expects a heap-allocated pointer.
  UA_TranslateBrowsePathsToNodeIdsResponse_deleteMembers (&response); // Idem

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

int main(int argc, char **argv)
{
  spdlog::set_pattern("cib : [%^%L%$] %v");
  spdlog::set_level(spdlog::level::trace); // Set global log level to info

  bool sort_by_type = false;
  NodeReportType type_filter = NodeReportType::all;

  for (int i = 1; i < argc; ++i)
  {
    const std::string arg(argv[i]);
    if (arg == "-s" || arg == "--sort")
    {
      sort_by_type = true;
      continue;
    }
    if (arg == "-h" || arg == "--help")
    {
      print_usage(argv[0]);
      return EXIT_SUCCESS;
    }
    if (arg == "-t" || arg == "--type")
    {
      if (i + 1 >= argc)
      {
        spdlog::error("Missing value for {}", arg);
        print_usage(argv[0]);
        return EXIT_FAILURE;
      }
      ++i;
      if (!parse_type_filter(argv[i], type_filter))
      {
        spdlog::error("Invalid node type '{}'. Use: variable, method, all", argv[i]);
        print_usage(argv[0]);
        return EXIT_FAILURE;
      }
      continue;
    }

    spdlog::error("Unknown argument '{}'", arg);
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

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
  BrowseReport report;
  browse_nodes_scan_cib(client, nullptr, type_filter, report);
  print_report(report, sort_by_type);


  UA_Client_disconnect(client);
  UA_Client_delete(client);
  return EXIT_SUCCESS;

}
