#ifndef SCREEN_H
#define SCREEN_H

#include <SPI.h>

void init_screen(SPIClass &spi);
void line_pos_show_menu(const char **str, int line_size, int show_line);

#endif