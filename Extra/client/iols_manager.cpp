#include <ncurses.h>
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <deque>
#include <atomic>

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
  else if (status == "ready" || status == "Y" || status == "good")
    color_pair = 2;
  else if (status == "warmup" || status == "pause" || status == "standby" || status == "operating" || status == "?" || status == "busy" )
    color_pair = 3;
  else if (status == "error")
    color_pair = 5;
  else if (status == "lasing")
    color_pair = 6;
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
    mvwprintw(right_pane, vpos, 2, "Warmup timer : %" PRIu32 " s", status.count("LS1.L1.warmup_timer_s") ? std::get<uint32_t>(status["LS1.L1.warmup_timer_s"]) : 0);
    vpos += 1;
    mvwprintw(right_pane, vpos, 2, "Standby timer : %" PRIu32" s", status.count("LS1.L1.standby_timer_s") ? std::get<uint32_t>(status["LS1.L1.standby_timer_s"]) : 0);
    vpos += 1;
    mvwprintw(right_pane, vpos, 2, "Pause timer : %" PRIu32 " s", status.count("LS1.L1.pause_timer_s") ? std::get<uint32_t>(status["LS1.L1.pause_timer_s"]) : 0);
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

int main(int argc, char** argv)
{
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
  init_pair(5, COLOR_RED, COLOR_BLACK);
  init_pair(6, COLOR_YELLOW, COLOR_BLACK);

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
