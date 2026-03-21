#include "ui.h"
#include "logic.h"
#include <stdio.h>
#include <stdlib.h>

// 1. دالة لإخفاء النص بعد فترة (ميكانيكية Data Decay)
static gboolean hide_label_timeout(gpointer button) {
    if (GTK_IS_BUTTON(button)) {
        gtk_button_set_label(GTK_BUTTON(button), "."); 
    }
    return FALSE; 
}

// 2. دالة المؤقت (التي كانت ناقصة وسببت الخطأ)
static gboolean timer_tick(gpointer user_data) {
    GameData *data = (GameData*)user_data;
    if (!GTK_IS_WIDGET(data->window)) return FALSE;

    data->time_left--;
    char buf[32];
    sprintf(buf, "Time Left: %ds ⏱️", data->time_left);
    gtk_label_set_text(GTK_LABEL(data->status_label), buf);

    if (data->time_left <= 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(data->window), 
                            GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, 
                            GTK_BUTTONS_YES_NO, "انتهى الوقت! هل تعيد المحاولة؟");
        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) {
            reset_game(data);
            gtk_widget_destroy(dialog);
        } else { exit(0); }
    }
    return TRUE;
}

// 3. دالة إعادة التشغيل
void reset_game(GameData *data) {
    init_board(data->board);
    data->time_left = 60;
    // إعادة تعيين شكل الشبكة
    for(int i = 0; i < ROWS; i++) {
        for(int j = 0; j < COLS; j++) {
            gtk_widget_set_sensitive(data->buttons[i][j], TRUE);
            gtk_button_set_label(GTK_BUTTON(data->buttons[i][j]), "");
        }
    }
    gtk_label_set_text(GTK_LABEL(data->status_label), "Time Left: 60s ⏱️");
}

// 4. نوافذ About و Help
static void show_about(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_about_dialog_new();
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Helwan Mines Pro");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "1.0");
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "© 2026 Saeed Badreldin");
    
    // الوصف بالإنجليزية مع الإشارة لميزة تلاشي البيانات المميزة
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), 
        "Professional edition for Helwan Linux featuring unique Data Decay mechanics.");
    
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "https://helwan-linux.github.io/helwanlinux/");
    
    // ربط النافذة بالنافذة الرئيسية
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(((GameData*)data)->window));
    
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void show_help(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(((GameData*)data)->window),
        GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
        "How to Play:\n\n"
        "1. Data Decay: Revealed numbers fade away after 5 seconds (turning into dots).\n"
        "2. Time Pressure: You must clear the field before the 60-second timer hits zero.\n"
        "3. Anomalies: Clicking a mine (anomaly) results in an immediate explosion.");
    
    gtk_window_set_title(GTK_WINDOW(dialog), "Game Manual");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// 5. تحديث الواجهة عند الضغط
static void update_ui(GameData *data) {
    for(int i = 0; i < ROWS; i++) {
        for(int j = 0; j < COLS; j++) {
            GtkWidget *btn = data->buttons[i][j];
            Cell *cell = &data->board[i][j];
            if(cell->is_revealed && gtk_widget_get_sensitive(btn)) {
                gtk_widget_set_sensitive(btn, FALSE);
                if(cell->is_mine) {
                    gtk_button_set_label(GTK_BUTTON(btn), "☢");
                } else if (cell->neighbor_mines > 0) {
                    char label[2];
                    sprintf(label, "%d", cell->neighbor_mines);
                    gtk_button_set_label(GTK_BUTTON(btn), label);
                    g_timeout_add_seconds(5, hide_label_timeout, btn);
                } else {
                    gtk_button_set_label(GTK_BUTTON(btn), " ");
                }
            }
        }
    }
}

static void on_click(GtkWidget *widget, gpointer user_data) {
    GameData *data = (GameData*)user_data;
    int r = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "row"));
    int c = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "col"));

    if (!reveal_cell(data->board, r, c)) {
        update_ui(data);
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(data->window), 
            GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "BOOM! Restart?");
        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) reset_game(data);
        else exit(0);
        gtk_widget_destroy(dialog);
    } else {
        update_ui(data);
        if (check_win(data->board)) {
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(data->window), 
                GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_YES_NO, "Winner! Play again?");
            if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) reset_game(data);
            else exit(0);
            gtk_widget_destroy(dialog);
        }
    }
}

// 6. إنشاء النافذة
void create_window(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    GameData *data = g_malloc(sizeof(GameData));
    data->time_left = 60;
    init_board(data->board);

    data->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(data->window), "Helwan Mines Pro");
    gtk_window_set_default_size(GTK_WINDOW(data->window), 400, 500);
    gtk_window_set_icon_from_file(GTK_WINDOW(data->window), "icon.png", NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(data->window), vbox);

    // Menu Bar
    GtkWidget *menubar = gtk_menu_bar_new();
    GtkWidget *game_mi = gtk_menu_item_new_with_label("Game");
    GtkWidget *help_mi = gtk_menu_item_new_with_label("Help");
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), game_mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help_mi);

    GtkWidget *game_menu = gtk_menu_new();
    GtkWidget *restart_item = gtk_menu_item_new_with_label("Restart");
    GtkWidget *quit_item = gtk_menu_item_new_with_label("Quit");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(game_mi), game_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(game_menu), restart_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(game_menu), quit_item);

    GtkWidget *help_menu = gtk_menu_new();
    GtkWidget *how_item = gtk_menu_item_new_with_label("How to Play");
    GtkWidget *about_item = gtk_menu_item_new_with_label("About");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_mi), help_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), how_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about_item);

    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    g_signal_connect_swapped(restart_item, "activate", G_CALLBACK(reset_game), data);
    g_signal_connect(quit_item, "activate", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(how_item, "activate", G_CALLBACK(show_help), data);
    g_signal_connect(about_item, "activate", G_CALLBACK(show_about), data);

    data->status_label = gtk_label_new("Time Left: 60s ⏱️");
    gtk_box_pack_start(GTK_BOX(vbox), data->status_label, FALSE, FALSE, 10);

    data->grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(data->grid), 2);
    gtk_grid_set_column_spacing(GTK_GRID(data->grid), 2);
    gtk_box_pack_start(GTK_BOX(vbox), data->grid, TRUE, TRUE, 10);
    gtk_widget_set_halign(data->grid, GTK_ALIGN_CENTER);

    for(int i = 0; i < ROWS; i++) {
        for(int j = 0; j < COLS; j++) {
            data->buttons[i][j] = gtk_button_new();
            gtk_widget_set_size_request(data->buttons[i][j], 40, 40);
            g_object_set_data(G_OBJECT(data->buttons[i][j]), "row", GINT_TO_POINTER(i));
            g_object_set_data(G_OBJECT(data->buttons[i][j]), "col", GINT_TO_POINTER(j));
            g_signal_connect(data->buttons[i][j], "clicked", G_CALLBACK(on_click), data);
            gtk_grid_attach(GTK_GRID(data->grid), data->buttons[i][j], j, i, 1, 1);
        }
    }

    data->timer_id = g_timeout_add_seconds(1, timer_tick, data);
    g_signal_connect(data->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(data->window);
    gtk_main();
}
