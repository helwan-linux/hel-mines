//gcc main.c logic.c ui.c -o helwan_mines_pro `pkg-config --cflags --libs gtk+-3.0 gdk-pixbuf-2.0`
#include "ui.h"

int main(int argc, char *argv[]) {
    create_window(argc, argv);
    return 0;
}
