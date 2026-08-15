#include "logic.h"
#include <stdlib.h>
#include <time.h>

void init_board(Cell board[MAX_ROWS][MAX_COLS], int rows, int cols, int mines) {
    srand(time(NULL));
    for(int i=0; i<rows; i++) {
        for(int j=0; j<cols; j++) {
            board[i][j].is_mine = false;
            board[i][j].neighbor_mines = 0;
            board[i][j].is_revealed = false;
            board[i][j].is_flagged = false;
        }
    }

    int placed = 0;
    while(placed < mines) {
        int r = rand() % rows, c = rand() % cols;
        if(!board[r][c].is_mine) {
            board[r][c].is_mine = true;
            placed++;
        }
    }

    for(int r=0; r<rows; r++) {
        for(int c=0; c<cols; c++) {
            if(board[r][c].is_mine) continue;
            int count = 0;
            for(int dr=-1; dr<=1; dr++) {
                for(int dc=-1; dc<=1; dc++) {
                    int nr = r+dr, nc = c+dc;
                    if(nr>=0 && nr<rows && nc>=0 && nc<cols && board[nr][nc].is_mine)
                        count++;
                }
            }
            board[r][c].neighbor_mines = count;
        }
    }
}

bool reveal_cell(Cell board[MAX_ROWS][MAX_COLS], int rows, int cols, int r, int c) {
    if(r<0 || r>=rows || c<0 || c>=cols || board[r][c].is_revealed || board[r][c].is_flagged) return true;
    board[r][c].is_revealed = true;
    if(board[r][c].is_mine) return false;
    if(board[r][c].neighbor_mines == 0) {
        for(int dr=-1; dr<=1; dr++)
            for(int dc=-1; dc<=1; dc++) reveal_cell(board, rows, cols, r+dr, c+dc);
    }
    return true;
}

bool check_win(Cell board[MAX_ROWS][MAX_COLS], int rows, int cols) {
    for(int i=0; i<rows; i++)
        for(int j=0; j<cols; j++)
            if(!board[i][j].is_mine && !board[i][j].is_revealed) return false;
    return true;
}
