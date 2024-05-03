#include <ctime>
#include <curses.h>
using namespace std;

int row, col;
class Game_of_life {
    const int table_size;
    bool** game_field;
    bool** create_table() {
        bool** new_game_field = new bool* [table_size];
        for (int i = 0; i < table_size; i++)
            new_game_field[i] = new bool[table_size];
        for (int i = 0; i < table_size; i++)
            for (int j = 0; j < table_size; j++)
                new_game_field[i][j] = false;
        return new_game_field;
    }
public:
    Game_of_life(int size) : table_size(size + 2) {
        game_field = create_table();
    }
    ~Game_of_life() {
        for (int i = 0; i < table_size; i++)
            delete[] game_field[i];
        delete[] game_field;
    }
    bool change(int x, int y) {
        if (x < table_size - 1 and x >= 0 and y < table_size and y >= 0) {
            game_field[x + 1][y + 1] = !game_field[x + 1][y + 1];
            return true;
        }
        else return false;
    }
    bool get(int x, int y) {
        return game_field[x + 1][y + 1];
    }
    void iterate() {
        bool** new_game_field = create_table();
        for (int i = 1; i < table_size - 1; i++) {
            for (int j = 1; j < table_size - 1; j++) {
                int sum_neighbor = game_field[i - 1][j - 1] + game_field[i - 1][j] + game_field[i - 1][j + 1] + game_field[i][j - 1] + game_field[i][j + 1] + game_field[i + 1][j - 1] + game_field[i + 1][j] + game_field[i + 1][j + 1];
                if (game_field[i][j] == false and sum_neighbor == 3)
                    new_game_field[i][j] = true;
                else if (game_field[i][j] == true and (sum_neighbor == 2 or sum_neighbor == 3))
                    new_game_field[i][j] = true;
                else new_game_field[i][j] = false;
            }
        }
        for (int i = 0; i < table_size; i++)
            for (int j = 0; j < table_size; j++)
                game_field[i][j] = new_game_field[i][j];
        for (int i = 0; i < table_size; i++)
            delete[] new_game_field[i];
        delete[] new_game_field;
    }
    void display(int x, int y) {
        clear();
        refresh();
        for (int i = 0; i < table_size; i++) {
            for (int j = 0; j < table_size; j++) {
                if (i - 1 == x and j - 1 == y)
                    attron(A_REVERSE);
                else
                    attroff(A_REVERSE);
                if (game_field[i][j]) mvaddch(j + row - table_size, i + i + row - table_size, '@');
                else if (i == 0 or j == 0 or i == table_size - 1 or j == table_size - 1) mvaddch(j + row - table_size, i + i + row - table_size, '~');
                else mvaddch(j + row - table_size, i + i + row - table_size, '.');
            }
        }
    }
};

int table_size = 10;
Game_of_life* my_game = new Game_of_life(table_size);



int main()
{
    initscr();
    getmaxyx(stdscr, row, col);
    keypad(stdscr, true);
    curs_set(0);
    noecho();

    int choice = 0;
    const char menu_items[5][10] = {
    "Start",
    "Options",
    "Tutorial",
    "Credits",
    "Exit"
    };
    int x = 0;
    int y = 0;
    bool flag_input = true;
    time_t last_time;

    while (true) {
        clear();
        for (int i = 0; i < 5; i++) {
            if (i == choice) addch('>');
            else addch(' ');
            printw("%s\n", menu_items[i]);
        }

        switch (getch()) {
        case KEY_UP:
            if (choice) choice--;
            break;
        case KEY_DOWN:
            if (choice != 4) choice++;
            break;
        case 10:
            clear();
            switch (choice) {
            case 0:
                x = 0;
                y = 0;
                while (flag_input) {
                    my_game->display(x, y);
                    switch (getch()) {
                    case KEY_UP:
                        if (y > 0) y--;
                        break;
                    case KEY_DOWN:
                        if (y < table_size - 1) y++;
                        break;
                    case KEY_RIGHT:
                        if (x < table_size - 1) x++;
                        break;
                    case KEY_LEFT:
                        if (x > 0) x--;
                        break;
                    case KEY_END:
                        my_game->change(x, y);
                        break;
                    case 10:
                        flag_input = false;
                        break;
                    }
                }
                flag_input = true;
                timeout(0);

                last_time = 0;
                while (true) {
                    if (time(NULL) - last_time > 0.5) {
                        my_game->iterate();
                        my_game->display(-100, -100);
                        last_time = time(NULL);
                    }
                    if (getch() == 10) break;
                }
                timeout(-1);
                break;
            case 1:
                printw("Set the table size: ");
                scanw("%d", &table_size);
                printw("New table size: %d", table_size);
                delete my_game;
                my_game = new Game_of_life(table_size);
                printw("\n\nPress any key...");
                getch();
                break;
            case 2:
                printw("This is the game of life\nA dead cell comes to life if there are 3 living cells near it\nA living cell dies if there are less than 2 and more than 3 living cells near it");
                printw("\n\nPress any key...");
                getch();
                break;
            case 3:
                printw("Chernikov A.M.\n20.11.2023");
                printw("\n\nPress any key...");
                getch();
                break;
            case 4:
                printw("Do you want to exit? Y/N");
                while (true) {
                    int ch = getch();
                    if (ch == 110 or ch == 78)
                        break;
                    else if (ch == 121 or ch == 89) {
                        endwin();
                        return 0;
                    }
                }
                break;
            }
            break;
        }
    }

    endwin();
    return 0;
}

// 27 - esc