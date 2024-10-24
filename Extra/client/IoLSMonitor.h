#ifndef IOLSMONITOR_H
#define IOLSMONITOR_H

#include "Open62541Client.h"
#include <string>
#include <vector>
#include "iols_client_functions.h"

class IoLSMonitor
/**
 * @class IoLSMonitor
 * @brief A class to monitor and interact with an OPC UA server running on the CIB.
 *
 * This class provides methods to connect, configure, monitor, and interact with the CIB.
 * It uses the Open62541Client for communication with the server.
 *
 * @note The class assumes that the Open62541Client is properly implemented and available.
 */
{
public:

    /**
     * @brief Constructs an IoLSMonitor object with the specified server URL.
     * @param serverUrl The URL of the OPC UA server to connect to.
     *
     * @note The server URL should be in the format "opc.tcp://<hostname>:<port>".
     */
    IoLSMonitor(const std::string &serverUrl);

    /**
     * @brief Destructor for the IoLSMonitor class.
     */
    ~IoLSMonitor();

    /**
     * @brief Connects to the OPC UA server.
     * @return True if the connection is successful, false otherwise.
     */
    bool connect();

    /**
     * @brief Configures the IoLS system.
     * @return True if the configuration is successful, false otherwise.
     */
    bool config(std::string location);

    /**
     * @brief Monitors the IoLS system for a series of subscribed variables.
     *
     * @note This method should be run in a separate thread to continuously monitor the system.
     */
    void monitor_server();

    /**
     * @brief Disconnects from the OPC UA server.
     */
    void disconnect();

    // these are the methods that are implemented in the IoLS SC server
    bool move_to_position(const std::string position);
    bool fire_at_position(const std::string position, const uint32_t num_shots);
    bool fire_segment(const std::string start_position, const std::string end_position);
    bool execute_scan(const std::string run_plan);
    bool pause();
    bool standby();
    bool resume();
    bool warmup();
    bool shutdown();
    bool stop();
    void get_status(iols_monitor_t &status) { status = m_monitored_items; }

    private : 
      void parse_method_response(const std::string response);

    Open62541Client m_client;
    std::string m_serverUrl;
    iols_monitor_t m_monitored_items;
};

#endif // IOLSMONITOR_H