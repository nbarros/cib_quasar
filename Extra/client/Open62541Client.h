#ifndef OPEN62541CLIENT_H
#define OPEN62541CLIENT_H

#include "open62541.h"
#include "FeedbackManager.h"
#include <string>
#include <vector>
#include <deque>
#include <stdexcept>
#include <mutex>

class Open62541Client
{
public:
    Open62541Client();
    ~Open62541Client();

    bool connect(const std::string &endpoint, FeedbackManager &feedback);
    void disconnect(FeedbackManager &feedback);
    bool is_connected() const;
    bool read_variable(const std::string &nodeId, UA_Variant &value, FeedbackManager &feedback);
    bool write_variable(const std::string &nodeId, const UA_Variant &value, FeedbackManager &feedback);
    bool browse(const std::string &nodeId, std::vector<UA_BrowseResult> &results, FeedbackManager &feedback);
    void call_method_old(const std::string &objectId, const std::string &methodId, const std::vector<UA_Variant> &inputArguments, std::vector<UA_Variant> &outputArguments, FeedbackManager &feedback);
    void call_method(const std::string &objectId, const std::string &methodId, const std::vector<UA_Variant> &inputArguments, std::vector<UA_Variant> &outputArguments, FeedbackManager &feedback);

        private : UA_Client *m_client;
    bool m_connected;
    mutable std::mutex m_mutex; // Mutex to protect shared resources

    void log_error(const std::string &message, UA_StatusCode status, FeedbackManager &feedback);
};

#endif // OPEN62541CLIENT_H
