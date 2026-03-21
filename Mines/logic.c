#include "logic.h"
#include <stdlib.h>
#include <time.h>

void init_board(Cell board[ROWS][COLS]) {
    srand(time(NULL));
    for(int i=0; i<ROWS; i++) {
        for(int j=0; j<COLS; j++) {
            board[i][j].is_mine = false;
            board[i][j].neighbor_mines = 0;
            board[i][j].is_revealed = false;
            board[i][j].is_flagged = false;
        }
    }

    int placed = 0;
    while(placed < MINES) {
        int r = rand() % ROWS, c = rand() % COLS;
        if(!board[r][c].is_mine) {
            board[r][c].is_mine = true;
            placed++;
        }
    }

    for(int r=0; r<ROWS; r++) {
        for(int c=0; c<COLS; c++) {
            if(board[r][c].is_mine) continue;
            int count = 0;
            for(int dr=-1; dr<=1; dr++) {
                for(int dc=-1; dc<=1; dc++) {
                    int nr = r+dr, nc = c+dc;
                    if(nr>=0 && nr<ROWS && nc>=0 && nc<COLS && board[nr][nc].is_mine)
                        count++;
                }
            }
            board[r][c].neighbor_mines = count;
        }
    }
}

bool reveal_cell(Cell board[ROWS][COLS], int r, int c) {
    if(r<0 || r>=ROWS || c<0 || c>=COLS || board[r][c].is_revealed) return true;
    board[r][c].is_revealed = true;
    if(board[r][c].is_mine) return false;
    if(board[r][c].neighbor_mines == 0) {
        for(int dr=-1; dr<=1; dr++)
            for(int dc=-1; dc<=1; dc++) reveal_cell(board, r+dr, c+dc);
    }
    return true;
}

bool check_win(Cell board[ROWS][COLS]) {
    for(int i=0; i<ROWS; i++)
        for(int j=0; j<COLS; j++)
            if(!board[i][j].is_mine && !board[i][j].is_revealed) return false;
    return true;
}
