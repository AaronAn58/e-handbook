#include "screen.h"
#include "EPD_2in9_V2.h"

GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(EPD_CS_PIN, EPD_DC_PIN, EPD_RST_PIN, EPD_BUSY_PIN));

extern const uint8_t chinese_gb2312[253023] U8G2_FONT_SECTION("chinese_gb2312");

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

uint8_t cn_char_len = 0;
uint8_t en_char_len = 0;
uint16_t screen_w = 0;

void init_screen()
{
    DEV_Module_Init();
    EPD_2IN9_V2_Init();
    EPD_2IN9_V2_Clear();
}

void screen_draw_white() {
    do
    {
        /* code */
        display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);
    } while (display.nextPage());
    delay(100);
};

void refresh_screen()
{
    do{

    }while(display.nextPage());
    delay(100);
};

void line_pos_show_menu(const char **str, int line_size, int show_line) {
    uint16_t left_x = 15;
    uint16_t init_height = 15;
    uint16_t h_gap = 5;
    int box_h = line_size * init_height + ((line_size - 1) * h_gap);

    int box_w = 4 * cn_char_len + 10;

    int box_y = show_line * init_height + ((show_line - 1) * h_gap);
    if (box_y < 0) {
        box_y = 0;
    } else if (box_y + box_h > display.height()) {
        box_y -= (box_y + box_h - display.height());
        box_y = box_y >0 ? box_y : 0;
    };

    int box_x = display.width() / 2;
    display.setPartialWindow(box_x, box_y, box_w, box_h);
    display.firstPage();

    int i = 0;

    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);

    int cursor_x = box_x + 5;
    int cursor_y = box_y + cn_char_len + 2;

    while (i < line_size) {
        // 设置文字
        u8g2Fonts.setCursor(cursor_x, cursor_y);
        u8g2Fonts.print(str[i++]);

        // 下划线
        display.drawFastHLine(cursor_x, cursor_y + 3, box_w, GxEPD_BLACK);
        cursor_y += cn_char_len + h_gap;
    };
    refresh_screen();
    // 关闭屏幕电源
    display.hibernate();
};


void multi_line_menu_show(const char **str, int line_num)
{
    screen_draw_white();
    display.setPartialWindow(0, 0, display.width(), display.height());
    display.firstPage();
    uint16_t left_x = 15;
    uint16_t init_height = 15;
    uint16_t h_gap = 5;
    Serial.println(line_num);

    int i = 0;
    while (i < line_num) {
        display.setCursor(left_x, init_height);
        u8g2Fonts.setCursor(left_x, init_height);
        init_height += cn_char_len + h_gap;
        u8g2Fonts.println(str[i++]);
    }

    refresh_screen();

    display.hibernate();
}

char* get_line_content(const char *str, uint16_t start_pos) {
    uint16_t str_len = strlen(str);
    uint8_t line_ch_len = screen_w / en_char_len;
    // 字符串占总屏幕宽度，默认两边间距为2；
    uint16_t all_len = 2;
    uint16_t i = start_pos;
    while (i < str_len - 3 && all_len <= (screen_w - cn_char_len)) {
        if (str[i] & 0x80) {
            i += 3;
            all_len += cn_char_len;
        } else {
            i += 1;
            all_len += en_char_len;
        }
    }

    uint8_t result_len = i - start_pos;
    if (result_len <= 0) {
        return NULL;
    }
    char *result = (char*)malloc(sizeof(char)*(result_len + 1));
    uint8_t j = 0;
    for (; j < result_len; j++) {
        if (str[start_pos+j] == '\n' || str[start_pos+j] == '\r') {
            result[j] = ' ';
        } else {
            result[j] = str[start_pos+j];
        }
    }

    result[j] = '\0';

    return result;
}

uint16_t text_multi_line_show(const char *str, bool contains_end)
{
    screen_w = display.width();
    uint16_t str_len = strlen(str);
    // 每一行结尾在str的下标
    uint16_t get_pos = 0;

    uint16_t init_height = 15;

    uint8_t lines_num = 0;

    uint8_t line_gap = 2;

    screen_draw_white();
    display.setPartialWindow(0, 0, display.width(), display.height());
    display.firstPage();

    // 获取到全部内容或者显示高度超出屏幕退出
    while (get_pos < str_len && init_height <= display.height()) {
        char *line;
        if (contains_end) {
            line = get_line_content_contains_end(str, get_pos);
        } else {
            line = get_line_content(str, get_pos);
        }
        if (!line) break;
        u8g2Fonts.setCursor(10, init_height);
        u8g2Fonts.println(line);
        init_height += cn_char_len + line_gap;
        get_pos += strlen(line);
        lines_num++;
        free(line);
    };

    refresh_screen();
    display.hibernate();
    return get_pos;
}