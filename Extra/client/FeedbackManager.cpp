#include "FeedbackManager.h"

FeedbackManager::FeedbackManager()
    : m_global_status(UA_STATUSCODE_GOOD)
{
}

FeedbackManager::~FeedbackManager()
{
}

void FeedbackManager::add_message(Severity severity, const std::string &message)
{
    m_messages.push_back({severity, message});
    update_global_status(severity);
}

std::vector<FeedbackMessage> FeedbackManager::get_messages()
{
    std::vector<FeedbackMessage> messages(m_messages.begin(), m_messages.end());
    m_messages.clear();
    m_global_status = UA_STATUSCODE_GOOD; // Reset global status after retrieving messages
    return messages;
}

void FeedbackManager::clear_messages()
{
    m_messages.clear();
    m_global_status = UA_STATUSCODE_GOOD; // Reset global status after clearing messages
}

UA_StatusCode FeedbackManager::get_global_status() const
{
    return m_global_status;
}

void FeedbackManager::set_global_status(UA_StatusCode status)
{
    m_global_status = status;
}

void FeedbackManager::update_global_status(Severity severity)
{
    if (severity == Severity::ERROR)
    {
      m_global_status = UA_STATUSCODE_BADUNEXPECTEDERROR;
    }
    else if (severity == Severity::WARN && m_global_status != UA_STATUSCODE_BADUNEXPECTEDERROR)
    {
        m_global_status = UA_STATUSCODE_UNCERTAINDATASUBNORMAL;
    }
}