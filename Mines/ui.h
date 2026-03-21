#ifndef UI_H
#define UI_H

#include <gtk/gtk.h>
#include "logic.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *buttons[ROWS][COLS];
    GtkWidget *status_label;
    GtkWidget *grid;
    Cell board[ROWS][COLS];
    int time_left;
    guint timer_id; // لإدارة المؤقت
} GameData;

void create_window(int argc, char *argv[]);
void reset_game(GameData *data); // خاصية الـ Restart

#endif
