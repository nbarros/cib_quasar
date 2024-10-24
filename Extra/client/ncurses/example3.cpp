#include <ncurses.h>
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <deque>

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

int main()
{
    initscr();            // Start curses mode
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

    int height = LINES - 2;
    int left_width = (COLS * 3) / 5 - 1;
    int right_width = (COLS * 2) / 5 - 1;

    WINDOW *left_pane = nullptr;
    WINDOW *right_pane = nullptr;

    initialize_pane(left_pane, height, left_width, 1, 0, "Left Pane");
    initialize_pane(right_pane, height, right_width, 1, left_width + 2, "Right Pane");

    std::string input;
    std::string statuses[5] = {"offline", "ready", "warmup", "pause", "standby"};
    int status_index = 0;
    std::deque<std::string> feedback;

    while (true)
    {
        // Get user input from the last line of the left pane
        char buffer[256];
        mvwgetstr(left_pane, height - 2, 1, buffer);
        input = buffer;

        // Add the input to the feedback deque
        feedback.push_front("You entered: " + input);
        if (feedback.size() > height - 4) // Keep the feedback within the pane height
        {
            feedback.pop_back();
        }

        // Update the left pane with feedback
        werase(left_pane);
        box(left_pane, 0, 0);
        mvwprintw(left_pane, 0, 1, "Left Pane");

        // Display feedback starting from the third line from the bottom
        int line = height - 4;
        for (const auto &msg : feedback)
        {
            write_to_pane(left_pane, line--, 1, msg);
        }

        // Prompt for input on the last line
        mvwprintw(left_pane, height - 2, 1, "Enter input: ");
        wrefresh(left_pane);

        // Update the right pane with labels
        werase(right_pane);
        box(right_pane, 0, 0);
        mvwprintw(right_pane, 0, 1, "Right Pane");

        // Set labels with different background colors based on status
        for (int i = 0; i < 5; ++i)
        {
            set_label_color(right_pane, i + 2, 1, "Label " + std::to_string(i + 1), statuses[(status_index + i) % 5]);
        }

        // Increment status index for next iteration
        status_index = (status_index + 1) % 5;

        // Sleep for 1 second to allow the user to see the output
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    delwin(left_pane);
    delwin(right_pane);
    endwin(); // End curses mode

    return 0;
}