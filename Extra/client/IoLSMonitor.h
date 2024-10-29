#ifndef IOLSMONITOR_H
#define IOLSMONITOR_H

#include "Open62541Client.h"
#include "FeedbackManager.h"
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <json.hpp>
using json = nlohmann::json;

// Define the variant type using std::variant
using iols_opc_variant_t = std::variant<std::string, float, double, int32_t, uint32_t, int16_t, uint16_t,bool>;
using iols_monitor_t = std::map<std::string, iols_opc_variant_t>;

class IoLSMonitor
{
public:
    IoLSMonitor();
    ~IoLSMonitor();

    bool connect(const std::string &serverUrl, FeedbackManager &feedback);
    void disconnect(FeedbackManager &feedback);
    bool is_connected() const;

    bool config(const std::string &location, FeedbackManager &feedback);
    bool move_to_position(const std::string &position, const std::string &approach, FeedbackManager &feedback);
    bool fire_at_position(const std::string &position, const std::string &approach, const uint32_t num_shots, FeedbackManager &feedback);
    bool fire_segment(const std::string &start_position, const std::string &end_position, FeedbackManager &feedback);
    bool execute_scan(const std::string &run_plan, FeedbackManager &feedback);
    bool execute_grid_scan(const std::string &run_plan, FeedbackManager &feedback);
    bool pause(FeedbackManager &feedback);
    bool standby(FeedbackManager &feedback);
    bool resume(FeedbackManager &feedback);
    bool warmup(FeedbackManager &feedback);
    bool shutdown(FeedbackManager &feedback);
    bool clear_error(FeedbackManager &feedback);
    bool stop(FeedbackManager &feedback);
    void get_status(iols_monitor_t &status) { status = m_monitored_items; }
    void set_monitored_vars(const std::vector<std::string> &var_names);
    bool read_variable(const std::string &variable, UA_Variant &value, FeedbackManager &feedback);
    bool set_pm_range(const int16_t &selection, FeedbackManager &feedback);
    bool set_pm_threshold(const uint16_t &selection, FeedbackManager &feedback);
    bool set_dac_threshold(const uint16_t &selection, FeedbackManager &feedback);
    bool set_att_pos(const uint32_t &value, FeedbackManager &feedback);
    bool call_method(const std::string &method,const std::string &argument, const std::string &type, FeedbackManager &feedback) {return true;}
private:
    void monitor_loop();
    void monitor_server();
    bool exec_method_simple(const std::string &method_node, FeedbackManager &feedback);
    bool exec_method_arg(const std::string &method_node, const UA_Variant &val, FeedbackManager &feedback);
    void update_monitored_item(const std::string &key, const iols_opc_variant_t &value);

    Open62541Client m_client;
    std::string m_serverUrl;
    iols_monitor_t m_monitored_items;
    std::unordered_map<std::string, UA_Variant> m_monitored_vars;
    std::thread m_monitor_thread;
    std::atomic<bool> m_running;
    bool m_connected;
};

#endif // IOLSMONITOR_H