#include "IoLSMonitor.h"
#include <spdlog/spdlog.h>

IoLSMonitor::IoLSMonitor(const std::string &serverUrl)
    : m_serverUrl(serverUrl)
{
}

IoLSMonitor::~IoLSMonitor()
{
    disconnect();
}

bool IoLSMonitor::connect()
{
    try
    {
        m_client.connect(m_serverUrl);
        return true;
    }
    catch (const std::exception &e)
    {
        spdlog::critical("Failed to connect to server: {}", e.what());
        return false;
    }
}

void IoLSMonitor::disconnect()
{
    m_client.disconnect();
}

bool IoLSMonitor::readVariable(const std::string &nodeId, UA_Variant &value)
{
    try
    {
        m_client.read_variable(nodeId, value);
        return true;
    }
    catch (const std::exception &e)
    {
        spdlog::critical("Failed to read variable: {}", e.what());
        return false;
    }
}

bool IoLSMonitor::writeVariable(const std::string &nodeId, const UA_Variant &value)
{
    try
    {
        m_client.write_variable(nodeId, value);
        return true;
    }
    catch (const std::exception &e)
    {
        spdlog::critical("Failed to write variable: {}", e.what());
        return false;
    }
}

bool IoLSMonitor::browse(const std::string &nodeId, std::vector<UA_BrowseResult> &results)
{
    try
    {
        m_client.browse(nodeId, results);
        return true;
    }
    catch (const std::exception &e)
    {
        spdlog::critical("Failed to browse: {}", e.what());
        return false;
    }
}

bool IoLSMonitor::callMethod(const std::string &objectId, const std::string &methodId, const std::vector<UA_Variant> &inputArguments, std::vector<UA_Variant> &outputArguments)
{
    try
    {
        m_client.call_method(objectId, methodId, inputArguments, outputArguments);
        return true;
    }
    catch (const std::exception &e)
    {
        spdlog::critical("Failed to call method: {}", e.what());
        return false;
    }
}