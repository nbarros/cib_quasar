#include "Open62541Client.h"
#include <ncurses.h>
#include <string>
#include <sstream>

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

int main()
{
    initscr();            // Start curses mode
    cbreak();             // Line buffering disabled
    keypad(stdscr, TRUE); // We get F1, F2 etc..
    refresh();

    int height = LINES - 2;
    int width = COLS / 2 - 1;

    WINDOW *left_pane = nullptr;
    WINDOW *right_pane = nullptr;

    initialize_pane(left_pane, height, width, 1, 0, "Left Pane");
    initialize_pane(right_pane, height, width, 1, width + 2, "Right Pane");

    try
    {
        Open62541Client client;
        client.connect("opc.tcp://localhost:4840");

        UA_Variant value;
        UA_Variant_init(&value);

        std::string input;
        while (true)
        {
            // Get user input from the left pane
            mvwgetstr(left_pane, 1, 1, &input[0]);

            // Read variable from OPC UA server
            client.read_variable(input, value);

            // Convert the value to a string
            std::ostringstream oss;
            oss << "Value: " << UA_Variant_toString(&value);

            // Write the output to the right pane
            write_to_pane(right_pane, 1, 1, oss.str());

            // Clear the input and output for the next iteration
            werase(left_pane);
            werase(right_pane);
            box(left_pane, 0, 0);
            box(right_pane, 0, 0);
            mvwprintw(left_pane, 0, 1, "Left Pane");
            mvwprintw(right_pane, 0, 1, "Right Pane");
            wrefresh(left_pane);
            wrefresh(right_pane);
        }

        UA_Variant_clear(&value);
        client.disconnect();
    }
    catch (const std::exception &e)
    {
        spdlog::critical("Exception caught: {}", e.what());
    }

    delwin(left_pane);
    delwin(right_pane);
    endwin(); // End curses mode

    return 0;
}
