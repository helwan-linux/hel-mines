#ifndef LOGIC_H
#define LOGIC_H

#include <stdbool.h>

#define ROWS 10
#define COLS 10
#define MINES 15

typedef struct {
    bool is_mine;
    int neighbor_mines;
    bool is_revealed;
    bool is_flagged; // أضفنا هذا الحقل لتجنب الـ warning
} Cell;

void init_board(Cell board[ROWS][COLS]);
bool reveal_cell(Cell board[ROWS][COLS], int r, int c);
bool check_win(Cell board[ROWS][COLS]);

#endif
