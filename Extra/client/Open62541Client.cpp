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
        disconnect();
        UA_Client_delete(m_client);
    }
}

bool Open62541Client::connect(const std::string &endpoint)
{
    if (m_connected)
    {
        log_error("Already connected to server", UA_STATUSCODE_BADCONNECTIONCLOSED);
        return false;
    }

    UA_StatusCode status = UA_Client_connect(m_client, endpoint.c_str());
    if (status != UA_STATUSCODE_GOOD)
    {
        log_error("Failed to connect to server", status);
        return false;
    }
    m_connected = true;
    return true;
}

void Open62541Client::disconnect()
{
    if (m_connected)
    {
        UA_Client_disconnect(m_client);
        m_connected = false;
    }
}

bool Open62541Client::is_connected() const
{
    return m_connected;
}

void Open62541Client::read_variable(const std::string &nodeId, UA_Variant &value)
{
    UA_StatusCode status = UA_Client_readValueAttribute(m_client, UA_NODEID_STRING_ALLOC(2,nodeId.c_str()), &value);
    if (status != UA_STATUSCODE_GOOD)
    {
        log_error("Failed to read variable", status);
        throw std::runtime_error("Failed to read variable: " + nodeId);
    }
}

void Open62541Client::write_variable(const std::string &nodeId, const UA_Variant &value)
{
    UA_StatusCode status = UA_Client_writeValueAttribute(m_client, UA_NODEID_STRING_ALLOC(2,nodeId.c_str()), &value);
    if (status != UA_STATUSCODE_GOOD)
    {
        log_error("Failed to write variable", status);
        throw std::runtime_error("Failed to write variable: " + nodeId);
    }
}

void Open62541Client::browse(const std::string &nodeId, std::vector<UA_BrowseResult> &results)
{
    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);
    bReq.requestedMaxReferencesPerNode = 0;
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;
    bReq.nodesToBrowse[0].nodeId = UA_NODEID_STRING_ALLOC(2,nodeId.c_str());
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;

    UA_BrowseResponse bResp = UA_Client_Service_browse(m_client, bReq);
    if (bResp.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
    {
      UA_BrowseRequest_clear(&bReq);
      UA_BrowseResponse_clear(&bResp);
      log_error("Failed to browse", bResp.responseHeader.serviceResult);
      throw std::runtime_error("Failed to browse: " + nodeId);
    }

    results.assign(bResp.results, bResp.results + bResp.resultsSize);

    UA_BrowseRequest_clear(&bReq);
    UA_BrowseResponse_clear(&bResp);
}

void Open62541Client::call_method(const std::string &objectId, const std::string &methodId, const std::vector<UA_Variant> &inputArguments, std::vector<UA_Variant> &outputArguments)
{
    UA_CallRequest callRequest;
    UA_CallRequest_init(&callRequest);
    callRequest.methodsToCallSize = 1;
    callRequest.methodsToCall = UA_CallMethodRequest_new();
    callRequest.methodsToCall[0].objectId = UA_NODEID_STRING_ALLOC(2,objectId.c_str());
    callRequest.methodsToCall[0].methodId = UA_NODEID_STRING_ALLOC(2,methodId.c_str());
    callRequest.methodsToCall[0].inputArgumentsSize = inputArguments.size();
    callRequest.methodsToCall[0].inputArguments = const_cast<UA_Variant *>(inputArguments.data());

    UA_CallResponse callResponse = UA_Client_Service_call(m_client, callRequest);
    if (callResponse.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
    {
      UA_CallRequest_clear(&callRequest);
      UA_CallResponse_clear(&callResponse);

      log_error("Failed to call method", callResponse.responseHeader.serviceResult);
      throw std::runtime_error("Failed to call method: " + methodId + " on object: " + objectId);
    }

    outputArguments.assign(callResponse.results[0].outputArguments, callResponse.results[0].outputArguments + callResponse.results[0].outputArgumentsSize);

    UA_CallRequest_clear(&callRequest);
    UA_CallResponse_clear(&callResponse);
}

void Open62541Client::log_error(const std::string &message, UA_StatusCode status)
{
  m_feedback_messages.push_back(message + ": " + UA_StatusCode_name(status));
}

std::deque<std::string> Open62541Client::get_feedback_messages()
{
  std::deque<std::string> messages = std::move(m_feedback_messages);
  m_feedback_messages.clear();
  return messages;
}