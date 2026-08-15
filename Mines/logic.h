#ifndef LOGIC_H
#define LOGIC_H

#include <stdbool.h>

#define MAX_ROWS 15
#define MAX_COLS 15

typedef enum {
    DIFFICULTY_EASY,
    DIFFICULTY_MEDIUM,
    DIFFICULTY_HARD
} Difficulty;

typedef struct {
    bool is_mine;
    int neighbor_mines;
    bool is_revealed;
    bool is_flagged;
} Cell;

void init_board(Cell board[MAX_ROWS][MAX_COLS], int rows, int cols, int mines);
bool reveal_cell(Cell board[MAX_ROWS][MAX_COLS], int rows, int cols, int r, int c);
bool check_win(Cell board[MAX_ROWS][MAX_COLS], int rows, int cols);

#endif
