#include <ncurses.h>
#include <stdio.h>

#define WIDTH 80
#define HEIGHT 25

int count_neighbors(int field[HEIGHT][WIDTH], int y, int x);
void next_gen(int field[HEIGHT][WIDTH]);
int read_field(int field[HEIGHT][WIDTH]);
void default_pattern(int field[HEIGHT][WIDTH]);
void draw(int field[HEIGHT][WIDTH], int delay);
int handle_keys(int delay);
void run_game(int field[HEIGHT][WIDTH], int delay);
int init_screen(void);

int main(void) {
    int field[HEIGHT][WIDTH] = {0};
    int delay = 300;
    int result = 1;

    if (!read_field(field)) {
        default_pattern(field);
    }

    if (init_screen()) {
        run_game(field, delay);
        endwin();
    } else {
        fprintf(stderr, "Failed to initialize screen\n");
        result = 1;
    }

    return result;
}

int count_neighbors(int field[HEIGHT][WIDTH], int y, int x) {
    int count = 0;

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dy != 0 || dx != 0) {
                int ny = (y + dy + HEIGHT) % HEIGHT;
                int nx = (x + dx + WIDTH) % WIDTH;
                count += field[ny][nx];
            }
        }
    }

    return count;
}

void next_gen(int field[HEIGHT][WIDTH]) {
    int new_field[HEIGHT][WIDTH] = {0};

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int n = count_neighbors(field, y, x);

            new_field[y][x] = (field[y][x] && (n == 2 || n == 3)) || (!field[y][x] && n == 3);
        }
    }

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            field[y][x] = new_field[y][x];
        }
    }
}

int read_field(int field[HEIGHT][WIDTH]) {
    int result = 1;
    int c = fgetc(stdin);

    if (c == EOF || c == '\n') {
        result = 0;
    } else {
        ungetc(c, stdin);

        for (int y = 0; y < HEIGHT && result; y++) {
            for (int x = 0; x < WIDTH && result; x++) {
                c = fgetc(stdin);

                if (c == '0' || c == '1') {
                    field[y][x] = (c == '1');
                } else {
                    result = 0;
                }
            }

            if (result) {
                c = fgetc(stdin);

                if (c == '\r') {
                    c = fgetc(stdin);
                }

                if (c != '\n' && c != EOF) {
                    result = 0;
                }

                if (c == EOF && y != HEIGHT - 1) {
                    result = 0;
                }
            }
        }
    }

    return result;
}

void default_pattern(int field[HEIGHT][WIDTH]) {
    field[1][2] = 1;
    field[2][3] = 1;
    field[3][1] = 1;
    field[3][2] = 1;
    field[3][3] = 1;

    field[10][10] = 1;
    field[10][11] = 1;
    field[11][10] = 1;
    field[11][11] = 1;
}

void draw(int field[HEIGHT][WIDTH], int delay) {
    clear();

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            mvaddch(y, x, field[y][x] ? 'O' : '.');
        }
    }

    mvprintw(HEIGHT + 1, 0,"Speed: %d ms | A-faster Z-slower Space-exit", delay);
    refresh();
}

int handle_keys(int delay) {
    int ch = getch();
    int new_delay = delay;

    if (ch == 'a' || ch == 'A') {
        new_delay = (delay > 50) ? delay - 50 : 50;
    } else if (ch == 'z' || ch == 'Z') {
        new_delay = (delay < 500) ? delay + 50 : 500;
    } else if (ch == ' ') {
        new_delay = -1;
    }

    return new_delay;
}

void run_game(int field[HEIGHT][WIDTH], int delay) {
    int running = 1;

    while (running) {
        draw(field, delay);
        delay = handle_keys(delay);

        if (delay == -1) {
            running = 0;
        } else {
            next_gen(field);
            napms(delay);
        }
    }
}

int init_screen(void) {
    int result = 0;

    if (freopen("/dev/tty", "r", stdin) != NULL) {
        if (initscr() != NULL) {
            cbreak();
            noecho();
            curs_set(0);
            keypad(stdscr, TRUE);
            timeout(0);
            result = 1;
        }
    }

    return result;
}