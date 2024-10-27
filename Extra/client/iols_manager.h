#ifndef IOLS_MANAGER_H
#define IOLS_MANAGER_H

#include <ncurses.h>
#include <string>
#include <deque>
#include <atomic>
#include "IoLSMonitor.h"
#include "FeedbackManager.h"

// Function prototypes
void initialize_pane(WINDOW *&pane, int height, int width, int starty, int startx, const std::string &title);
void write_to_pane(WINDOW *pane, int y, int x, const std::string &text);
void set_label_color(WINDOW *pane, int y, int x, const std::string &label, const std::string &status);
void reset_right_pane(WINDOW *right_pane);
void add_feedback(Severity severity, const std::string &msg);
void update_feedback(std::vector<FeedbackMessage> &feedback);
void update_right_pane(WINDOW *right_pane, std::atomic<bool> &running, int height, IoLSMonitor *monitor);
void refresh_left_panel(WINDOW *pane, int height);

#endif // IOLS_MANAGER_H