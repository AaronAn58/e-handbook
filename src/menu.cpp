#include "menu.h"


void display();
void enter_menu(void);
void next_menu(void);
void next_book(void);
void no_op(void);
void no_op_str(String param);
void back_menu(void);
void enter_book(void);
void display_book_op_menu();
void init_menu_scroll();
void twice_back_menu();
void twice_enter_menu();
void file_recv_op();
void next_page_read(long pos, const char* file_name);
void last_page_read(long pos, const char* file_name);
void last_page_read_content_and_pos(long pos, const char* file_name);
void rec_file_exit();
void del_op_back_menu();
void del_book_reset_filelist();
void reset_books_file();
void book_list_not_empty_enter();

MenuAction *home;
MenuAction *curr_m;

ScrollMenuArr scroll_menu = {
    .max_line_num = max_menus_line,
    .all_len = -1,
    .data_start_pos = 0,
    .curr_pos = 0,
    .topmost = 0,
    .lowest = -1
};

// 初始化成员变量
// home
MenuAction m_home = {
    .name = "Home",
    .next_menus = NULL,
    .next_menus_len = 3,
    .last_lev_menus = NULL,
    .last_menus_len = 0,
    .param_val = "",
    .display = &display,
    .enter_call = &book_list_not_empty_enter,
    .choose_call = &next_menu,
    .back_call = &no_op,
    .level = 0
};

MenuAction m_book_list = {
    "书籍列表",
    .next_menus = NULL,
    .next_menus_len = 1,
    .last_lev_menus = NULL,
    .last_menus_len = 1,
    .param_val = "",
    .display = &display_books,
    .enter_call = &enter_menu,
    .back_call = &back_menu,
    .choose_call = &next_book,
    .level = 1,
};

MenuAction m_book_op = {
    .name = "阅读功能",
    .next_menus = NULL,
    .next_menus_len = 2,
    .last_lev_menus = NULL,
    .last_menus_len = 3,
    .param_val = "",
    .display = &display_book_op_menu,
    .enter_call = &enter_menu,
    .choose_call = &next_menu,
    .back_call = &back_menu,
    .level = 2
};

MenuAction m_read = {
    .name = "阅读",
    .next_menus = NULL,
    .next_menus_len = 0,
    .last_lev_menus = NULL,
    .last_menus_len = 1,
    .param_val = "",
    .display = &enter_read,
    .enter_call = &next_page,
    .choose_call = &exit_read,
    .back_call = &last_page,
    .level = 3
};

MenuAction m_del = {
    .name = "删除",
    .next_menus = NULL,
    .next_menus_len = 2,
    .last_lev_menus = NULL,
    .last_menus_len = 1,
    .param_val = "",
    .display = &display_book_op_menu,
    .enter_call = &enter_menu,
    .choose_call = &next_menu,
    .back_call = &twice_back_menu,
    .level = 3
};

MenuAction m_confirm = {
    .name = "确认",
    .next_menus = NULL,
    .next_menus_len = 0,
    .last_lev_menus = NULL,
    .last_menus_len = 2,
    .param_val = "",
    .display = &del_book_reset_filelist,
    .enter_call = NULL,
    .choose_call = NULL,
    .back_call = &del_op_back_menu,
    .level = 3
};

MenuAction m_cancel = {
    .name = "取消",
    .next_menus = NULL,
    .next_menus_len = 0,
    .last_lev_menus = NULL,
    .last_menus_len = 2,
    .param_val = "",
    .display = &del_op_back_menu,
    .enter_call = NULL,
    .choose_call = NULL,
    .back_call = NULL,
    .level = 3
};

MenuAction *home_m_arr[] = {&m_home};
MenuAction *first_lev_menu[] = {&m_book_list};
MenuAction *book_next_ms[] = {&m_book_op};
MenuAction *book_op_next_ms[] = {&m_read, &m_del};
MenuAction *del_next_ms[] = {&m_confirm, &m_cancel};


void init_menu() {
    menu();
    init_menu_scroll();
    home->display();
}

/**
 * 菜单滚动条初始化
 */
void init_menu_scroll() {
    if (curr_m->next_menus != NULL) {
        scroll_menu.all_len = curr_m->next_menus_len;
        if (scroll_menu.all_len < scroll_menu.max_line_num) {
            scroll_menu.lowest = scroll_menu.all_len - 1;
        } else {
            scroll_menu.lowest = scroll_menu.max_line_num - 1;
        }
    } else {
        scroll_menu.all_len = 0;
    }
}

/**
 * 菜单关系绑定
 */
void menu() {
    // home 设置
    home = &m_home;
    // 当前菜单
    curr_m = home;

    m_home.next_menus = first_lev_menu;

    m_book_list.last_lev_menus = home_m_arr;

    m_book_list.next_menus = book_next_ms;

    m_book_op.last_lev_menus = first_lev_menu;

    m_book_op.next_menus = book_op_next_ms;

    m_del.last_lev_menus = book_next_ms;
    m_read.last_lev_menus = book_next_ms;

    m_del.next_menus = del_next_ms;

    m_confirm.last_lev_menus = book_op_next_ms;
    m_cancel.last_lev_menus = book_op_next_ms;
}

void doAction(int action) {
    switch (action) {
        case MID_ACTION:
            Serial.println("MID_ACTION");
            if (curr_m->choose_call) {
                curr_m->choose_call();
            }
            break;
        case LEFT_ACTION:
            Serial.println("LEFT_ACTION");
            if (curr_m->back_call) {
                curr_m->back_call();
            }
            break;
        case RIGHT_ACTION:
            
            break;
    }
}

char* menu_content_collect(const char *show_arr[], int len) {
    char *curr_str = NULL;
    for (int i = scroll_menu.topmost; i < arr_len; i++) {
        if (menu_pos == i) {
            curr_str = malloc_and_concat("> ", curr_m->next_menus[i] -> name.c_str(), NULL);
            show_arr[i] = curr_str;
        } else {
            show_arr[i] = curr_m->next_menus[i] -> name.c_str();
        }
        Serial.println(show_arr[i]);
    }
    return curr_str;
}

void display() {
    if (!curr_m->display || !curr_m->next_menus) {
        return;
    }

    int len = scroll_menu.all_len < scroll_menu.max_line_num ? scroll_menu.all_len : scroll_menu.max_line_num;
    const char *show_arr[scroll_menu.max_line_num] = {};
    char *curr_str = menu_content_collect(show_arr, len);
    multi_line_menu_show(show_arr, len - scroll_menu.topmost);
    free(curr_str);
}


void twice_enter_menu() {
    if (!curr_m->next_menus) {
        return;
    }

    menu_pos = 0;
    curr_m = (curr_m->next_menus)[menu_pos];
    enter_menu()
}

void enter_menu() {
    if (!curr_m->next_menus) {
        return;
    }

    curr_m = (curr_m->next_menus)[menu_pos];

    menu_pos = 0;

    init_menu_scroll();

    curr_m->display();
}

void display_books() {

}