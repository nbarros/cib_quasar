#ifndef OPEN62541CLIENT_H
#define OPEN62541CLIENT_H

#include <open62541.h>
#include <string>
#include <vector>
#include <spdlog/spdlog.h>
#include <stdexcept>

class Open62541Client
{
public:
    Open62541Client();
    ~Open62541Client();

    void connect(const std::string &endpoint);
    void disconnect();
    bool is_connected() const;
    void read_variable(const std::string &nodeId, UA_Variant &value);
    void write_variable(const std::string &nodeId, const UA_Variant &value);
    void browse(const std::string &nodeId, std::vector<UA_BrowseResult> &results);
    void call_method(const std::string &objectId, const std::string &methodId, const std::vector<UA_Variant> &inputArguments, std::vector<UA_Variant> &outputArguments);

private:
    UA_Client *m_client;
    bool m_connected;

    void log_error(const std::string &message, UA_StatusCode status);
};

#endif // OPEN62541CLIENT_H
