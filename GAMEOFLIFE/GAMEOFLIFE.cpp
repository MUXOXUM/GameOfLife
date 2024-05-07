#include <curses.h>
#include <vector>
#include <random>

int ROWS, COLS;
bool ISLIGHTMODE = false;
double ITERATIONTIME = 0.5;
int TABLEWIDTH = 100;
int TABLEHEIGHT = 23;

static constexpr int KEY_ARROW_UP = KEY_UP;
static constexpr int KEY_ARROW_DOWN = KEY_DOWN;
static constexpr int KEY_ARROW_LEFT = KEY_LEFT;
static constexpr int KEY_ARROW_RIGHT = KEY_RIGHT;
static constexpr int KEY_CHANGE_CELL = KEY_END;
static constexpr int KEY_RANDOM_FILL = KEY_HOME;
static constexpr int KEY_CLEAR_SCREEN = KEY_DC; //DEL
static constexpr int KEY_INS = KEY_IC; //INS
static constexpr int KEY_CONFIRM = 10; //ENTER
static constexpr int KEY_ESC = 27; //ESC
static constexpr int KEY_Y_UPPER = 89;
static constexpr int KEY_Y_LOWER = 121;
static constexpr int KEY_N_UPPER = 78;
static constexpr int KEY_N_LOWER = 110;

static void reverseColor() {
    ISLIGHTMODE ? attroff(A_REVERSE) : attron(A_REVERSE);
    ISLIGHTMODE = !ISLIGHTMODE;
}

class GameOfLife {
protected:
    int Width, Height, Generation, Population;
    std::vector<std::vector<bool>> CurrGameTable;
    std::vector<std::vector<bool>> PrevGameTable;

    bool getNormalizedValue(const int& x, const int& y) {
        return PrevGameTable[(y + Height) % Height][(x + Width) % Width];
    }

    void changeCell(const int& x, const int& y) {
        if (x >= 0 && y >= 0 && x < Width && y < Height)
            PrevGameTable[y][x] = !PrevGameTable[y][x];
    }

public:
    GameOfLife(int height, int width) : Height(height), Width(width), Generation(0), Population(0), CurrGameTable(height, std::vector<bool>(width)), PrevGameTable(height, std::vector<bool>(width)) {}

    int getGeneration() const { return Generation; }

    int getPopulation() const { return Population; }

    bool iterate() {
        Population = 0;
        for (int y = 0; y < Height; y++) {
            for (int x = 0; x < Width; x++) {
                int neighborSum = 0;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx != 0 || dy != 0) {
                            if (getNormalizedValue(x + dx, y + dy))
                                neighborSum++;
                        }
                    }
                }
                if (!PrevGameTable[y][x] && neighborSum == 3)
                    CurrGameTable[y][x] = true;
                else if (PrevGameTable[y][x] && (neighborSum == 2 || neighborSum == 3))
                    CurrGameTable[y][x] = true;
                else
                    CurrGameTable[y][x] = false;
                if (CurrGameTable[y][x])
                    Population++;
            }
        }
        Generation++;
        bool identicalFlag = (PrevGameTable == CurrGameTable);
        PrevGameTable = CurrGameTable;
        return identicalFlag;
    }

    void reset() {
        for (auto& row : PrevGameTable)
            std::fill(row.begin(), row.end(), false);
        Generation = 0;
        Population = 0;
    }

    void resize(int newHeight, int newWidth) {
        Height = newHeight;
        Width = newWidth;
        CurrGameTable.resize(Height, std::vector<bool>(Width));
        PrevGameTable.resize(Height, std::vector<bool>(Width));
    }
};

class Game : public GameOfLife {
private:
    void randomFill() {
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<int> distribution(0, 1);
        for (int y = 0; y < Height; y++)
            for (int x = 0; x < Width; x++)
                PrevGameTable[y][x] = (distribution(generator) == 0);
    }

    bool editTable() {
        int cursorX = 0;
        int cursorY = 0;
        while (true) {
            display(cursorX, cursorY);
            mvprintw(offsetY + Height + 1, (COLS - 74) / 2, "Change the state - END | Random fill - HOME | Clear - DEL | Start - ENTER");
            mvprintw(offsetY + Height + 2, (COLS - 41) / 2, "UP/DOWN/RIGHT/LEFT - Arrows | Exit - ESC");
            int key = getch();
            switch (key) {
            case KEY_CONFIRM:
                return true;
                break;
            case KEY_ESC:
                return false;
                break;
            case KEY_CHANGE_CELL:
                changeCell(cursorX, cursorY);
                break;
            case KEY_RANDOM_FILL:
                randomFill();
                break;
            case KEY_CLEAR_SCREEN:
                reset();
                break;
            case KEY_ARROW_UP:
                cursorY = (cursorY - 1 + Height) % Height;
                break;
            case KEY_ARROW_DOWN:
                cursorY = (cursorY + 1) % Height;
                break;
            case KEY_ARROW_LEFT:
                cursorX = (cursorX - 1 + Width) % Width;
                break;
            case KEY_ARROW_RIGHT:
                cursorX = (cursorX + 1) % Width;
                break;
            }
        }
    }

    int offsetX, offsetY;

public:
    Game(int height, int width) : GameOfLife(height, width), offsetX((COLS - Width) / 2), offsetY((ROWS - Height) / 2) {}

    void run() {
        bool isStable = false;
        time_t initialTime = time(NULL);
        time_t lastTime = time(NULL);
        offsetX = (COLS - Width) / 2;
        offsetY = (ROWS - Height) / 2;

        if (!editTable())
            return;
        while (true) {
            switch (getch()) {
            case KEY_INS:
                editTable();
                isStable = false;
                lastTime = 0;
                break;
            case KEY_ESC:
                //reset(); //нужно ли сбрасывать таблицу при выходе?
                return;
                break;
            default:
                if (time(NULL) - lastTime > ITERATIONTIME and !isStable) {
                    display();
                    isStable = iterate();
                    lastTime = time(NULL);
                }
                if (isStable) mvprintw(offsetY - 3, (COLS - 35) / 2, "Stable condition has been achieved!");
                mvprintw(offsetY + Height + 1, (COLS - 10) / 2, "Edit - INS");
                mvprintw(offsetY + Height + 2, (COLS - 10) / 2, "Exit - ESC");
                mvprintw(offsetY + Height + 1, (COLS - 10) / 2 - 25, "Generation: %d", getGeneration());
                mvprintw(offsetY + Height + 2, (COLS - 10) / 2 - 25, "Population: %d", getPopulation());
                mvprintw(offsetY + Height + 1, (COLS - 10) / 2 + 20, "Time: %d %s", (time(NULL) - initialTime), "sec");
                break;
            }
        }
    }

    void display(int targetX = -1, int targetY = -1) {
        for (int i = 0; i < ROWS; i++)
            for (int j = 0; j < COLS; j++)
                mvaddch(i, j, ' ');
        for (int y = 0; y < Height + 2; y++) {
            for (int x = 0; x < Width + 2; x++) {
                int posX = offsetX + x;
                int posY = offsetY + y - 2;
                if (x == 0 or y == 0 or x == Width + 1 or y == Height + 1) {
                    reverseColor();
                    mvwaddch(stdscr, posY, posX, ' ');
                    reverseColor();
                }
                else {
                    if (x - 1 == targetX and y - 1 == targetY) {
                        reverseColor();
                        PrevGameTable[y - 1][x - 1] ? mvaddch(posY, posX, '@') : mvaddch(posY, posX, ' ');
                        reverseColor();
                    }
                    else
                        PrevGameTable[y - 1][x - 1] ? mvaddch(posY, posX, '@') : mvaddch(posY, posX, ' ');
                }
            }
        }
    }
};

class MainMenu : public GameOfLife {
private:
    const std::vector< std::vector<std::string>> titles = {
    {
        " @@@@    @@@@   @@   @@  @@@@@      @@@@   @@@@@@     @@      @@@@@@  @@@@@@  @@@@@",
        "@@      @@  @@  @@@ @@@  @@        @@  @@  @@         @@        @@    @@      @@   ",
        "@@ @@@  @@@@@@  @@ @ @@  @@@@      @@  @@  @@@@       @@        @@    @@@@    @@@@ ",
        "@@  @@  @@  @@  @@   @@  @@        @@  @@  @@         @@        @@    @@      @@   ",
        " @@@@   @@  @@  @@   @@  @@@@@      @@@@   @@         @@@@@@  @@@@@@  @@      @@@@@"
    },
    {
        " @@@@   @@@@@   @@@@@@  @@@@@@   @@@@   @@  @@   @@@@ ",
        "@@  @@  @@  @@    @@      @@    @@  @@  @@@ @@  @@    ",
        "@@  @@  @@@@@     @@      @@    @@  @@  @@ @@@   @@@@ ",
        "@@  @@  @@        @@      @@    @@  @@  @@  @@      @@",
        " @@@@   @@        @@    @@@@@@   @@@@   @@  @@   @@@@ "
    },
    {
        "@@@@@@  @@  @@  @@@@@@   @@@@   @@@@@   @@@@@@   @@@@   @@    ",
        "  @@    @@  @@    @@    @@  @@  @@  @@    @@    @@  @@  @@    ",
        "  @@    @@  @@    @@    @@  @@  @@@@@     @@    @@@@@@  @@    ",
        "  @@    @@  @@    @@    @@  @@  @@  @@    @@    @@  @@  @@    ",
        "  @@     @@@@     @@     @@@@   @@  @@  @@@@@@  @@  @@  @@@@@@"
    },
    {
        " @@@@   @@@@@   @@@@@  @@@@@   @@@@@@  @@@@@@   @@@@ ",
        "@@  @@  @@  @@  @@     @@  @@    @@      @@    @@    ",
        "@@      @@@@@   @@@@   @@  @@    @@      @@     @@@@ ",
        "@@  @@  @@  @@  @@     @@  @@    @@      @@        @@",
        " @@@@   @@  @@  @@@@@  @@@@@   @@@@@@    @@     @@@@ "
    },
    {
        "@@@@@   @@  @@   @@@@@@   @@@@@@",
        "@@       @@@@      @@       @@  ",
        "@@@@      @@       @@       @@  ",
        "@@       @@@@      @@       @@  ",
        "@@@@@   @@  @@   @@@@@@     @@  "
    },
    };

    std::vector<std::string> mainMenuItems = {
        "Start",
        "Options",
        "Tutorial",
        "Credits",
        "Exit"
    };

    Game myGame{ TABLEHEIGHT, TABLEWIDTH };
    time_t lastTime;
    bool isTitleRunning;
    int itemsOffset;
    int startMenuPos;

    void showOptions();
    void showTutorial();
    void showCredits();
    bool exitProgram();

    void updateField(int titleNumber) {
        isTitleRunning = false;
        const int heightTitle = 5;
        const int lengthTitle = titles[titleNumber][0].size();

        reset();
        for (int y = 0; y < heightTitle; y++) {
            for (int x = 0; x < lengthTitle; x++) {
                if (titles[titleNumber][y][x] == '@')
                    changeCell((COLS - lengthTitle) / 2 + x, startMenuPos + y);
            }
        }
    }

    void displayTitle() {
        if (isTitleRunning and time(NULL) - lastTime > ITERATIONTIME) {
            iterate();
            lastTime = time(NULL);
        }
        for (int y = 0; y < ROWS; y++) {
            for (int x = 0; x < COLS; x++) {
                PrevGameTable[y][x] ? mvwaddch(stdscr, y, x, '@') : mvwaddch(stdscr, y, x, ' ');
            }
        }
    }

    void DispTextCenter(const char* str, int shiftY, int shiftX = 0) const {
        int currY = (ROWS / 2) + shiftY;
        int currX = ((COLS - strlen(str)) / 2) + shiftX;
        mvprintw(currY, currX, "%s", str);
    }

public:
    MainMenu() : GameOfLife(ROWS, COLS), startMenuPos(ROWS * 0.333 - 3), itemsOffset(8), isTitleRunning(false), lastTime(0) {}

    void run() {
        int mainMenuChoice = 0;
        int sizeMainMenu = mainMenuItems.size();
        int lengthMainMenu = mainMenuItems[2].size();

        updateField(0);
        while (true) {
            displayTitle();
            for (int i = 0; i < mainMenuItems.size(); i++) {
                move(startMenuPos + itemsOffset + i, (COLS - lengthMainMenu) / 2 - 1);
                i == mainMenuChoice ? printw(">%s", mainMenuItems[i]) : printw(" %s", mainMenuItems[i]);
            }

            switch (getch()) {
            case KEY_END:
                isTitleRunning = !isTitleRunning;
                break;
            case KEY_UP:
                if (mainMenuChoice > 0) mainMenuChoice--;
                else mainMenuChoice = sizeMainMenu - 1;
                break;
            case KEY_DOWN:
                if (mainMenuChoice < sizeMainMenu - 1) mainMenuChoice++;
                else mainMenuChoice = 0;
                break;
            case KEY_CONFIRM:
                clear();
                refresh();
                switch (mainMenuChoice) {
                case 0:
                    myGame.run();
                    updateField(0);
                    break;
                case 1:
                    updateField(1);
                    showOptions();
                    updateField(0);
                    break;
                case 2:
                    updateField(2);
                    showTutorial();
                    updateField(0);
                    break;
                case 3:
                    updateField(3);
                    showCredits();
                    updateField(0);
                    break;
                case 4:
                    updateField(4);
                    if (exitProgram())
                        return;
                    updateField(0);
                    break;
                }
                break;
            }
        }
    }
};


int main() {
    initscr();
    keypad(stdscr, true);
    curs_set(0);
    noecho();
    cbreak();
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_RED);
    getmaxyx(stdscr, ROWS, COLS);
    timeout(0);

    MainMenu myMenu;
    myMenu.run();

    endwin();
    return 0;
}

void MainMenu::showOptions() { //реализация функции проклята
    int optionMenuChoice = 0;
    int sizeOptionMenu = 4;
    int offsetX = (COLS - 16) / 2;
    int offsetY = startMenuPos + itemsOffset;
    bool isEdit = false;

    while (true) {
        displayTitle();

        move(offsetY + 0, offsetX);
        if (optionMenuChoice == 0 and !isEdit) {
            printw(">LightMode: [");
            ISLIGHTMODE ? printw("x]") : printw(" ]");
        }
        else if (optionMenuChoice == 0 and isEdit) {
            mvprintw(offsetY + 0, offsetX, ">LightMode: ");
            mvprintw(offsetY + 1, offsetX, " Iteration time: %.1f", ITERATIONTIME);
            mvprintw(offsetY + 2, offsetX, " Table size: %d %d", TABLEHEIGHT, TABLEWIDTH);
            mvprintw(offsetY + 3, offsetX, " Console size: %d %d", ROWS, COLS);
            move(offsetY + 0, offsetX + 12);
            reverseColor();
            isEdit = false;
        }
        else {
            printw(" LightMode: [");
            ISLIGHTMODE ? printw("x]") : printw(" ]");
        }

        move(offsetY + 1, offsetX);
        if (optionMenuChoice == 1 and !isEdit)
            printw(">Iteration time(sec): %.1f", ITERATIONTIME);
        else if (optionMenuChoice == 1 and isEdit) {
            mvprintw(offsetY + 1, offsetX, ">Iteration time(sec): ");
            mvprintw(offsetY + 2, offsetX, " Table size: %d %d", TABLEHEIGHT, TABLEWIDTH);
            mvprintw(offsetY + 3, offsetX, " Console size: %d %d", ROWS, COLS);
            move(offsetY + 1, offsetX + 22);
            double newIterationTime;
            char inputStr[15];
            echo();
            getstr(inputStr);
            sscanf_s(inputStr, "%lf", &newIterationTime);
            if (newIterationTime > 0.0)
                ITERATIONTIME = newIterationTime;
            else {
                clear();
                attron(COLOR_PAIR(1));
                DispTextCenter("Invalid input. Please enter positive float value.", -2);
                DispTextCenter("             Press ESC to continue...            ", -1);
                attroff(COLOR_PAIR(1));
                while (!(getch() == KEY_ESC));
            }
            noecho();
            isEdit = false;
        }
        else
            printw(" Iteration time: %.1f", ITERATIONTIME);

        move(offsetY + 2, offsetX);
        if (optionMenuChoice == 2 and !isEdit)
            printw(">Table size(H W): %d %d", TABLEHEIGHT, TABLEWIDTH);
        else if (optionMenuChoice == 2 and isEdit) {
            mvprintw(offsetY + 2, offsetX, ">Table size(H W): ");
            mvprintw(offsetY + 3, offsetX, " Console size: %d %d", ROWS, COLS);
            move(offsetY + 2, offsetX + 18);
            int newWidth, newHeight;
            echo();
            scanw("%d %d", &newHeight, &newWidth);
            if (newHeight >= 1 and newWidth >= 1 and newHeight <= 100 and newWidth <= 200) {
                TABLEHEIGHT = newHeight;
                TABLEWIDTH = newWidth;
                myGame.resize(TABLEHEIGHT, TABLEWIDTH);
            }
            else {
                clear();
                attron(COLOR_PAIR(1));          
                DispTextCenter("                  Invalid input                     ", -2);
                DispTextCenter(" The range of acceptable table sizes: 1x1 - 100x200 ", -1);
                DispTextCenter("              Press ESC to continue...              ",  0);
                attroff(COLOR_PAIR(1));
                while (!(getch() == KEY_ESC));
            }
            noecho();
            isEdit = false;
        }
        else
            printw(" Table size: %d %d", TABLEHEIGHT, TABLEWIDTH);

        move(offsetY + 3, offsetX);
        if (optionMenuChoice == 3 and !isEdit)
            printw(">Console size(H W): %d %d", ROWS, COLS);
        else if (optionMenuChoice == 3 and isEdit) {
            mvprintw(offsetY + 3, offsetX, ">Console size(H W): ");
            move(offsetY + 3, offsetX + 20);
            int newRows, newCols;
            echo();
            scanw("%d %d", &newRows, &newCols);
            if (newRows >= 20 and newCols >= 90 and newRows <= 90 and newCols <= 200) {
                ROWS = newRows;
                COLS = newCols;
                resize_term(newRows, newCols);
                refresh();
            }
            else {
                clear();
                attron(COLOR_PAIR(1));
                DispTextCenter("                    Invalid input                       ", -2);
                DispTextCenter(" The range of acceptable console sizes: 20x90 - 90x200 ", -1);
                DispTextCenter("                Press ESC to continue...                ",  0);
                attroff(COLOR_PAIR(1));
                while (!(getch() == KEY_ESC));
            }
            resize(ROWS, COLS);
            startMenuPos = ROWS * 0.333 - 3;
            noecho();
            isEdit = false;
        }
        else
            printw(" Console size: %d %d", ROWS, COLS);

        switch (getch()) {
        case KEY_UP:
            if (optionMenuChoice > 0) optionMenuChoice--;
            else optionMenuChoice = sizeOptionMenu - 1;
            break;
        case KEY_DOWN:
            if (optionMenuChoice < sizeOptionMenu - 1) optionMenuChoice++;
            else optionMenuChoice = 0;
            break;
        case KEY_CONFIRM:
            isEdit = true;
            break;
        case KEY_END:
            isTitleRunning = !isTitleRunning;
            break;
        case KEY_ESC:
            return;
            break;
        }
    }
}

void MainMenu::showTutorial() {
    while (true) {
        displayTitle();
        DispTextCenter("This is the game of life", 0);
        DispTextCenter("A dead cell comes to life if there are 3 living cells near it", 1);
        DispTextCenter("A living cell dies if there are less than 2 and more than 3 living cells near it", 2);
        DispTextCenter("The boundaries of the game are interconnected. Press the END key)", 3);
        DispTextCenter("Press ESC to exit...", 4);

        switch (getch()) {
        case KEY_END:
            isTitleRunning = !isTitleRunning;
            break;
        case KEY_ESC:
            return;
            break;
        }
    }
}

void MainMenu::showCredits() {
    while (true) {
        displayTitle();
        DispTextCenter("Chernikov A.M. 20.11.2023 - 07.05.2024", 0, -1);
        DispTextCenter("Press ESC to exit...", 1, -1);

        switch (getch()) {
        case KEY_END:
            isTitleRunning = !isTitleRunning;
            break;
        case KEY_ESC:
            return;
            break;
        }
    }
}

bool MainMenu::exitProgram() {
    while (true) {
        displayTitle();
        DispTextCenter("Do you want to exit? Y/N", 0, -1);

        int key = getch();
        if (key == KEY_END)
            isTitleRunning = !isTitleRunning;
        else if (key == KEY_N_LOWER or key == KEY_N_UPPER or key == KEY_ESC)
            return false;
        else if (key == KEY_Y_LOWER or key == KEY_Y_UPPER or key == KEY_CONFIRM)
            return true;
    }
}