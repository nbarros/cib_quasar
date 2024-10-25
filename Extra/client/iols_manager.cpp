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
IoLSMonitor *g_monitor;
std::deque<std::string> g_feedback;
std::vector<std::string> g_vars_to_monitor = {"LS1.state", "LS1.RNN800.state", "LS1.RNN600.state", "LS1.LSTAGE.state", "LS1.A1.state", "LS1.PM1.state", "LS1.PM1.energy_reading", "LS1.PM1.average_reading", "LS1.RNN800.current_position_motor", "RNN800.current_position_cib", "LS1.RNN600.current_position_motor", "LS1.RNN600.current_position_cib", "LS1.LSTAGE.current_position_motor", "LS1.LSTAGE.current_position_cib", "LS1.A1.position"};

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
  if (status == "offline")
    color_pair = 1;
  else if (status == "ready")
    color_pair = 2;
  else if (status == "warmup" || status == "pause" || status == "standby")
    color_pair = 3;
  else
    color_pair = 0; // Default color

  wattron(pane, COLOR_PAIR(color_pair));
  mvwprintw(pane, y, x, label.c_str());
  wattroff(pane, COLOR_PAIR(color_pair));
  wrefresh(pane);
}

void update_right_pane(WINDOW *right_pane, std::atomic<bool> &running, int height, iols_monitor_t &status)
{
  std::vector<std::string> labels = {"RNN800", "RNN600", "LSTAGE", "A1", "PM1", "L1"};
  while (running)
  {
    reset_right_pane(right_pane);
    int hpos = 2;
    // redraw everything
    // first the states
    wattron(right_pane, A_BOLD);
    mvwprintw(right_pane, hpos, 1, "Status Monitor");
    wattroff(right_pane, A_BOLD);
    for (size_t i = 0; i < labels.size(); i++)
    {
      hpos = 3+i;
      mvwprintw(right_pane, hpos, 2, labels[i].c_str());
      std::string entry = "LS1." + labels[i] + ".state";
      auto it = status.find(entry);
      std::string v = (it != status.end()) ? std::get<std::string>(it->second) : "unknown";

      set_label_color(right_pane, hpos, 15, v, v);
    }
    hpos = 3 + labels.size();
    // iols
    auto it = status.find("LS1.state");
    std::string v = (it != status.end()) ? std::get<std::string>(it->second) : "unknown";

    set_label_color(right_pane, hpos, 15, v, v);
    // set_label_color(right_pane, hpos, 15, v, v);
    hpos += 2;
    // Add a horizontal line below the title
    wmove(right_pane, hpos, 1);
    whline(right_pane, ACS_HLINE, getmaxx(right_pane) - 2);
    hpos += 2;
    // now add in the other monitored items
    mvwprintw(right_pane, hpos, 2, "Motor : [%d, %d, %d]", 
              status.count("LS1.RNN800.current_position_motor") ? std::get<int>(status["LS1.RNN800.current_position_motor"]) : -1,
              status.count("LS1.RNN600.current_position_motor") ? std::get<int>(status["LS1.RNN600.current_position_motor"]) : -1,
              status.count("LS1.LSTAGE.current_position_motor") ? std::get<int>(status["LS1.LSTAGE.current_position_motor"]) : -1);
    hpos += 1;
    mvwprintw(right_pane, hpos, 2, "CIB   : [%d, %d, %d]", 
              status.count("LS1.RNN800.current_position_cib") ? std::get<int>(status["LS1.RNN800.current_position_cib"]) : -1,
              status.count("LS1.RNN600.current_position_cib") ? std::get<int>(status["LS1.RNN600.current_position_cib"]) : -1,
              status.count("LS1.LSTAGE.current_position_cib") ? std::get<int>(status["LS1.LSTAGE.current_position_cib"]) : -1);
    hpos += 2;
    mvwprintw(right_pane, hpos, 2, "Attenuator position : %d", 
              status.count("LS1.A1.position") ? std::get<int>(status["LS1.A1.position"]) : -1);
    // attenuator position
    // mvwprintw(right_pane, hpos, 2, "Attenuator position : %d", std::get<int>(status["LS1.A1.attenuator_position"]));
    hpos += 2;
    mvwprintw(right_pane, hpos, 2, "Power Meter latest energy   : %.2f", 
              status.count("LS1.PM1.energy_reading") ? std::get<double>(status["LS1.PM1.energy_reading"]) : -1.0);
    hpos += 1;
    mvwprintw(right_pane, hpos, 2, "Power Meter average energy  : %.2f", 
              status.count("LS1.PM1.average_energy") ? std::get<double>(status["LS1.PM1.average_energy"]) : -1.0);
    hpos += 1;
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

void print_help()
{
  update_feedback("Available commands:");
  update_feedback("  help");
  update_feedback("       Prints this help");
  update_feedback("   connect <server>");
  update_feedback("       Connect to a server. Options are:");
  update_feedback("         cib1 : connects to P1");
  update_feedback("         cib2 : connects to P2");
  update_feedback("         opc.tcp://11.22.33.44:5555 : connects to a server in another location");
  update_feedback("   disconnect");
  update_feedback("       Disconnect from the server");
  update_feedback("   shutdown");
  update_feedback("       Shutdown the IoLS system");
  update_feedback("   config <location>");
  update_feedback("       Configure the IoLS system. Location points to the configuration file");
  update_feedback("   move_to_position <position> [approach]");
  update_feedback("       Move to a specified position. Approach is optional");
  update_feedback("       Example: move_to_position [155,256,367] uud");
  update_feedback("   warmup");
  update_feedback("       Start warmup of the laser. During this stage only motors can be moved.");
  update_feedback("   pause");
  update_feedback("       Pause the system. This will *keep* the laser from firing, but shutter is closed.");
  update_feedback("   standby");
  update_feedback("       Pause the system. This will close the internal shutter and stop QSWITCH.");
  update_feedback("   resume");
  update_feedback("       Resume the system. This will open the shutters and start QSWITCH.");
  update_feedback("   stop");
  update_feedback("       Stop the system. This will stop the system (fire, qswitch, and return shutters to default position).");
  update_feedback("   fire_at_position <position> <num_shots>");
  update_feedback("       Fire at a specified position. Number of shots is optional.");
  update_feedback("       Example: fire_at_position [1,2,3] 10");
  update_feedback("   fire_segment <start_position> <end_position>");
  update_feedback("       Fire at a segment between two positions.");
  update_feedback("       Example: fire_segment [1,2,3] [4,5,6]");
  update_feedback("   execute_scan <run_plan>");
  update_feedback("       Execute a scan plan. The run plan should be a JSON object with a 'scan_plan' array.");
  update_feedback("       Example: execute_scan '{\"scan_plan\":[{\"start\":[1,2,3],\"end\":[4,5,6]}, {\"start\":[7,8,9],\"end\":[10,11,12]}]}'");
  // update_feedback("   add_monitor <variable>");
  // update_feedback("       Add a variable to the monitor list. Variable must be a fully qualified OPC-UA node");
  update_feedback("   read_variable <variable>");
  update_feedback("       Read the value of a variable. Variable must be a fully qualified OPC-UA node");
  update_feedback("  exit");
  update_feedback("       Exit the program");
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
    if (g_monitor != nullptr)
    {
      json resp;
      g_monitor->shutdown(resp);
      update_feedback(resp);
      g_monitor->disconnect();
      auto v = g_monitor->get_feedback_messages();
      for (const auto &msg : v)
      {
        update_feedback(msg);
      }
      delete g_monitor;
      g_monitor = nullptr;
    }
    return 255;
  }
  else if (cmd == "connect")
  {
    if (g_monitor != nullptr)
    {
      update_feedback("Already connected to a server. Disconnect first.");
      return 0;
    }
    if (argc < 2)
    {
      update_feedback("Usage: connect <server>");
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
      update_feedback("Connecting to server: " + server);
      g_monitor = new IoLSMonitor(server);
      auto v = g_monitor->get_feedback_messages();
      for (const auto &msg : v)
      {
        update_feedback(msg);
      }
      if (g_monitor->connect())
      {
        auto v = g_monitor->get_feedback_messages();
        for (const auto &msg : v)
        {
          update_feedback(msg);
        }
        update_feedback("Connected to server.");
        // update the variables that are to be kept under surveillance
        g_monitor->set_monitored_vars(g_vars_to_monitor);
      }
      else
      {
        auto v = g_monitor->get_feedback_messages();
        for (const auto &msg : v)
        {
          update_feedback(msg);
        }
        update_feedback("Failed to connect to server.");
        delete g_monitor;
        g_monitor = nullptr;
      }
      // start connection
      return 0;
    }
  }
  else if (cmd == "disconnect")
  {
    if (g_monitor == nullptr)
    {
      update_feedback("Not connected to a server.");
      return 0;
    }
    iols_monitor_t status;
    g_monitor->get_status(status);
    if (std::get<std::string>(status["LS1.state"]) != "offline")
    {
      update_feedback("Cannot disconnect while the system is running. Call 'shutdown' first.");
      return 0;
    }
    g_monitor->disconnect();
    auto v = g_monitor->get_feedback_messages();
    for (const auto &msg : v)
    {
      update_feedback(msg);
    }
    update_feedback("Disconnecting from server.");
    delete g_monitor;
    g_monitor = nullptr;
    return 0;
  }
  else if (cmd == "shutdown")
  {
    if (g_monitor == nullptr)
    {
      update_feedback("Not connected to a server.");
      return 0;
    }
    update_feedback("Shutting down the system.");
    json resp;
    g_monitor->shutdown(resp);
    update_feedback(resp);
    return 0;
  }
  else if (cmd == "config")
  {
    if (g_monitor == nullptr)
    {
      update_feedback("Not connected to a server.");
      return 0;
    }
    if (argc < 2)
    {
      update_feedback("Usage: config <location>");
      return 0;
    }
    std::string location(argv[1]);
    json resp;
    if (g_monitor->config(location, resp))
    {
      update_feedback("Configuration successful.");
    }
    else
    {
      update_feedback("Configuration failed.");
    }
    update_feedback(resp);
    return 0;
  }
  else if (cmd == "move_to_position")
  {
    if (g_monitor == nullptr)
    {
      update_feedback("Not connected to a server.");
      return 0;
    }
    if (argc < 2)
    {
      update_feedback("Usage: move_to_position <position>");
      return 0;
    }
    std::string position(argv[1]);
    json resp;
    std::string approach = "---";
    if (argc == 3)
    {
      approach = argv[2];
    }
    if (g_monitor->move_to_position(position, approach, resp))
    {
      update_feedback("Move to position successful.");
    }
    else
    {
      update_feedback("Move to position failed.");
    }
    update_feedback(resp);
  }
  else if (cmd == "warmup")
  {
    if (g_monitor == nullptr)
    {
      update_feedback("Not connected to a server.");
      return 0;
    }
    json resp;
    if (g_monitor->warmup(resp))
    {
      update_feedback("Warmup successful.");
    }
    else
    {
      update_feedback("Warmup failed.");
    }
    update_feedback(resp);
  }
  else if (cmd == "pause")
  {
    if (g_monitor == nullptr)
    {
      update_feedback("Not connected to a server.");
      return 0;
    }
    json resp;
    if (g_monitor->pause(resp))
    {
      update_feedback("Pause successful.");
    }
    else
    {
      update_feedback("Pause failed.");
    }
    update_feedback(resp);
  }
  else if (cmd == "standby")
  {
    if (g_monitor == nullptr)
    {
      update_feedback("Not connected to a server.");
      return 0;
    }
    json resp;
    if (g_monitor->standby(resp))
    {
      update_feedback("Standby successful.");
    }
    else
    {
      update_feedback("Standby failed.");
    }
    update_feedback(resp);
  }
  else if (cmd == "resume")
  {
    if (g_monitor == nullptr)
    {
      update_feedback("Not connected to a server.");
      return 0;
    }
    json resp;
    if (g_monitor->resume(resp))
    {
      update_feedback("Resume successful.");
    }
    else
    {
      update_feedback("Resume failed.");
    }
    update_feedback(resp);
  }
  else if (cmd == "stop")
  {
    if (g_monitor == nullptr)
    {
      update_feedback("Not connected to a server.");
      return 0;
    }
    json resp;
    if (g_monitor->stop(resp))
    {
      update_feedback("Stop successful.");
    }
    else
    {
      update_feedback("Stop failed.");
    }
    update_feedback(resp);
  }
  else if (cmd == "fire_at_position")
  {
    if (g_monitor == nullptr)
    {
      update_feedback("Not connected to a server.");
      return 0;
    }
    if (argc != 3)
    {
      update_feedback("Usage: fire_at_position <position> <num_shots>");
      return 0;
    }
    std::string position(argv[1]);
    uint32_t num_shots = std::stoi(argv[2]);
    json resp;
    if (g_monitor->fire_at_position(position, num_shots, resp))
    {
      update_feedback("Fire at position successful.");
    }
    else
    {
      update_feedback("Fire at position failed.");
    }
    update_feedback(resp);    
  }
  else if (cmd == "fire_segment")
  {
    if (g_monitor == nullptr)
    {
      update_feedback("Not connected to a server.");
      return 0;
    }
    if (argc != 3)
    {
      update_feedback("Usage: fire_segment <start_position> <end_position>");
      return 0;
    }
    std::string start_position(argv[1]);
    std::string end_position(argv[2]);
    json resp;
    if (g_monitor->fire_segment(start_position, end_position, resp))
    {
      update_feedback("Fire segment successful.");
    }
    else
    {
      update_feedback("Fire segment failed.");
    }
    update_feedback(resp);
  }
  else if (cmd == "execute_scan")
  {
    if (g_monitor == nullptr)
    {
      update_feedback("Not connected to a server.");
      return 0;
    }
    if (argc != 2)
    {
      update_feedback("Usage: execute_scan <run_plan>");
      return 0;
    }
    std::string run_plan(argv[1]);
    json resp;
    if (g_monitor->execute_scan(run_plan, resp))
    {
      update_feedback("Execute scan successful.");
    }
    else
    {
      update_feedback("Execute scan failed.");
    }
    update_feedback(resp);
  }
  else if (cmd == "shutdown")
  {
    if (g_monitor == nullptr)
    {
      update_feedback("Not connected to a server.");
      return 0;
    }
    update_feedback("Shutting down the system.");
    json resp;
    g_monitor->shutdown(resp);
    update_feedback(resp);
    return 0;
  }
  else if (cmd == "help")
  {
    print_help();
    return 0;
  }
  else if (cmd == "read_variable")
  {
    if (g_monitor == nullptr)
    {
      update_feedback("Not connected to a server.");
      return 0;
    }
    if (argc != 2)
    {
      update_feedback("Usage: read_variable <variable>");
      return 0;
    }
    std::string variable(argv[1]);
    UA_Variant value;
    g_monitor->read_variable(variable, value);
    std::ostringstream msg;
    msg << variable << ": ";
    if (value.type == &UA_TYPES[UA_TYPES_STRING])
    {
      msg << std::string(reinterpret_cast<char *>(value.data));
    }
    else if (value.type == &UA_TYPES[UA_TYPES_DOUBLE])
    {
      msg << *static_cast<double *>(value.data);
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
    else
    {
      msg << "Unknown type";
    }
    update_feedback(msg.str());
  }
  else if (cmd == "add_monitor")
  {
    if (g_monitor == nullptr)
    {
      update_feedback("Not connected to a server.");
      return 0;
    }
    if (argc != 2)
    {
      update_feedback("Usage: add_monitor <variable>");
      return 0;
    }
    std::string variable(argv[1]);
    g_vars_to_monitor.push_back(variable);
    g_monitor->set_monitored_vars(g_vars_to_monitor);
    update_feedback("Added variable to monitor list.");
  }
  else
  {
    update_feedback("Unknown command");
    return 0;
  }
  // do something with the command
  return 0;
}

void refresh_left_panel(WINDOW *pane, int height)
{
  werase(pane);
  box(pane, 0, 0);
  mvwprintw(pane, 0, 1, "Command Terminal");

  // Display feedback starting from the third line from the bottom
  int line = height - 4;
  for (const auto &msg : g_feedback)
  {
    write_to_pane(pane, line--, 1, msg);
  }

  // Prompt for input on the last line
  mvwprintw(pane, height - 2, 1, ">> ");
  wrefresh(pane);
}

void update_feedback(const std::string &msg)
{
  g_feedback.push_front(msg);
  if (g_feedback.size() > static_cast<size_t>(g_height - 4))
  {
    g_feedback.pop_back();
  }
}

void update_feedback(json &resp)
{
  if (resp.contains("messages"))
  {
    for (const auto &msg : resp["messages"])
    {
      update_feedback(msg);
    }
  }
}

int main(int argc, char** argv)
{

  // initialize globals
  g_monitor = nullptr;

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
  iols_monitor_t status; // Assuming you have a way to initialize this structure

  // start the monitoring thread
  std::thread right_pane_thread(update_right_pane, right_pane, std::ref(run_monitor), g_height, std::ref(status));
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
    update_feedback(msg.str());
    if (tokens.size() > 0)
    {
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
