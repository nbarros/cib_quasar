#include <open62541.h>
#include <string>
#include <vector>
#include <spdlog/spdlog.h>
#include <stdexcept>

class Open62541Client
{
public:
    Open62541Client()
    {
        m_client = UA_Client_new();
        UA_ClientConfig_setDefault(UA_Client_getConfig(m_client));
    }

    ~Open62541Client()
    {
        disconnect();
        UA_Client_delete(m_client);
    }

    void connect(const std::string &endpoint)
    {
        UA_StatusCode status = UA_Client_connect(m_client, endpoint.c_str());
        if (status != UA_STATUSCODE_GOOD)
        {
            log_error("Failed to connect", status);
            throw std::runtime_error("Failed to connect to server: " + endpoint);
        }
    }

    void disconnect()
    {
        if (m_client)
        {
            UA_Client_disconnect(m_client);
        }
    }

    void read_variable(const std::string &nodeId, UA_Variant &value)
    {
        UA_NodeId node = UA_NODEID_STRING_ALLOC(1, nodeId.c_str());
        UA_StatusCode status = UA_Client_readValueAttribute(m_client, node, &value);
        UA_NodeId_clear(&node);
        if (status != UA_STATUSCODE_GOOD)
        {
            log_error("Failed to read variable", status);
            throw std::runtime_error("Failed to read variable: " + nodeId);
        }
    }

    void write_variable(const std::string &nodeId, const UA_Variant &value)
    {
        UA_NodeId node = UA_NODEID_STRING_ALLOC(1, nodeId.c_str());
        UA_StatusCode status = UA_Client_writeValueAttribute(m_client, node, &value);
        UA_NodeId_clear(&node);
        if (status != UA_STATUSCODE_GOOD)
        {
            log_error("Failed to write variable", status);
            throw std::runtime_error("Failed to write variable: " + nodeId);
        }
    }

    void browse(const std::string &nodeId, std::vector<UA_BrowseResult> &results)
    {
        UA_NodeId node = UA_NODEID_STRING_ALLOC(1, nodeId.c_str());
        UA_BrowseRequest bReq;
        UA_BrowseRequest_init(&bReq);
        bReq.requestedMaxReferencesPerNode = 0;
        bReq.nodesToBrowse = UA_BrowseDescription_new();
        bReq.nodesToBrowseSize = 1;
        bReq.nodesToBrowse[0].nodeId = node;
        bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;

        UA_BrowseResponse bResp = UA_Client_Service_browse(m_client, bReq);
        results.push_back(bResp.results[0]);

        UA_BrowseRequest_clear(&bReq);
        UA_NodeId_clear(&node);
        if (bResp.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
        {
            log_error("Failed to browse", bResp.responseHeader.serviceResult);
            throw std::runtime_error("Failed to browse nodes: " + nodeId);
        }
    }

    void call_method(const std::string &objectId, const std::string &methodId, const std::vector<UA_Variant> &inputArguments, std::vector<UA_Variant> &outputArguments)
    {
        UA_NodeId objectNodeId = UA_NODEID_STRING_ALLOC(1, objectId.c_str());
        UA_NodeId methodNodeId = UA_NODEID_STRING_ALLOC(1, methodId.c_str());

        UA_CallRequest callRequest;
        UA_CallRequest_init(&callRequest);
        callRequest.methodsToCallSize = 1;
        callRequest.methodsToCall = UA_CallMethodRequest_new();
        callRequest.methodsToCall[0].objectId = objectNodeId;
        callRequest.methodsToCall[0].methodId = methodNodeId;
        callRequest.methodsToCall[0].inputArgumentsSize = inputArguments.size();
        callRequest.methodsToCall[0].inputArguments = const_cast<UA_Variant *>(inputArguments.data());

        UA_CallResponse callResponse = UA_Client_Service_call(m_client, callRequest);
        for (size_t i = 0; i < callResponse.resultsSize; ++i)
        {
            outputArguments.push_back(callResponse.results[i].outputArguments[0]);
        }

        UA_CallRequest_clear(&callRequest);
        UA_NodeId_clear(&objectNodeId);
        UA_NodeId_clear(&methodNodeId);
        if (callResponse.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
        {
            log_error("Failed to call method", callResponse.responseHeader.serviceResult);
            throw std::runtime_error("Failed to call method: " + methodId + " on object: " + objectId);
        }
    }

private:
    UA_Client *m_client;

    void log_error(const std::string &message, UA_StatusCode status)
    {
        spdlog::critical("{}: {}", message, UA_StatusCode_name(status));
    }
};
