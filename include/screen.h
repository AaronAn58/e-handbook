#ifndef SCREEN_H
#define SCREEN_H

#include <SPI.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include "DEV_Config.h"

#include <U8g2_for_Adafruit_GFX.h>
#include <U8g2lib.h>

void init_screen();

void line_pos_show_menu(const char **str, int line_size, int show_line);

void multi_line_menu_show(const char **str, int line_size, int show_line);

uint16_t text_multi_line_show(const char *str, bool contains_end);

void center_tip(const char* tip);

#endif