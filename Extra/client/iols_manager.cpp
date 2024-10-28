#include <ncurses.h>
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <deque>
#include <atomic>
// extern "C"
// {
// #include <readline/readline.h>
// #include <readline/history.h>
// #include <unistd.h>
// };

// #include "iols_client_functions.h"
#include "iols_manager.h"
#include "toolbox.h"
#include "IoLSMonitor.h"
/*
  Global variables to be manipulated
*/
int g_height;
IoLSMonitor g_monitor;  
std::deque<FeedbackMessage> g_feedback;
std::vector<std::string> g_vars_to_monitor = {"LS1.state", "LS1.RNN800.state", "LS1.RNN600.state", "LS1.LSTAGE.state", "LS1.A1.state", "LS1.L1.state", "LS1.PM1.state", "LS1.PM1.energy_reading", "LS1.PM1.average_reading", "LS1.RNN800.current_position_motor", "LS1.RNN800.current_position_cib", "LS1.RNN600.current_position_motor", "LS1.RNN600.current_position_cib", "LS1.LSTAGE.current_position_motor", "LS1.LSTAGE.current_position_cib", "LS1.A1.position",
                                              "LS1.L1.ext_shutter_open", "LS1.L1.laser_shutter_open", "LS1.L1.laser_status_code", "LS1.L1.standby_timer_s", "LS1.L1.pause_timer_s",  "LS1.L1.warmup_timer_s", "LS1.L1.fire_active","LS1.L1.qswitch_active",
                                              "LS1.CIB1.dac_threshold"
                                              };

void initialize_pane(WINDOW *&pane, int height, int width, int starty, int startx, const std::string &title)
{
  pane = newwin(height, width, starty, startx);
  box(pane, 0, 0);
  mvwprintw(pane, 0, 1, title.c_str());
  wrefresh(pane);
}

void write_to_pane(WINDOW *pane, int y, int x, const std::string &text)
{
  mvwprintw(pane, y, x, text.c_str());
  wrefresh(pane);
}

void set_label_color(WINDOW *pane, int y, int x, const std::string &label, const std::string &status)
{
  int color_pair;
  if (status == "offline" || status == "N")
    color_pair = 1;
  else if (status == "ready" || status == "Y")
    color_pair = 2;
  else if (status == "warmup" || status == "pause" || status == "standby" || status == "operating" || status == "?")
    color_pair = 3;
  else
    color_pair = 0; // Default color

  wattron(pane, COLOR_PAIR(color_pair));
  mvwprintw(pane, y, x, label.c_str());
  wattroff(pane, COLOR_PAIR(color_pair));
  wrefresh(pane);
}

void update_right_pane(WINDOW *right_pane, std::atomic<bool> &running, int height, IoLSMonitor &monitor)
{
  std::vector<std::string> labels = {"RNN800", "RNN600", "LSTAGE", "A1", "PM1", "L1"};
  while (running)
  {
    try
    {
    iols_monitor_t status;
    if (monitor.is_connected())
    {
      monitor.get_status(status);
    }
    reset_right_pane(right_pane);
    int vpos = 2;
    int hpos = 2;
    // redraw everything
    // first the states
    wattron(right_pane, A_BOLD);
    mvwprintw(right_pane, hpos, 1, " Status Monitor ");
    wattroff(right_pane, A_BOLD);
    vpos += 3;
    for (size_t i = 0; i < labels.size(); i++)
    {
      if (i%2 == 0)
      {
        vpos++;
        hpos = 2;
      }
      else
      {
        hpos = 25;
      }
      mvwprintw(right_pane, vpos, hpos, labels[i].c_str());
      std::string entry = "LS1." + labels[i] + ".state";
      auto it = status.find(entry);
      std::string v = (it != status.end()) ? std::get<std::string>(it->second) : "UNKNOWN";
      set_label_color(right_pane, vpos, hpos + 10, v, v);
    }
    vpos += 2;
    // iols
    auto it = status.find("LS1.state");
    std::string v = (it != status.end()) ? std::get<std::string>(it->second) : "UNKNOWN";
    mvwprintw(right_pane, vpos, 2, "IoLS");
    set_label_color(right_pane, vpos, 12, v, v);
    // set_label_color(right_pane, hpos, 15, v, v);
    vpos += 2;
    // Add a horizontal line below the title
    wmove(right_pane, vpos, 1);
    whline(right_pane, ACS_HLINE, getmaxx(right_pane) - 2);
    vpos += 2;
    // now add in the other monitored items
    mvwprintw(right_pane, vpos, 2, "Motor : [%d, %d, %d]",
              status.count("LS1.RNN800.current_position_motor") ? std::get<int>(status["LS1.RNN800.current_position_motor"]) : -1,
              status.count("LS1.RNN600.current_position_motor") ? std::get<int>(status["LS1.RNN600.current_position_motor"]) : -1,
              status.count("LS1.LSTAGE.current_position_motor") ? std::get<int>(status["LS1.LSTAGE.current_position_motor"]) : -1);
    vpos += 1;
    mvwprintw(right_pane, vpos, 2, "CIB   : [%d, %d, %d]", 
              status.count("LS1.RNN800.current_position_cib") ? std::get<int>(status["LS1.RNN800.current_position_cib"]) : -1,
              status.count("LS1.RNN600.current_position_cib") ? std::get<int>(status["LS1.RNN600.current_position_cib"]) : -1,
              status.count("LS1.LSTAGE.current_position_cib") ? std::get<int>(status["LS1.LSTAGE.current_position_cib"]) : -1);
    vpos += 2;
    mvwprintw(right_pane, vpos, 2, "Attenuator position : %d", 
              status.count("LS1.A1.position") ? std::get<int>(status["LS1.A1.position"]) : -1);
    vpos += 2;
    mvwprintw(right_pane, vpos, 2, "DAC : %d",
              status.count("LS1.CIB1.dac_threshold") ? std::get<uint16_t>(status["LS1.CIB1.dac_threshold"]) : -1);
    vpos += 2;
    // A few laser updates
    mvwprintw(right_pane, vpos, 2, "Laser status code : %02d",
              status.count("LS1.L1.laser_status_code") ? std::get<uint16_t>(status["LS1.L1.laser_status_code"]) : -1);

    vpos += 2;
    wmove(right_pane, vpos, 1);
    whline(right_pane, ACS_HLINE, getmaxx(right_pane) - 2);
    // attenuator position
    // mvwprintw(right_pane, hpos, 2, "Attenuator position : %d", std::get<int>(status["LS1.A1.attenuator_position"]));
    vpos += 2;
    mvwprintw(right_pane, vpos, 2, "Power Meter :  energy   : %.3g J \t\t average : %.3g  J",
              status.count("LS1.PM1.energy_reading") ? std::get<double>(status["LS1.PM1.energy_reading"]) : -1.0,
              status.count("LS1.PM1.average_reading") ? std::get<double>(status["LS1.PM1.average_reading"]) : -1.0);

    vpos += 2;
    // Add a horizontal line
    wmove(right_pane, vpos, 1);
    whline(right_pane, ACS_HLINE, getmaxx(right_pane) - 2);
    mvwprintw(right_pane, vpos, 2, " Laser Timers ");
    vpos += 2;
    mvwprintw(right_pane, vpos, 2, "Warmup timer : %d s", status.count("LS1.L1.warmup_timer_s") ? std::get<uint32_t>(status["LS1.L1.warmup_timer_s"]) : 0);
    vpos += 1;
    mvwprintw(right_pane, vpos, 2, "Standby timer : %d s", status.count("LS1.L1.standby_timer_s") ? std::get<uint32_t>(status["LS1.L1.standby_timer_s"]) : 0);
    vpos += 1;
    mvwprintw(right_pane, vpos, 2, "Pause timer : %d s", status.count("LS1.L1.pause_timer_s") ? std::get<uint32_t>(status["LS1.L1.pause_timer_s"]) : 0);
    vpos += 2;
    // Add a horizontal line
    wmove(right_pane, vpos, 1);
    whline(right_pane, ACS_HLINE, getmaxx(right_pane) - 2);
    mvwprintw(right_pane, vpos, 2, " Laser Part States ");

    vpos += 2;
    // -- now show the states of the interesting parts
    char esh_open = status.count("LS1.L1.ext_shutter_open") ? (std::get<bool>(status["LS1.L1.ext_shutter_open"])?'Y':'N') : '?';
    char ish_open = status.count("LS1.L1.laser_shutter_open") ? (std::get<bool>(status["LS1.L1.laser_shutter_open"]) ? 'Y' : 'N') : '?';
    char fire_active = status.count("LS1.L1.fire_active") ? (std::get<bool>(status["LS1.L1.fire_active"]) ? 'Y' : 'N') : '?';
    char qswitch_active = status.count("LS1.L1.qswitch_active") ? (std::get<bool>(status["LS1.L1.qswitch_active"]) ? 'Y' : 'N') : '?';
    mvwprintw(right_pane, vpos, 2, "ESH OPEN : ");
    set_label_color(right_pane, vpos, 14, std::string(1, esh_open), std::string(1, esh_open));
    mvwprintw(right_pane, vpos, 17, "LSH OPEN : ");
    set_label_color(right_pane, vpos, 29, std::string(1, ish_open), std::string(1, ish_open));
    mvwprintw(right_pane, vpos, 31, "FIRE : ");
    set_label_color(right_pane, vpos, 39, std::string(1, fire_active), std::string(1, fire_active));
    mvwprintw(right_pane, vpos, 41, "QSWITCH : ");
    set_label_color(right_pane, vpos, 52, std::string(1, qswitch_active), std::string(1, qswitch_active));
    } 
    catch(const std::exception &e)
    {
      //add_feedback(Severity::ERROR, "Exception in update_right_pane: " + std::string(e.what()));
    }
    wrefresh(right_pane);

    // Sleep for 500 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

void reset_right_pane(WINDOW *right_pane)
{
  werase(right_pane);
  box(right_pane, 0, 0);
  mvwprintw(right_pane, 0, 1, "IoLS Monitoring");
}

void refresh_left_panel(WINDOW *pane, int height)
{
  werase(pane);
  box(pane, 0, 0);
  mvwprintw(pane, 0, 1, "Command Terminal");

  // Display feedback starting from the third line from the bottom
  int line = height - 4;
  int color_pair = 0;
  std::string label = "      ";
  for (const auto &msg : g_feedback)
  {
    switch(msg.severity)
    {
      case Severity::INFO:
        label = "INFO ";
        color_pair = 2;
        break;
      case Severity::WARN:
        label = "WARN ";
        color_pair = 3;
        break;
      case Severity::ERROR:
        label = "ERROR";
        color_pair = 1;
        break;
      case Severity::REPORT:
        label = "     ";
        color_pair = 4;
        break;
      default:
        label = "      ";
        color_pair = 0;
        break;
    }
    wattron(pane, COLOR_PAIR(color_pair));
    write_to_pane(pane, line, 1, label);
    wattroff(pane, COLOR_PAIR(color_pair));
    write_to_pane(pane, line--, 7, msg.message);
  }

  // Prompt for input on the last line
  mvwprintw(pane, height - 2, 1, ">> ");
  wrefresh(pane);
}

void add_feedback(Severity severity, const std::string &msg)
{
  g_feedback.push_front({severity, msg});
  if (g_feedback.size() > static_cast<size_t>(g_height - 4))
  {
    g_feedback.pop_back();
  }
}

void update_feedback(std::vector<FeedbackMessage> &feedback)
{
  for (const auto &msg : feedback)
  {
    add_feedback(msg.severity, msg.message);
  }
}


void print_help()
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
  add_feedback(Severity::INFO, "   fire_at_position <position> <num_shots>");
  add_feedback(Severity::INFO, "       Fire at a specified position. Number of shots is optional.");
  add_feedback(Severity::INFO, "       Example: fire_at_position [1,2,3] 10");
  add_feedback(Severity::INFO, "   fire_segment <start_position> <end_position>");
  add_feedback(Severity::INFO, "       Fire at a segment between two positions.");
  add_feedback(Severity::INFO, "       Example: fire_segment [1,2,3] [4,5,6]");
  add_feedback(Severity::INFO, "   execute_scan <run_plan>");
  add_feedback(Severity::INFO, "       Execute a scan plan. The run plan should be a JSON object with a 'scan_plan' array.");
  add_feedback(Severity::INFO, "       Example: execute_scan '{\"scan_plan\":[{\"start\":[1,2,3],\"end\":[4,5,6]}, {\"start\":[7,8,9],\"end\":[10,11,12]}]}'");
  // add_feedback(Severity::INFO, "   add_monitor <variable>");
  // add_feedback(Severity::INFO, "       Add a variable to the monitor list. Variable must be a fully qualified OPC-UA node");
  add_feedback(Severity::INFO, "   read_variable <variable>");
  add_feedback(Severity::INFO, "       Read the value of a variable. Variable must be a fully qualified OPC-UA node");
  add_feedback(Severity::INFO, "   set_pm_range <setting>");
  add_feedback(Severity::INFO, "       Check the variable LS1.PM1.range_options for reference");
  add_feedback(Severity::INFO, "   set_pm_threshold <setting>");
  add_feedback(Severity::INFO, "       Threshold should be something above 100 (units of 0.01%)");
  add_feedback(Severity::INFO, "   set_att_position <setting>");
  add_feedback(Severity::INFO, "       Position should be a value in [-10000; 10000]");
  add_feedback(Severity::INFO, "   set_dac <value>");
  add_feedback(Severity::INFO, "       Value cannot be above 4095");
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
int run_command(int argc, char**argv)
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

      //g_monitor->shutdown(feedback);
      // std::vector<FeedbackMessage> messages = feedback.get_messages();
      // update_feedback(messages);
      // g_monitor->disconnect(feedback);
      // messages = feedback.get_messages();
      // update_feedback(messages);
      // delete g_monitor;
      // g_monitor = nullptr;
    }
    return 255;
  }
  else if (cmd == "connect")
  {
    if (g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR,"Already connected to a server. Disconnect first.");
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
      add_feedback(Severity::INFO,"Connecting to server: " + server);
      // g_monitor = new IoLSMonitor(server);
      FeedbackManager feedback;
      bool res = g_monitor.connect(server,feedback);
      std::vector<FeedbackMessage> messages = feedback.get_messages();
      update_feedback(messages);
      if (res)
      {
        add_feedback(Severity::INFO,"Connected to server.");
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
      add_feedback(Severity::ERROR,"Not connected to a server.");
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
    add_feedback(Severity::INFO,"Disconnecting from server.");
    // delete g_monitor;
    // g_monitor = nullptr;
    return 0;
  }
  else if (cmd == "shutdown")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR,"Not connected to a server.");
      return 0;
    }
    add_feedback(Severity::INFO,"Shutting down the system.");
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
      add_feedback(Severity::ERROR,"Not connected to a server.");
      return 0;
    }
    if (argc < 2)
    {
      add_feedback(Severity::WARN,"Usage: config <location>");
      return 0;
    }
    std::string location(argv[1]);
    FeedbackManager feedback;
    bool res = g_monitor.config(location, feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    if (res)
    {
      add_feedback(Severity::INFO,"Configuration successful.");
    }
    else
    {
      add_feedback(Severity::ERROR,"Configuration failed.");
    }
    return 0;
  }
  else if (cmd == "move_to_position")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR,"Not connected to a server.");
      return 0;
    }
    if (argc < 2)
    {
      add_feedback(Severity::WARN,"Usage: move_to_position <position>");
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
      add_feedback(Severity::INFO,"Move to position successful.");
    }
    else
    {
      add_feedback(Severity::ERROR,"Move to position failed.");
    }
  }
  else if (cmd == "warmup")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR,"Not connected to a server.");
      return 0;
    }
    FeedbackManager feedback;
    bool res = g_monitor.warmup(feedback);
    std::vector<FeedbackMessage> messages = feedback.get_messages();
    update_feedback(messages);
    if (res)
    {
      add_feedback(Severity::INFO,"Warmup successful.");
    }
    else
    {
      add_feedback(Severity::ERROR,"Warmup failed.");
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
    if (argc != 3)
    {
      add_feedback(Severity::WARN, "Usage: fire_at_position <position> <num_shots>");
      return 0;
    }
    std::string position(argv[1]);
    uint32_t num_shots = std::stoi(argv[2]);
    FeedbackManager feedback;
    bool res = g_monitor.fire_at_position(position, num_shots, feedback);
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
  else if (cmd == "help")
  {
    print_help();
    return 0;
  }
  else if (cmd == "read_variable")
  {
    if (!g_monitor.is_connected())
    {
      add_feedback(Severity::ERROR, "Not connected to a server.");
      return 0;
    }
    if (argc != 2)
    {
      add_feedback(Severity::WARN, "Usage: read_variable <variable>");
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
      msg << std::string((char*)static_cast<UA_String*>(value.data)->data,(size_t)static_cast<UA_String*>(value.data)->length);
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


int main(int argc, char** argv)
{

  // initialize globals
  //g_monitor = nullptr;

  //
  // Initialize the ncurses window
  //
  initscr(); // Start curses mode
  if (!has_colors())
  {
    endwin();
    std::cerr << "Error: Terminal does not support colors." << std::endl;
    return 1;
  }
  start_color();        // Start color functionality
  cbreak();             // Line buffering disabled
  keypad(stdscr, TRUE); // We get F1, F2 etc..
  refresh();

  // Initialize color pairs with black text
  init_pair(1, COLOR_BLACK, COLOR_RED);
  init_pair(2, COLOR_BLACK, COLOR_GREEN);
  init_pair(3, COLOR_BLACK, COLOR_YELLOW);
  init_pair(4, COLOR_BLACK, COLOR_BLUE);

  //int height = LINES - 2;
  g_height = LINES - 2;
  int left_width = (COLS * 3) / 5 - 1;
  int right_width = (COLS * 2) / 5 - 1;

  WINDOW *left_pane = nullptr;
  WINDOW *right_pane = nullptr;

  initialize_pane(left_pane, g_height, left_width, 1, 0, "Command Terminal");
  initialize_pane(right_pane, g_height, right_width, 1, left_width + 2, "IoLS Monitoring");

  std::atomic<bool> run_monitor(true);

  std::string input;
  std::string statuses[5] = {"offline", "ready", "warmup", "pause", "standby"};
  std::atomic<int> status_index(0);

  // Start the thread to update the right pane
  // iols_monitor_t status; // Assuming you have a way to initialize this structure

  // start the monitoring thread
  std::thread right_pane_thread(update_right_pane, right_pane, std::ref(run_monitor), g_height, std::ref(g_monitor));
  // this part is similar to the cib_manager, but with the
  // ncurse interface
  // -- now start the real work
  // Get user input from the last line of the left pane
  std::atomic<bool> stop;
  stop = false;
  while(true)
  {
    // Get user input from the last line of the left pane
    refresh_left_panel(left_pane, g_height);
    char buffer[256];
    // mvwprintw(left_pane, height - 2, 1, ">> ");
    mvwgetstr(left_pane, g_height - 2, 4, buffer);

    input = buffer;

    std::vector<std::string> tokens;
    tokens = toolbox::split_string(input,' ');
    std::ostringstream msg;
    // msg << "Got " << tokens.size() << " tokens";
    // update_feedback(msg.str());
    if (tokens.size() > 0)
    {
      add_feedback(Severity::INFO,">> " + input);
      char **cmd = new char *[tokens.size()];
      for (size_t i = 0; i < tokens.size(); i++)
      {
        cmd[i] = const_cast<char*>(tokens[i].c_str());
      }
      int ret = run_command(tokens.size(), cmd);
      refresh_left_panel(left_pane, g_height);
      delete[] cmd;
      if (ret == 255)
      {
        goto leave;
        return 0;
      }
      //   // if (ret != 0)
      //   // {
      //   //   goto leave;
      //   //   return ret;
      //   // }
    }
    else
    {
      // if there are no commands, just continue
      continue;
      // goto leave;
      // return 0;
    }
  }

leave:
  // Stop the right pane thread
  run_monitor.store(false);
  right_pane_thread.join();

  delwin(left_pane);
  delwin(right_pane);
  endwin(); // End curses mode

  return 0;
}
