#include "Open62541Client.h"

Open62541Client::Open62541Client()
  : m_client(UA_Client_new()), m_connected(false)
{
  UA_ClientConfig_setDefault(UA_Client_getConfig(m_client));
}

Open62541Client::~Open62541Client()
{
  if (m_client)
  {
    FeedbackManager feedback;
    disconnect(feedback);
    UA_Client_delete(m_client);
  }
}

bool Open62541Client::connect(const std::string &endpointUrl, FeedbackManager &feedback)
{
  std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex

  if (m_connected)
  {
    log_error("Already connected to server", UA_STATUSCODE_BADCONNECTIONCLOSED, feedback);
    return false;
  }

  UA_StatusCode status = UA_Client_connect(m_client, endpointUrl.c_str());
  if (status != UA_STATUSCODE_GOOD)
  {
    log_error("Failed to connect to server", status, feedback);
    return false;
  }
  m_connected = true;
  feedback.add_message(Severity::INFO, "Successfully connected to server.");
  return true;
}

void Open62541Client::disconnect(FeedbackManager &feedback)
{
  std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex

  if (m_connected)
  {
    UA_Client_disconnect(m_client);
    m_connected = false;
    feedback.add_message(Severity::INFO, "Disconnected from server.");
  }
}

bool Open62541Client::is_connected() const
{
  std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex
  return m_connected;
}

bool Open62541Client::read_variable(const std::string &nodeId, UA_Variant &value, FeedbackManager &feedback)
{
  std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex

  if (!m_connected)
  {
    log_error("Client is not connected", UA_STATUSCODE_BADCONNECTIONCLOSED, feedback);
    return false;
  }
  UA_StatusCode status = UA_Client_readValueAttribute(m_client, UA_NODEID_STRING_ALLOC(2, nodeId.c_str()), &value);
  if (status != UA_STATUSCODE_GOOD)
  {
    log_error("Failed to read variable.", status, feedback);
    return false;
  }
  feedback.add_message(Severity::INFO, "Successfully read variable: " + nodeId);
  return true;
}

bool Open62541Client::write_variable(const std::string &nodeId, const UA_Variant &value, FeedbackManager &feedback)
{
  std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex

  if (!m_connected)
  {
    log_error("Client is not connected", UA_STATUSCODE_BADCONNECTIONCLOSED, feedback);
    return false;
  }
  UA_StatusCode status = UA_Client_writeValueAttribute(m_client, UA_NODEID_STRING_ALLOC(2, nodeId.c_str()), &value);
  if (status != UA_STATUSCODE_GOOD)
  {
    log_error("Failed to write variable", status, feedback);
    return false;
  }
  feedback.add_message(Severity::INFO, "Successfully wrote variable: " + nodeId);
  return true;
}

bool Open62541Client::browse(const std::string &nodeId, std::vector<UA_BrowseResult> &results, FeedbackManager &feedback)
{
  std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex

  UA_BrowseRequest bReq;
  UA_BrowseRequest_init(&bReq);
  bReq.nodesToBrowse = UA_BrowseDescription_new();
  bReq.nodesToBrowseSize = 1;
  bReq.nodesToBrowse[0].nodeId = UA_NODEID_STRING_ALLOC(2, nodeId.c_str());
  bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;

  UA_BrowseResponse bResp = UA_Client_Service_browse(m_client, bReq);
  if (bResp.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
  {
    UA_BrowseRequest_clear(&bReq);
    UA_BrowseResponse_clear(&bResp);
    log_error("Failed to browse", bResp.responseHeader.serviceResult, feedback);
    return false;
  }

  results.assign(bResp.results, bResp.results + bResp.resultsSize);
  feedback.add_message(Severity::INFO, "Successfully browsed node: " + nodeId);

  UA_BrowseRequest_clear(&bReq);
  UA_BrowseResponse_clear(&bResp);
  return true;
}

void Open62541Client::call_method(const std::string &objectId, const std::string &methodId, const std::vector<UA_Variant> &inputArguments, std::vector<UA_Variant> &outputArguments, FeedbackManager &feedback)
{
  std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex
  size_t outputSize;
  UA_Variant *output;

  UA_Variant input;
  UA_Variant_init(&input);

  UA_Variant *in_ptr;
  if (inputArguments.size() == 0)
  {
    in_ptr = &input;
  }
  else
  {
    in_ptr = const_cast<UA_Variant *>(inputArguments.data());
  }

  UA_StatusCode retval = UA_Client_call(m_client, 
                                        UA_NODEID_STRING_ALLOC(2, objectId.c_str()), 
                                        UA_NODEID_STRING_ALLOC(2, methodId.c_str()), 
                                        inputArguments.size(), 
                                        in_ptr, 
                                        &outputSize, &output);

  if (retval == UA_STATUSCODE_GOOD)
  {
    feedback.add_message(Severity::INFO, "Method called successfully. Returned " + std::to_string(outputSize) + " arguments (1 expected)");
    outputArguments.push_back(output[0]);
    // // actually, this returns a typical response string
    // std::string response((char *)static_cast<UA_String *>(output[0].data)->data, (size_t)static_cast<UA_String *>(output[0].data)->length);
    //UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
  }
  else
  {
    feedback.add_message(Severity::ERROR, "Failed to execute method. Got error " + std::to_string(retval) + " : " + UA_StatusCode_name(retval));
  }
  //
}

void Open62541Client::call_method_old(const std::string &objectId, const std::string &methodId, const std::vector<UA_Variant> &inputArguments, std::vector<UA_Variant> &outputArguments, FeedbackManager &feedback)
{
  std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex

  UA_CallRequest callRequest;
  UA_CallRequest_init(&callRequest);
  callRequest.methodsToCallSize = 1;
  callRequest.methodsToCall = UA_CallMethodRequest_new();
  callRequest.methodsToCall[0].objectId = UA_NODEID_STRING_ALLOC(2, objectId.c_str());
  callRequest.methodsToCall[0].methodId = UA_NODEID_STRING_ALLOC(2, methodId.c_str());
  callRequest.methodsToCall[0].inputArgumentsSize = inputArguments.size();
  callRequest.methodsToCall[0].inputArguments = const_cast<UA_Variant *>(inputArguments.data());

  UA_CallResponse callResponse = UA_Client_Service_call(m_client, callRequest);
  if (callResponse.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
  {
    printf("Failed to call method: %s\n", UA_StatusCode_name(callResponse.responseHeader.serviceResult));
    UA_CallRequest_clear(&callRequest);
    UA_CallResponse_clear(&callResponse);
    
    log_error("Failed to call method", callResponse.responseHeader.serviceResult, feedback);
    throw std::runtime_error("Failed to call method: " + methodId + " on object: " + objectId);
  }
  printf("Dealing with a response\n");
  outputArguments.assign(callResponse.results[0].outputArguments, callResponse.results[0].outputArguments + callResponse.results[0].outputArgumentsSize);
  feedback.add_message(Severity::INFO, "Successfully called method: " + methodId + " on object: " + objectId);

  UA_CallRequest_clear(&callRequest);
  UA_CallResponse_clear(&callResponse);
}

void Open62541Client::log_error(const std::string &message, UA_StatusCode status, FeedbackManager &feedback)
{
  feedback.add_message(Severity::ERROR, message + ": " + UA_StatusCode_name(status));
}