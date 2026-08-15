#ifndef UI_H
#define UI_H

#include <gtk/gtk.h>
#include "logic.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *buttons[MAX_ROWS][MAX_COLS];
    GtkWidget *status_label;
    GtkWidget *score_label;
    GtkWidget *grid;
    Cell board[MAX_ROWS][MAX_COLS];
    int rows;
    int cols;
    int mines;
    int time_left;
    int score;
    int high_score;
    int pings_left;
    bool flag_mode;
    guint timer_id;
    Difficulty current_difficulty;
} GameData;

void create_window(int argc, char *argv[]);
void reset_game(GameData *data);

#endif
