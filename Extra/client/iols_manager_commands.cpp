#include "iols_manager.h"
#include "IoLSMonitor.h"

extern int g_height;
extern IoLSMonitor g_monitor;
extern std::deque<FeedbackMessage> g_feedback;
extern std::vector<std::string> g_vars_to_monitor;

void  print_help()
{
  add_feedback(Severity::INFO, "Available commands:");
  add_feedback(Severity::INFO, "  help");
  add_feedback(Severity::INFO, "       Prints this help");
  add_feedback(Severity::INFO, "   connect <server>");
  add_feedback(Severity::INFO, "       Connect to a server. Options are:");
  add_feedback(Severity::INFO, "         cib1 : connects to P1");
  add_feedback(Severity::INFO, "         cib2 : connects to P2");
  add_feedback(Severity::INFO, "         opc.tcp://11.22.33.44:5555 : connects to a server in another location");
  add_feedback(Severity::INFO, "   disconnect");
  add_feedback(Severity::INFO, "       Disconnect from the server");
  add_feedback(Severity::INFO, "   shutdown");
  add_feedback(Severity::INFO, "       Shutdown the IoLS system");
  add_feedback(Severity::INFO, "   config <location>");
  add_feedback(Severity::INFO, "       Configure the IoLS system. Location points to the configuration file");
  add_feedback(Severity::INFO, "   move_to_position <position> [approach]");
  add_feedback(Severity::INFO, "       Move to a specified position. Approach is optional");
  add_feedback(Severity::INFO, "       Example: move_to_position [155,256,367] uud");
  add_feedback(Severity::INFO, "   warmup");
  add_feedback(Severity::INFO, "       Start warmup of the laser. During this stage only motors can be moved.");
  add_feedback(Severity::INFO, "   pause");
  add_feedback(Severity::INFO, "       Pause the system. This will *keep* the laser firing, but shutter is closed.");
  add_feedback(Severity::INFO, "   standby");
  add_feedback(Severity::INFO, "       Pause the system. This will close the internal shutter and stop QSWITCH.");
  add_feedback(Severity::INFO, "   resume");
  add_feedback(Severity::INFO, "       Resume the system. This will open the shutters and start QSWITCH.");
  add_feedback(Severity::INFO, "   stop");
  add_feedback(Severity::INFO, "       Stop the system. This will stop the system (fire, qswitch, and return shutters to default position).");
  add_feedback(Severity::INFO, "   fire_at_position <position> <approach> <num_shots>");
  add_feedback(Severity::INFO, "       Fire at a specified position. Number of shots is optional.");
  add_feedback(Severity::INFO, "       Example: fire_at_position [1,2,3] uud 10");
  add_feedback(Severity::INFO, "   fire_segment <start_position> <end_position>");
  add_feedback(Severity::INFO, "       Fire at a segment between two positions.");
  add_feedback(Severity::INFO, "       Example: fire_segment [1,2,3] [4,5,6]");
  add_feedback(Severity::INFO, "   execute_scan <run_plan>");
  add_feedback(Severity::INFO, "       Execute a scan plan. The run plan should be a JSON object with a 'scan_plan' array.");
  add_feedback(Severity::INFO, "       Example: execute_scan '{\"scan_plan\":[{\"start\":[1,2,3],\"end\":[4,5,6]}, {\"start\":[7,8,9],\"end\":[10,11,12]}]}'");
  add_feedback(Severity::INFO, "   grid_scan <run_plan>");
  add_feedback(Severity::INFO, "       Generate and execute a scan plan.");
  add_feedback(Severity::INFO, "       Example: grid_scan '{\"center\":[1,2,3],\"range\":[0,1000,1000],\"step\":[0,100,1000],\"approach\":\"uuu\", \"scan_axis\":1}'");
  // add_feedback(Severity::INFO, "   add_monitor <variable>");
  // add_feedback(Severity::INFO, "       Add a variable to the monitor list. Variable must be a fully qualified OPC-UA node");
  add_feedback(Severity::INFO, "   read_var <variable>");
  add_feedback(Severity::INFO, "       Read the value of a variable. Variable must be a fully qualified OPC-UA node");
  // add_feedback(Severity::INFO, "   write_var <variable> <value> <type>");
  // add_feedback(Severity::INFO, "       Read the value of a variable. Variable must be a fully qualified OPC-UA node");
  add_feedback(Severity::INFO, "   set_pm_range <setting>");
  add_feedback(Severity::INFO, "       Check the variable LS1.PM1.range_options for reference");
  add_feedback(Severity::INFO, "   set_pm_threshold <setting>");
  add_feedback(Severity::INFO, "       Threshold should be something above 100 (units of 0.01%)");
  add_feedback(Severity::INFO, "   set_att_position <setting>");
  add_feedback(Severity::INFO, "       Position should be a value in [-10000; 10000]");
  add_feedback(Severity::INFO, "   set_dac <value>");
  add_feedback(Severity::INFO, "       Value cannot be above 4095");
  add_feedback(Severity::INFO, "   clear_error");
  add_feedback(Severity::INFO, "       Clears IoLS error state, returning task messages");
  add_feedback(Severity::INFO, "  exit");
  add_feedback(Severity::INFO, "       Exit the program");
}

/**
 * @brief Executes a command based on the provided arguments.
 *
 * This function processes various commands such as "exit", "connect", "disconnect", and "help".
 * It performs actions like connecting to a server, disconnecting from a server, shutting down
 * the monitor, and printing help information.
 *
 * @param argc The number of arguments.
 * @param argv The array of arguments.
 * @return An integer status code indicating the result of the command execution.
 *         - 255: Indicates the "exit" command was executed.
 *         - 1: Indicates insufficient arguments were provided.
 *         - 0: Indicates successful execution of the command or an error message was provided.
 *
 * Commands:
 * - "exit": Shuts down and disconnects the monitor if it is running.
 * - "connect <server>": Connects to the specified server. If the server is "cib1" or "cib2",
 *   it connects to predefined addresses.
 * - "disconnect": Disconnects from the server if connected and the system is not running.
 * - "help": Prints help information.
 * - Any other command: Provides an "Unknown command" feedback.
 *
 * @note The global variable `g_monitor` is used to manage the connection state.
 * @note The `status` parameter is a std::map<std::string, std::variant<std::string, double, int>>.
 */
int run_command(int argc, char **argv)
{
  if (argc < 1)
  {
    return 1;
  }
  std::string cmd(argv[0]);
  // check command request
  if (cmd == "exit")
  {
    if (g_monitor.is_connected())
    {
      FeedbackManager feedback;
      g_monitor.disconnect(feedback);
      std::vector<FeedbackMessage> messages = feedback.get_messages();
      update_feedback(messages);

      // g_monitor->shutdown(feedback);
      //  std::vector<FeedbackMessage> messages = feedback.get_messages();
      //  update_feedback(messages);
      //  g_monitor->disconnect(feedback);
      //  messages = feedback.get_messages();
      //  update_feedback(messages);
      //  delete g_monitor;
      //  g_monitor = nullptr;
    }
    return 255;
  }
  else if (cmd == "connect")
  {
    if (g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Already connected to a server. Disconnect first.");
      return 0;
    }
    if (argc < 2)
    {
      add_feedback(Severity::WARN, "Usage: connect <server>");
      return 0;
    }
    else
    {
      std::string server(argv[1]);
      if (server == "cib1")
      {
        server = "opc.tcp://10.73.137.147:4841";
      }
      else if (server == "cib2")
      {
        server = "opc.tcp://10.73.137.148:4841";
      }
      add_feedback(Severity::INFO, "Connecting to server: " + server);
      // g_monitor = new IoLSMonitor(server);
      FeedbackManager feedback;
      bool res = g_monitor.connect(server, feedback);
      std::vector<FeedbackMessage> messages = feedback.get_messages();
      update_feedback(messages);
      if (res)
      {
        add_feedback(Severity::INFO, "Connected to server.");
        // update the variables that are to be kept under surveillance
        g_monitor.set_monitored_vars(g_vars_to_monitor);
      }
      else
      {
        add_feedback(Severity::ERROR, "Failed to connect to server.");
      }
      // start connection
      return 0;
    }
  }
  else if (cmd == "disconnect")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    // iols_monitor_t status;
    // g_monitor->get_status(status);
    // if (status.count("LS1.state") != 0)
    // {
    //   if ((std::get<std::string>(status["LS1.state"]) != "offline") &&
    //       (std::get<std::string>(status["LS1.state"]) != "unknown"))
    //   {
    //     update_feedback("Cannot disconnect while the system is running. Call 'shutdown' first.");
    //     return 0;
    //   }
    // }
    FeedbackManager feedback;
    g_monitor.disconnect(feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    add_feedback(Severity::INFO, "Disconnecting from server.");
    // delete g_monitor;
    // g_monitor = nullptr;
    return 0;
  }
  else if (cmd == "shutdown")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    add_feedback(Severity::INFO, "Shutting down the system.");
    FeedbackManager feedback;
    g_monitor.shutdown(feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    return 0;
  }
  else if (cmd == "config")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    if (argc < 2)
    {
      add_feedback(Severity::WARN, "Usage: config <location>");
      return 0;
    }
    std::string location(argv[1]);
    FeedbackManager feedback;
    bool res = g_monitor.config(location, feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    if (res)
    {
      add_feedback(Severity::INFO, "Configuration successful.");
    }
    else
    {
      add_feedback(Severity::ERROR, "Configuration failed.");
    }
    return 0;
  }
  else if (cmd == "move_to_position")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    if (argc < 2)
    {
      add_feedback(Severity::WARN, "Usage: move_to_position <position>");
      return 0;
    }
    std::string position(argv[1]);
    FeedbackManager feedback;
    std::string approach = "---";
    if (argc == 3)
    {
      approach = argv[2];
    }
    bool res = g_monitor.move_to_position(position, approach, feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    if (res)
    {
      add_feedback(Severity::INFO, "Move to position successful.");
    }
    else
    {
      add_feedback(Severity::ERROR, "Move to position failed.");
    }
  }
  else if (cmd == "warmup")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    FeedbackManager feedback;
    bool res = g_monitor.warmup(feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    if (res)
    {
      add_feedback(Severity::INFO, "Warmup successful.");
    }
    else
    {
      add_feedback(Severity::ERROR, "Warmup failed.");
    }
  }
  else if (cmd == "pause")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    FeedbackManager feedback;
    bool res = g_monitor.pause(feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    if (res)
    {
      add_feedback(Severity::INFO, "Pause successful.");
    }
    else
    {
      add_feedback(Severity::ERROR, "Pause failed.");
    }
  }
  else if (cmd == "standby")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    FeedbackManager feedback;
    bool res = g_monitor.standby(feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    if (res)
    {
      add_feedback(Severity::INFO, "Standby successful.");
    }
    else
    {
      add_feedback(Severity::ERROR, "Standby failed.");
    }
  }
  else if (cmd == "resume")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    FeedbackManager feedback;
    bool res = g_monitor.resume(feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    if (res)
    {
      add_feedback(Severity::INFO, "Resume successful.");
    }
    else
    {
      add_feedback(Severity::ERROR, "Resume failed.");
    }
  }
  else if (cmd == "stop")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    FeedbackManager feedback;
    bool res = g_monitor.stop(feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    if (res)
    {
      add_feedback(Severity::INFO, "Stop successful.");
    }
    else
    {
      add_feedback(Severity::ERROR, "Stop failed.");
    }
  }
  else if (cmd == "fire_at_position")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    if (argc != 4)
    {
      add_feedback(Severity::WARN, "Usage: fire_at_position <position> <approach> <num_shots>");
      return 0;
    }
    std::string position(argv[1]);
    std::string approach(argv[2]);
    uint32_t num_shots = std::stoi(argv[3]);
    FeedbackManager feedback;
    bool res = g_monitor.fire_at_position(position, approach, num_shots, feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    if (res)
    {
      add_feedback(Severity::INFO, "Fire at position successful.");
    }
    else
    {
      add_feedback(Severity::ERROR, "Fire at position failed.");
    }
  }
  else if (cmd == "fire_segment")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    if (argc != 3)
    {
      add_feedback(Severity::WARN, "Usage: fire_segment <start_position> <end_position>");
      return 0;
    }
    std::string start_position(argv[1]);
    std::string end_position(argv[2]);
    FeedbackManager feedback;
    bool res = g_monitor.fire_segment(start_position, end_position, feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    if (res)
    {
      add_feedback(Severity::INFO, "Fire segment successful.");
    }
    else
    {
      add_feedback(Severity::ERROR, "Fire segment failed.");
    }
  }
  else if (cmd == "execute_scan")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    if (argc != 2)
    {
      add_feedback(Severity::WARN, "Usage: execute_scan <run_plan>");
      return 0;
    }
    std::string run_plan(argv[1]);
    FeedbackManager feedback;
    bool res = g_monitor.execute_scan(run_plan, feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    if (res)
    {
      add_feedback(Severity::INFO, "Execute scan successful.");
    }
    else
    {
      add_feedback(Severity::ERROR, "Execute scan failed.");
    }
  }
  else if (cmd == "grid_scan")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    if (argc != 2)
    {
      add_feedback(Severity::WARN, "Usage: grid_scan <run_plan>");
      return 0;
    }
    std::string run_plan(argv[1]);
    FeedbackManager feedback;
    bool res = g_monitor.execute_grid_scan(run_plan, feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    if (res)
    {
      add_feedback(Severity::INFO, "Execute scan successful.");
    }
    else
    {
      add_feedback(Severity::ERROR, "Execute scan failed.");
    }
  }
  else if (cmd == "shutdown")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    add_feedback(Severity::INFO, "Shutting down the system.");
    FeedbackManager feedback;
    g_monitor.shutdown(feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    return 0;
  }
  else if (cmd == "clear_error")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    add_feedback(Severity::INFO, "Clearing error state.");
    FeedbackManager feedback;
    g_monitor.clear_error(feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    return 0;
  }
  else if (cmd == "help")
  {
    print_help();
    return 0;
  }
  else if (cmd == "write_var")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    if (argc != 4)
    {
      add_feedback(Severity::WARN, "Usage: write_var <node> <value> <type>");
      return 0;
    }
    std::string node = argv[1];
    std::string value = argv[2];
    std::string type = argv[3];
    FeedbackManager feedback;
    bool success = g_monitor.write_variable(node, value, type, feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    if (success)
    {
      add_feedback(Severity::INFO, "Warmup timer target set successfully.");
    }
    else
    {
      add_feedback(Severity::ERROR, "Failed to set warmup timer target.");
    }
    return 0;
  }
  else if (cmd == "set_warmup_timer")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    if (argc != 2)
    {
      add_feedback(Severity::WARN, "Usage: set_warmup_timer <value>");
      return 0;
    }
    std::string value =argv[1];
    std::string type = "u32";
    FeedbackManager feedback;
    bool success = g_monitor.write_variable("LS1.L1.warmup_target_min", value, type, feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    if (success)
    {
      add_feedback(Severity::INFO, "Warmup timer target set successfully.");
    }
    else
    {
      add_feedback(Severity::ERROR, "Failed to set warmup timer target.");
    }
    return 0;    
  }
  else if (cmd == "read_var")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    if (argc != 2)
    {
      add_feedback(Severity::WARN, "Usage: read_var <variable>");
      return 0;
    }
    std::string variable(argv[1]);
    UA_Variant value;
    UA_Variant_init(&value);

    FeedbackManager feedback;
    g_monitor.read_variable(variable, value, feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    std::ostringstream msg;
    msg << variable << ": ";
    if (value.type == &UA_TYPES[UA_TYPES_STRING])
    {
      msg << std::string((char *)static_cast<UA_String *>(value.data)->data, (size_t) static_cast<UA_String *>(value.data)->length);
    }
    else if (value.type == &UA_TYPES[UA_TYPES_DOUBLE])
    {
      msg << *static_cast<double *>(value.data);
    }
    else if (value.type == &UA_TYPES[UA_TYPES_FLOAT])
    {
      msg << *static_cast<float *>(value.data);
    }
    else if (value.type == &UA_TYPES[UA_TYPES_INT32])
    {
      msg << *static_cast<int32_t *>(value.data);
    }
    else if (value.type == &UA_TYPES[UA_TYPES_UINT32])
    {
      msg << *static_cast<uint32_t *>(value.data);
    }
    else if (value.type == &UA_TYPES[UA_TYPES_UINT16])
    {
      msg << *static_cast<uint16_t *>(value.data);
    }
    else if (value.type == &UA_TYPES[UA_TYPES_BOOLEAN])
    {
      msg << *static_cast<bool *>(value.data);
    }
    else
    {
      msg << "Unknown type";
    }
    add_feedback(Severity::INFO, msg.str());
    UA_Variant_clear(&value);
  }
  else if (cmd == "set_pm_range")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    if (argc != 2)
    {
      add_feedback(Severity::WARN, "Usage: set_pm_range <setting>");
      add_feedback(Severity::WARN, "      Check the variable LS1.PM1.range_options for reference");
      return 0;
    }
    int16_t setting = std::stoi(argv[1]);
    FeedbackManager feedback;
    g_monitor.set_pm_range(setting, feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
  }
  else if (cmd == "set_pm_threshold")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    if (argc != 2)
    {
      add_feedback(Severity::WARN, "Usage: set_pm_threshold <setting>");
      add_feedback(Severity::WARN, "      Threshold should be something above 100");
      return 0;
    }
    uint16_t setting = std::stoi(argv[1]);
    FeedbackManager feedback;
    g_monitor.set_pm_threshold(setting, feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
  }
  else if (cmd == "set_att_position")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    if (argc != 2)
    {
      add_feedback(Severity::WARN, "Usage: set_att_position <setting>");
      add_feedback(Severity::WARN, "      Position should be a value in [-10000; 10000]");
      return 0;
    }
    uint32_t setting = std::stoi(argv[1]);
    //= std::stoi(argv[1]);
    FeedbackManager feedback;
    g_monitor.set_att_pos(setting, feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
  }
  else if (cmd == "exec_method")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    if (argc != 4)
    {
      add_feedback(Severity::WARN, "Usage: exec_method <method> <value> <type>");
      add_feedback(Severity::WARN, "      Call generic method with an argument. Types: \'i\',\'d\',\'f\',\'s\',\'i32\',\'u32\',\'u16\',\'i16\',\'b\'");
      return 0;
    }
    // the trouble here is to cast the value into the right type
    // FIXME: Implement this
    FeedbackManager feedback;
    std::string method = argv[1];
    std::string value = argv[2];
    std::string type = argv[3];

    bool success = g_monitor.call_method(method, value, type, feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    if (success)
    {
      add_feedback(Severity::INFO, "Method executed with success");
    }
  }
  else if (cmd == "set_dac")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    if (argc != 2)
    {
      add_feedback(Severity::WARN, "Usage: set_dac <value>");
      add_feedback(Severity::WARN, "      Value cannot be above 4095");
      return 0;
    }
    uint16_t value = std::stoi(argv[1]);
    if (value > 4095)
    {
      add_feedback(Severity::ERROR, "Value cannot be above 4095");
      return 0;
    }
    FeedbackManager feedback;
    g_monitor.set_dac_threshold(value, feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
  }
  else if (cmd == "add_monitor")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    if (argc != 2)
    {
      add_feedback(Severity::WARN, "Usage: add_monitor <variable>");
      return 0;
    }
    std::string variable(argv[1]);
    g_vars_to_monitor.push_back(variable);
    FeedbackManager feedback;
    g_monitor.set_monitored_vars(g_vars_to_monitor);
    add_feedback(Severity::INFO, "Added variable to monitor list.");
  }

  else if (cmd == "add_monitor")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    if (argc != 2)
    {
      add_feedback(Severity::WARN, "Usage: add_monitor <variable>");
      return 0;
    }
    std::string variable(argv[1]);
    g_vars_to_monitor.push_back(variable);
    FeedbackManager feedback;
    g_monitor.set_monitored_vars(g_vars_to_monitor);
    add_feedback(Severity::INFO, "Added variable to monitor list.");
  }
  else
  {
    add_feedback(Severity::ERROR, "Unknown command");
    return 0;
  }
  // do something with the command
  return 0;
}
