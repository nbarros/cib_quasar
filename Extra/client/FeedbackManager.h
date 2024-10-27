#ifndef FEEDBACKMANAGER_H
#define FEEDBACKMANAGER_H

#include <string>
#include <deque>
#include <vector>
#include "open62541.h"

enum class Severity
{
    INFO,
    WARN,
    ERROR
};

struct FeedbackMessage
{
    Severity severity;
    std::string message;
};

class FeedbackManager
{
public:
    FeedbackManager();
    ~FeedbackManager();

    void add_message(Severity severity, const std::string &message);
    std::vector<FeedbackMessage> get_messages(); // Retrieve and clear messages
    void clear_messages();
    UA_StatusCode get_global_status() const;
    void set_global_status(UA_StatusCode status);

private:
    std::deque<FeedbackMessage> m_messages;
    UA_StatusCode m_global_status;

    void update_global_status(Severity severity);
};

#endif // FEEDBACKMANAGER_H