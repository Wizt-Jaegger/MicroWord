#ifndef MW_UI_H
#define MW_UI_H

#include <stddef.h>

/* ANSI styles (only emitted when color is enabled) */
#define UI_RESET   "\x1b[0m"
#define UI_BOLD    "\x1b[1m"
#define UI_DIM     "\x1b[2m"
#define UI_ITALIC  "\x1b[3m"
#define UI_RED     "\x1b[91m"
#define UI_GREEN   "\x1b[92m"
#define UI_YELLOW  "\x1b[93m"
#define UI_BLUE    "\x1b[94m"
#define UI_MAGENTA "\x1b[95m"
#define UI_CYAN    "\x1b[96m"
#define UI_WHITE   "\x1b[97m"

void ui_init(void);                 /* detects color support (NO_COLOR, tty) */
void ui_clear(void);

/* styled output: ui_print(UI_BOLD UI_CYAN, "hola %s", name) */
void ui_print(const char *style, const char *fmt, ...);
void ui_style(const char *style);   /* start a style (no-op without color)   */
void ui_reset(void);

void ui_banner(void);               /* clears screen + logo                   */
void ui_box_top(void);
void ui_box_sep(void);
void ui_box_bottom(void);
void ui_box_line(const char *style, const char *fmt, ...);
void ui_menu_item(char key, const char *label);   /* inside a box            */

void ui_ok(const char *fmt, ...);
void ui_err(const char *fmt, ...);
void ui_warn(const char *fmt, ...);
void ui_info(const char *fmt, ...);

/* Reads one line (UTF-8), strips the newline, drains over-long input.
 * Returns 0 on EOF. */
int  ui_readline(char *buf, size_t n);
int  ui_prompt(const char *label, char *buf, size_t n);
int  ui_confirm(const char *question);            /* default: no (letters come from the language file) */
void ui_pause(void);

#endif
