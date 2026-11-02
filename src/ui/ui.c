#include "ui/ui.h"
#include "i18n/i18n.h"
#include "platform/platform.h"
#include "core/util.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BOX_W 64                    /* inner width of boxes */

static int use_color = 0;

void ui_init(void)
{
    use_color = mw_stdout_is_tty() && !getenv("NO_COLOR");
}

void ui_clear(void)          { mw_clear(); }
void ui_style(const char *s) { if (use_color && s) fputs(s, stdout); }
void ui_reset(void)          { if (use_color) fputs(UI_RESET, stdout); }

static void vprint(const char *style, const char *fmt, va_list ap)
{
    ui_style(style);
    vprintf(fmt, ap);
    ui_reset();
}

void ui_print(const char *style, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprint(style, fmt, ap);
    va_end(ap);
}

/* ---- boxes -------------------------------------------------------- */

static void rep(const char *s, int n) { while (n-- > 0) fputs(s, stdout); }

void ui_box_top(void)
{
    ui_style(UI_CYAN);
    fputs("╔", stdout); rep("═", BOX_W); fputs("╗\n", stdout);
    ui_reset();
}

void ui_box_sep(void)
{
    ui_style(UI_CYAN);
    fputs("╟", stdout); rep("─", BOX_W); fputs("╢\n", stdout);
    ui_reset();
}

void ui_box_bottom(void)
{
    ui_style(UI_CYAN);
    fputs("╚", stdout); rep("═", BOX_W); fputs("╝\n", stdout);
    ui_reset();
}

/* Cuts buf (in place) so it fits `cols` terminal columns, adding an ellipsis. */
static void fit(char *buf, size_t cols)
{
    if (mw_utf8_width(buf) <= cols) return;
    buf[mw_utf8_prefix(buf, cols - 1)] = '\0';
    strcat(buf, "…");
}

void ui_box_line(const char *style, const char *fmt, ...)
{
    char buf[1024];
    va_list ap;
    int pad;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    fit(buf, BOX_W - 2);

    pad = BOX_W - 2 - (int)mw_utf8_width(buf);
    ui_style(UI_CYAN); fputs("║", stdout); ui_reset();
    fputs(" ", stdout);
    ui_style(style); fputs(buf, stdout); ui_reset();
    rep(" ", pad > 0 ? pad : 0);
    fputs(" ", stdout);
    ui_style(UI_CYAN); fputs("║\n", stdout); ui_reset();
}

void ui_menu_item(char key, const char *label)
{
    char buf[1024];
    int pad;
    snprintf(buf, sizeof buf, "%s", label);
    fit(buf, BOX_W - 10);                         /* 4 indent + [x] + 2 gaps + 1 margin */
    pad = BOX_W - 10 - (int)mw_utf8_width(buf);
    ui_style(UI_CYAN); fputs("║", stdout); ui_reset();
    fputs("    ", stdout);
    ui_style(UI_BOLD UI_YELLOW); printf("[%c]", key); ui_reset();
    printf("  %s", buf);
    rep(" ", pad > 0 ? pad : 0);
    fputs(" ", stdout);
    ui_style(UI_CYAN); fputs("║\n", stdout); ui_reset();
}

void ui_banner(void)
{
    ui_clear();
    ui_box_top();
    ui_box_line(UI_BOLD UI_WHITE, "  m i c r o W o r d");
    ui_box_line(UI_DIM, "  %s", tr("app.tagline"));
    ui_box_line(UI_DIM, tr("app.credit"), MW_OS_NAME);
    ui_box_bottom();
}

/* ---- messages ----------------------------------------------------- */

#define MSG(name, style, icon)                         \
void name(const char *fmt, ...)                        \
{                                                      \
    va_list ap;                                        \
    ui_style(style);                                   \
    fputs("  " icon " ", stdout);                      \
    va_start(ap, fmt);                                 \
    vprintf(fmt, ap);                                  \
    va_end(ap);                                        \
    ui_reset();                                        \
    fputc('\n', stdout);                               \
}

MSG(ui_ok,   UI_GREEN,  "✔")
MSG(ui_err,  UI_RED,    "✘")
MSG(ui_warn, UI_YELLOW, "!")
MSG(ui_info, UI_CYAN,   "•")

/* ---- input -------------------------------------------------------- */

int ui_readline(char *buf, size_t n)
{
    size_t len;
    if (!fgets(buf, (int)n, stdin)) { buf[0] = '\0'; return 0; }
    len = strlen(buf);
    if (len && buf[len - 1] == '\n') {
        buf[--len] = '\0';
    } else {
        int c;                                  /* line too long: drain it */
        while ((c = getchar()) != '\n' && c != EOF) { }
    }
    if (len && buf[len - 1] == '\r') buf[len - 1] = '\0';
    return 1;
}

int ui_prompt(const char *label, char *buf, size_t n)
{
    ui_style(UI_BOLD UI_YELLOW);
    printf("\n  %s ❯ ", label);
    ui_reset();
    fflush(stdout);
    return ui_readline(buf, n);
}

int ui_confirm(const char *question)
{
    char b[16];
    ui_style(UI_BOLD UI_YELLOW);
    printf("\n  %s %s ❯ ", question, tr("confirm.hint"));
    ui_reset();
    fflush(stdout);
    if (!ui_readline(b, sizeof b) || !b[0]) return 0;
    return strchr(tr("confirm.yes"), b[0]) != NULL;
}

void ui_pause(void)
{
    char b[16];
    ui_print(UI_DIM, "\n  %s", tr("ui.press_enter"));
    fflush(stdout);
    ui_readline(b, sizeof b);
}
