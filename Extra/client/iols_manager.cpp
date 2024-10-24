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

#include "iols_client_functions.h"
#include "iols_manager.h"
#include "toolbox.h"
#include "IoLSMonitor.h"
/*
  Global variables to be manipulated
*/
int g_height;
IoLSMonitor *g_monitor;
std::deque<std::string> g_feedback;

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
  while (running)
  {
    reset_right_pane(right_pane);

    // redraw everything
    // first the states
    wattron(right_pane, A_BOLD);
    mvwprintw(right_pane, 2, 1, "Status Monitor");
    wattroff(right_pane, A_BOLD);
    // motor 1
    mvwprintw(right_pane, 4, 2, "RNN800 :");
    set_label_color(right_pane, 4, 12, status.m1.state, status.m1.state);
    // motor 2
    mvwprintw(right_pane, 5, 2, "RNN600 :");
    set_label_color(right_pane, 5, 12, status.m2.state, status.m2.state);
    // motor 3
    mvwprintw(right_pane, 6, 2, "LSTAGE :");
    set_label_color(right_pane, 6, 12, status.m3.state, status.m3.state);
    // attenuator
    mvwprintw(right_pane, 7, 2, "Attenuator :");
    set_label_color(right_pane, 7, 12, status.att.state, status.att.state);
    // power meter
    mvwprintw(right_pane, 8, 2, "Power Meter :");
    set_label_color(right_pane, 8, 12, status.pm.state, status.pm.state);
    // laser unit
    mvwprintw(right_pane, 9, 2, "Laser Unit :");
    set_label_color(right_pane, 9, 12, status.laser.state, status.laser.state);

    // iols
    mvwprintw(right_pane, 11, 2, "IoLS :");
    set_label_color(right_pane, 11, 12, status.iols_state, status.iols_state);

    // Add a horizontal line below the title
    wmove(right_pane, 13, 1);
    whline(right_pane, ACS_HLINE, getmaxx(right_pane) - 2);

    // now add in the other monitored items
    mvwprintw(right_pane, 15, 2, "Periscope position");
    mvwprintw(right_pane, 17, 2, "Motor : [%d, %d, %d]", status.m1.position_motor, status.m2.position_motor, status.m3.position_motor);
    mvwprintw(right_pane, 18, 2, "CIB   : [%d, %d, %d]", status.m1.position_cib, status.m2.position_cib, status.m3.position_cib);

    // attenuator position
    mvwprintw(right_pane, 20, 2, "Attenuator position : %d", status.att.position);

    // power meter readings
    mvwprintw(right_pane, 22, 2, "Power Meter latest energy   : %.2f", status.pm.latest_reading);
    mvwprintw(right_pane, 23, 2, "Power Meter average energy  : %.2f", status.pm.average_reading);

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
  update_feedback("  connect <server>");
  update_feedback("     Connect to a server. Options are:");
  update_feedback("         cib1 : connects to P1");
  update_feedback("         cib2 : connects to P2");
  update_feedback("         opc.tcp://11.22.33.44:5555 : connects to a server in another location");
  update_feedback("  help");
  update_feedback("     Prints this help");
  update_feedback("  exit");
  update_feedback("     Exit the program");
}

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
      g_monitor->disconnect();
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
    if (status.iols_state != "offline")
    {
      update_feedback("Cannot disconnect while the system is running. Call 'shutdown' first.");
      return 0;
    }
    g_monitor->disconnect();
    update_feedback("Disconnecting from server.");
    delete g_monitor;
    g_monitor = nullptr;
    return 0;
  }
  else if (cmd == "help")
  {
    print_help();
    return 0;
  }
  else
  {
    update_feedback("Unknown command");
    return 0;
  }
  update_feedback("Did something interesting");
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
  status.att.position = 0;
  status.iols_state = "offline";
  status.laser.state = "offline";
  status.m1.state = "offline";
  status.m2.state = "offline";
  status.m3.state = "offline";
  status.pm.state = "offline";
  status.pm.latest_reading = 0.0;
  status.pm.average_reading = 0.0;
  status.m1.position_motor = 0;
  status.m1.position_cib = 0;
  status.m2.position_motor = 0;
  status.m2.position_cib = 0;
  status.m3.position_motor = 0;
  status.m3.position_cib = 0;
  status.att.state = "offline";

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
    // if (buffer[0] == '\n')
    // {
    //   // repeat
    //   continue;
    // }
    // printf("Passed here\n");
    input = buffer;
    //readline(input.c_str());
    // if (input.length() > 0)
    // {
    //   add_history(input.c_str()); 
    // }

    std::vector<std::string> tokens;
    tokens = toolbox::split_string(input,' ');
    std::ostringstream msg;
    msg << "Got " << tokens.size() << " tokens";
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
    }
    else
    {
      // if there are no commands, just continue
      continue;
      // goto leave;
      // return 0;
    }
    // char *delim = (char *)" ";
    // int count = 1;
    // char *ptr = const_cast<char*>(input.c_str());
    // while ((ptr = strchr(ptr, delim[0])) != NULL)
    // {
    //   count++;
    //   ptr++;
    // }
    // if (count > 0)
    // {
    //   update_feedback(">> " + input);
    //   char **cmd = new char *[count];
    //   cmd[0] = strtok(const_cast<char*>(input.c_str()), delim);
    //   int i;
    //   for (i = 1; cmd[i - 1] != NULL && i < count; i++)
    //   {
    //     cmd[i] = strtok(NULL, delim);
    //   }
    //   if (cmd[i - 1] == NULL)
    //     i--;
    //   // FIXME: Implement this function
    //   //  Add the input to the feedback deque
    //   // feedback.push_front(">> " + input);
    //   // if (feedback.size() > height - 4) // Keep the feedback within the pane height
    //   // {
    //   //   feedback.pop_back();
    //   // }
    //   // run the command
    //   int ret = run_command(i, cmd);
    //   refresh_left_panel(left_pane, g_height);
    //   delete[] cmd;
    //   if (ret == 255)
    //   {
    //     goto leave;
    //     return 0;
    //   }
    //   // if (ret != 0)
    //   // {
    //   //   goto leave;
    //   //   return ret;
    //   // }
    // }
    // else
    // {
    //   // if there are no commands, just continue
    //   continue;
    //   // goto leave;
    //   // return 0;
    // }

  }

  // while (true)
  // {
  //   // Get user input from the last line of the left pane
  //   char buffer[256];
  //   mvwgetstr(left_pane, height - 2, 1, buffer);
  //   input = buffer;
  //   status.laser.state = "ready";
    
  //   // Add the input to the feedback deque
  //   feedback.push_front(">> " + input);
  //   if (feedback.size() > height - 4) // Keep the feedback within the pane height
  //   {
  //     feedback.pop_back();
  //   }

  //   // Update the left pane with feedback
  //   werase(left_pane);
  //   box(left_pane, 0, 0);
  //   mvwprintw(left_pane, 0, 1, "Left Pane");

  //   // Display feedback starting from the third line from the bottom
  //   int line = height - 4;
  //   for (const auto &msg : feedback)
  //   {
  //     write_to_pane(left_pane, line--, 1, msg);
  //   }

  //   // Prompt for input on the last line
  //   mvwprintw(left_pane, height - 2, 1, "Enter input: ");
  //   wrefresh(left_pane);
  // }

leave:
  // Stop the right pane thread
  run_monitor.store(false);
  right_pane_thread.join();

  delwin(left_pane);
  delwin(right_pane);
  endwin(); // End curses mode

  return 0;
}
