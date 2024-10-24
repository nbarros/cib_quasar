#include <ncurses.h>
#include <string>

void initialize_pane(WINDOW *pane, int height, int width, int starty, int startx, const std::string &title)
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

    write_to_pane(left_pane, 1, 1, "Hello from the left pane!");
    write_to_pane(right_pane, 1, 1, "Hello from the right pane!");

    getch(); // Wait for user input

    delwin(left_pane);
    delwin(right_pane);
    endwin(); // End curses mode

    return 0;
}

