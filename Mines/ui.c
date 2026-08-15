#include "ui.h"
#include <stdio.h>
#include <stdlib.h>

static void on_click(GtkWidget *widget, gpointer user_data);
static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

// دالة تطبيق الثيمات مع دعم المسار المحلي ومسار النظام الرسمي
static void apply_css_theme(const char *filename, GameData *data) {
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);

    gtk_style_context_add_provider_for_screen(screen, 
        GTK_STYLE_PROVIDER(provider), 
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    char filepath[256];
    FILE *f = fopen(filename, "r");
    if (f) {
        fclose(f);
        snprintf(filepath, sizeof(filepath), "%s", filename);
    } else {
        snprintf(filepath, sizeof(filepath), "/usr/share/hel-mines/%s", filename);
    }

    GError *error = NULL;
    gtk_css_provider_load_from_path(provider, filepath, &error);
    if (error) {
        g_warning("Failed to load CSS from %s: %s", filepath, error->message);
        g_error_free(error);
    }
    g_object_unref(provider);
}

static void set_theme_classic(GtkWidget *w, gpointer data) {
    apply_css_theme("style_classic.css", (GameData*)data);
}

static void set_theme_nord(GtkWidget *w, gpointer data) {
    apply_css_theme("style_nord.css", (GameData*)data);
}

static void set_theme_matrix(GtkWidget *w, gpointer data) {
    apply_css_theme("style_matrix.css", (GameData*)data);
}

static gboolean hide_label_timeout(gpointer button) {
    if (GTK_IS_BUTTON(button)) {
        const gchar *current_label = gtk_button_get_label(GTK_BUTTON(button));
        if (current_label && current_label[0] != '.' && current_label[0] != '\0') {
            gtk_button_set_label(GTK_BUTTON(button), "."); 
        }
    }
    return FALSE; 
}

static void update_status_labels(GameData *data) {
    char buf[64];
    sprintf(buf, "Time: %ds ⏱️ | Score: %d | High: %d | Pings: %d 🔍", data->time_left, data->score, data->high_score, data->pings_left);
    gtk_label_set_text(GTK_LABEL(data->status_label), buf);
}

static gboolean timer_tick(gpointer user_data) {
    GameData *data = (GameData*)user_data;
    if (!GTK_IS_WIDGET(data->window)) return FALSE;

    data->time_left--;
    update_status_labels(data);

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

void reset_game(GameData *data) {
    if (data->timer_id > 0) {
        g_source_remove(data->timer_id);
    }
    
    GtkWidget *parent = gtk_widget_get_parent(data->grid);
    if (parent) {
        gtk_container_remove(GTK_CONTAINER(parent), data->grid);
    }
    
    if (data->current_difficulty == DIFFICULTY_EASY) {
        data->rows = 8; data->cols = 8; data->mines = 8; data->time_left = 45;
    } else if (data->current_difficulty == DIFFICULTY_MEDIUM) {
        data->rows = 10; data->cols = 10; data->mines = 15; data->time_left = 60;
    } else {
        data->rows = 12; data->cols = 12; data->mines = 25; data->time_left = 90;
    }

    init_board(data->board, data->rows, data->cols, data->mines);
    data->pings_left = 2;
    data->flag_mode = false;

    data->grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(data->grid), 2);
    gtk_grid_set_column_spacing(GTK_GRID(data->grid), 2);
    
    GtkWidget *vbox = gtk_bin_get_child(GTK_BIN(data->window));
    gtk_box_pack_start(GTK_BOX(vbox), data->grid, TRUE, TRUE, 10);
    gtk_widget_set_halign(data->grid, GTK_ALIGN_CENTER);

    for(int i = 0; i < data->rows; i++) {
        for(int j = 0; j < data->cols; j++) {
            data->buttons[i][j] = gtk_button_new();
            gtk_widget_set_size_request(data->buttons[i][j], 35, 35);
            g_object_set_data(G_OBJECT(data->buttons[i][j]), "row", GINT_TO_POINTER(i));
            g_object_set_data(G_OBJECT(data->buttons[i][j]), "col", GINT_TO_POINTER(j));
            
            g_signal_connect(data->buttons[i][j], "clicked", G_CALLBACK(on_click), data);
            g_signal_connect(data->buttons[i][j], "button-press-event", G_CALLBACK(on_button_press), data);
            
            gtk_grid_attach(GTK_GRID(data->grid), data->buttons[i][j], j, i, 1, 1);
        }
    }
    
    update_status_labels(data);
    data->timer_id = g_timeout_add_seconds(1, timer_tick, data);
    gtk_widget_show_all(data->window);
}

static void show_about(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_about_dialog_new();
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Helwan Mines Pro");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "2.0");
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "© 2026 Saeed Badreldin");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), 
        "Professional edition featuring Difficulty Levels, Ping Scan, Themes, and Data Decay mechanics.");
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "https://helwan-linux.github.io/helwanlinux/");
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(((GameData*)data)->window));
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void show_help(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(((GameData*)data)->window),
        GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
        "How to Play:\n\n"
        "1. Data Decay: Revealed numbers fade into dots after 5 seconds.\n"
        "2. Ping Scan: Use the scan tool to reveal safe hint cells.\n"
        "3. Flag Mode: Right-click buttons to place flags (🚩).\n"
        "4. Themes: Switch between Classic, Nordic, and Matrix themes instantly!\n"
        "5. Time & Score: Beat the timer to accumulate high scores!");
    gtk_window_set_title(GTK_WINDOW(dialog), "Game Manual");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void update_ui(GameData *data) {
    for(int i = 0; i < data->rows; i++) {
        for(int j = 0; j < data->cols; j++) {
            GtkWidget *btn = data->buttons[i][j];
            Cell *cell = &data->board[i][j];
            if (cell->is_flagged) {
                gtk_button_set_label(GTK_BUTTON(btn), "🚩");
            } else if(cell->is_revealed && gtk_widget_get_sensitive(btn)) {
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

    if (data->board[r][c].is_flagged) return;

    if (!reveal_cell(data->board, data->rows, data->cols, r, c)) {
        update_ui(data);
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(data->window), 
            GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "BOOM! Restart?");
        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) reset_game(data);
        else exit(0);
        gtk_widget_destroy(dialog);
    } else {
        update_ui(data);
        if (check_win(data->board, data->rows, data->cols)) {
            data->score += data->time_left * 10;
            if (data->score > data->high_score) data->high_score = data->score;
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(data->window), 
                GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_YES_NO, "Winner! Play again?");
            if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) reset_game(data);
            else exit(0);
            gtk_widget_destroy(dialog);
        }
    }
}

static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    GameData *data = (GameData*)user_data;
    if (event->button == GDK_BUTTON_SECONDARY) { 
        int r = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "row"));
        int c = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "col"));
        if (!data->board[r][c].is_revealed) {
            data->board[r][c].is_flagged = !data->board[r][c].is_flagged;
            gtk_button_set_label(GTK_BUTTON(widget), data->board[r][c].is_flagged ? "🚩" : "");
        }
        return TRUE;
    }
    return FALSE;
}

static void use_ping_scan(GtkWidget *widget, gpointer user_data) {
    GameData *data = (GameData*)user_data;
    if (data->pings_left <= 0) {
        GtkWidget *dlg = gtk_message_dialog_new(GTK_WINDOW(data->window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "No Pings left!");
        gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy(dlg);
        return;
    }
    
    for (int i = 0; i < data->rows; i++) {
        for (int j = 0; j < data->cols; j++) {
            if (!data->board[i][j].is_mine && !data->board[i][j].is_revealed) {
                reveal_cell(data->board, data->rows, data->cols, i, j);
                data->pings_left--;
                update_ui(data);
                update_status_labels(data);
                return;
            }
        }
    }
}

static void set_difficulty_easy(GtkWidget *w, gpointer data) { ((GameData*)data)->current_difficulty = DIFFICULTY_EASY; reset_game(data); }
static void set_difficulty_medium(GtkWidget *w, gpointer data) { ((GameData*)data)->current_difficulty = DIFFICULTY_MEDIUM; reset_game(data); }
static void set_difficulty_hard(GtkWidget *w, gpointer data) { ((GameData*)data)->current_difficulty = DIFFICULTY_HARD; reset_game(data); }

void create_window(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    GameData *data = g_malloc(sizeof(GameData));
    data->current_difficulty = DIFFICULTY_MEDIUM;
    data->rows = 10; data->cols = 10; data->mines = 15;
    data->time_left = 60;
    data->score = 0;
    data->high_score = 0;
    data->pings_left = 2;
    data->timer_id = 0;

    data->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(data->window), "Helwan Mines Pro");
    gtk_window_set_default_size(GTK_WINDOW(data->window), 450, 550);
    gtk_window_set_icon_from_file(GTK_WINDOW(data->window), "icon.png", NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(data->window), vbox);

    // Menu Bar
    GtkWidget *menubar = gtk_menu_bar_new();
    GtkWidget *game_mi = gtk_menu_item_new_with_label("Game");
    GtkWidget *diff_mi = gtk_menu_item_new_with_label("Difficulty");
    GtkWidget *tools_mi = gtk_menu_item_new_with_label("Tools");
    GtkWidget *theme_mi = gtk_menu_item_new_with_label("Themes");
    GtkWidget *help_mi = gtk_menu_item_new_with_label("Help");
    
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), game_mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), diff_mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), tools_mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), theme_mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help_mi);

    // Game Submenu
    GtkWidget *game_menu = gtk_menu_new();
    GtkWidget *restart_item = gtk_menu_item_new_with_label("Restart");
    GtkWidget *quit_item = gtk_menu_item_new_with_label("Quit");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(game_mi), game_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(game_menu), restart_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(game_menu), quit_item);

    // Difficulty Submenu
    GtkWidget *diff_menu = gtk_menu_new();
    GtkWidget *easy_item = gtk_menu_item_new_with_label("Easy (8x8)");
    GtkWidget *med_item = gtk_menu_item_new_with_label("Medium (10x10)");
    GtkWidget *hard_item = gtk_menu_item_new_with_label("Hard (12x12)");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(diff_mi), diff_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(diff_menu), easy_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(diff_menu), med_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(diff_menu), hard_item);

    // Tools Submenu
    GtkWidget *tools_menu = gtk_menu_new();
    GtkWidget *ping_item = gtk_menu_item_new_with_label("Ping Scan (Hint)");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(tools_mi), tools_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(tools_menu), ping_item);

    // Themes Submenu
    GtkWidget *theme_menu = gtk_menu_new();
    GtkWidget *classic_item = gtk_menu_item_new_with_label("Classic (Default)");
    GtkWidget *nord_item = gtk_menu_item_new_with_label("Nordic Dark");
    GtkWidget *matrix_item = gtk_menu_item_new_with_label("Matrix Green");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(theme_mi), theme_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(theme_menu), classic_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(theme_menu), nord_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(theme_menu), matrix_item);

    // Help Submenu
    GtkWidget *help_menu = gtk_menu_new();
    GtkWidget *how_item = gtk_menu_item_new_with_label("How to Play");
    GtkWidget *about_item = gtk_menu_item_new_with_label("About");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_mi), help_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), how_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about_item);

    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    g_signal_connect_swapped(restart_item, "activate", G_CALLBACK(reset_game), data);
    g_signal_connect(quit_item, "activate", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(easy_item, "activate", G_CALLBACK(set_difficulty_easy), data);
    g_signal_connect(med_item, "activate", G_CALLBACK(set_difficulty_medium), data);
    g_signal_connect(hard_item, "activate", G_CALLBACK(set_difficulty_hard), data);
    g_signal_connect(ping_item, "activate", G_CALLBACK(use_ping_scan), data);
    g_signal_connect(classic_item, "activate", G_CALLBACK(set_theme_classic), data);
    g_signal_connect(nord_item, "activate", G_CALLBACK(set_theme_nord), data);
    g_signal_connect(matrix_item, "activate", G_CALLBACK(set_theme_matrix), data);
    g_signal_connect(how_item, "activate", G_CALLBACK(show_help), data);
    g_signal_connect(about_item, "activate", G_CALLBACK(show_about), data);

    data->status_label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(vbox), data->status_label, FALSE, FALSE, 10);

    // تحميل الثيم الكلاسيكي عند البداية
    apply_css_theme("style_classic.css", data);

    // بناء اللوحة الأولية
    data->grid = gtk_grid_new();
    init_board(data->board, data->rows, data->cols, data->mines);

    for(int i = 0; i < data->rows; i++) {
        for(int j = 0; j < data->cols; j++) {
            data->buttons[i][j] = gtk_button_new();
            gtk_widget_set_size_request(data->buttons[i][j], 35, 35);
            g_object_set_data(G_OBJECT(data->buttons[i][j]), "row", GINT_TO_POINTER(i));
            g_object_set_data(G_OBJECT(data->buttons[i][j]), "col", GINT_TO_POINTER(j));
            g_signal_connect(data->buttons[i][j], "clicked", G_CALLBACK(on_click), data);
            g_signal_connect(data->buttons[i][j], "button-press-event", G_CALLBACK(on_button_press), data);
            gtk_grid_attach(GTK_GRID(data->grid), data->buttons[i][j], j, i, 1, 1);
        }
    }
    gtk_box_pack_start(GTK_BOX(vbox), data->grid, TRUE, TRUE, 10);
    gtk_widget_set_halign(data->grid, GTK_ALIGN_CENTER);

    update_status_labels(data);
    data->timer_id = g_timeout_add_seconds(1, timer_tick, data);
    g_signal_connect(data->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(data->window);
    gtk_main();
}
