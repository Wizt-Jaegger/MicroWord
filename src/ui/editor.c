#include "ui/editor.h"
#include "formats/format.h"
#include "i18n/i18n.h"
#include "ui/ui.h"
#include "core/util.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX_LEN 8192
#define PREVIEW_ROWS 12
#define PREVIEW_COLS 52

static char flash[512];
static const char *flash_style = UI_DIM;

/* Status line shown on the next redraw: say(style, icon, key, args...) */
static void say(const char *style, const char *icon, const char *key, ...)
{
    char tmp[400];
    va_list ap;
    va_start(ap, key);
    vsnprintf(tmp, sizeof tmp, tr(key), ap);
    va_end(ap);
    snprintf(flash, sizeof flash, "%s %s", icon, tmp);
    flash_style = style;
}

/* Command letters come from the language file (key.add, key.save, ...). */
static char key_of(const char *key) { return (char)toupper((unsigned char)tr(key)[0]); }
static int  is_key(char c, const char *key) { return (char)toupper((unsigned char)c) == key_of(key); }

static const char *style_tag(MwStyle s)
{
    switch (s) {
    case MW_H1:     return "H1";
    case MW_H2:     return "H2";
    case MW_BOLD:   return " B";
    case MW_ITALIC: return " I";
    default:        return "  ";
    }
}

static const char *style_color(MwStyle s)
{
    switch (s) {
    case MW_H1:     return UI_BOLD UI_YELLOW;
    case MW_H2:     return UI_YELLOW;
    case MW_BOLD:   return UI_BOLD UI_WHITE;
    case MW_ITALIC: return UI_ITALIC UI_MAGENTA;
    default:        return UI_WHITE;
    }
}

static void print_para(size_t i, const MwPara *p, size_t maxcols)
{
    size_t cut = maxcols ? mw_utf8_prefix(p->text, maxcols) : strlen(p->text);
    ui_print(UI_DIM, "  %3zu ", i + 1);
    ui_print(UI_CYAN, "│ ");
    ui_print(UI_DIM, "%s ", style_tag(p->style));
    ui_style(style_color(p->style));
    printf("%.*s", (int)cut, p->text);
    if (p->text[cut]) fputs("…", stdout);
    ui_reset();
    fputc('\n', stdout);
}

static void draw(const MwDoc *d, const char *path, int modified)
{
    size_t i, start = d->count > PREVIEW_ROWS ? d->count - PREVIEW_ROWS : 0;
    const MwFormatOps *ops = mw_format_for_path(path);
    const char *name = strrchr(path, '/');
    name = name ? name + 1 : path;

    ui_clear();
    ui_box_top();
    ui_box_line(UI_BOLD UI_WHITE, tr("ed.title"), name);
    ui_box_line(UI_DIM, "  %s%s", ops ? ops->name : "?", modified ? "  ●" : "");
    ui_box_line(UI_DIM, tr("ed.info"), d->count);
    ui_box_sep();
    ui_menu_item(key_of("key.add"),    tr("ed.m.add"));
    ui_menu_item(key_of("key.insert"), tr("ed.m.insert"));
    ui_menu_item(key_of("key.edit"),   tr("ed.m.edit"));
    ui_menu_item(key_of("key.delete"), tr("ed.m.delete"));
    ui_menu_item(key_of("key.type"),   tr("ed.m.type"));
    ui_menu_item(key_of("key.view"),   tr("ed.m.view"));
    ui_menu_item(key_of("key.save"),   tr("ed.m.save"));
    ui_menu_item(key_of("key.quit"),   tr("ed.m.quit"));
    ui_box_bottom();

    fputc('\n', stdout);
    if (d->count == 0) {
        ui_print(UI_DIM, "  %s\n", tr("ed.empty"));
    } else {
        if (start) {
            ui_style(UI_DIM);
            printf("  ");
            printf(tr("ed.more_before"), start, key_of("key.view"));
            ui_reset();
            fputc('\n', stdout);
        }
        for (i = start; i < d->count; i++) print_para(i, &d->items[i], PREVIEW_COLS);
    }
    fputc('\n', stdout);
    if (flash[0]) { ui_print(flash_style, "  %s\n", flash); flash[0] = '\0'; }
}

/* Parses an optional paragraph number after the command letter. */
static int get_index(const char *arg, const MwDoc *d, int allow_end, size_t *out)
{
    char buf[32];
    char *end;
    long n;
    if (!*arg) {
        if (!ui_prompt(tr("prompt.paragraph"), buf, sizeof buf)) return 0;
        arg = mw_trim(buf);
    }
    n = strtol(arg, &end, 10);
    if (*end || n < 1 || (size_t)n > d->count + (allow_end ? 1 : 0)) {
        say(UI_RED, "✘", "ed.err_index", d->count + (allow_end ? 1 : 0));
        return 0;
    }
    *out = (size_t)n - 1;
    return 1;
}

static void add_mode(MwDoc *d, size_t at, int *modified)
{
    char *line = mw_xmalloc(LINE_MAX_LEN);
    size_t added = 0;

    ui_clear();
    ui_box_top();
    ui_box_line(UI_BOLD UI_WHITE, "  %s", tr("add.title"));
    ui_box_sep();
    ui_box_line(UI_DIM, "  %s", tr("add.hint1"));
    ui_box_line(UI_DIM, "  %s", tr("add.hint2"));
    ui_box_line(UI_GREEN, tr("add.hint3"), tr("end.word"));
    ui_box_bottom();
    fputc('\n', stdout);

    for (;;) {
        MwStyle st = MW_NORMAL;
        char *text;
        ui_print(UI_DIM, "  %3zu ", at + 1);
        ui_print(UI_CYAN, "│ ");
        fflush(stdout);
        if (!ui_readline(line, LINE_MAX_LEN)) break;
        if (mw_ieq(line, tr("end.word"))) break;
        text = mwdoc_parse_markup(line, &st);
        if (!text) text = line;
        mwdoc_insert(d, at++, text, st);
        added++;
    }
    free(line);
    if (added) *modified = 1;
    say(UI_GREEN, "✔", "add.done", added);
}

static void edit_para(MwDoc *d, size_t i, int *modified)
{
    char *line = mw_xmalloc(LINE_MAX_LEN);
    MwStyle st = d->items[i].style;

    ui_clear();
    ui_box_top();
    ui_box_line(UI_BOLD UI_WHITE, tr("edit.title"), i + 1, tr(mwstyle_key(st)));
    ui_box_bottom();
    fputc('\n', stdout);
    ui_print(UI_DIM, "  %s\n", tr("edit.current"));
    print_para(i, &d->items[i], 0);
    ui_print(UI_DIM, "\n  %s", tr("edit.hint"));

    if (ui_prompt(tr("prompt.text"), line, LINE_MAX_LEN) && line[0]) {
        MwStyle n = st;
        char *text = mwdoc_parse_markup(line, &n);
        mwdoc_set_text(d, i, text ? text : line);
        if (text) d->items[i].style = n;
        *modified = 1;
        say(UI_GREEN, "✔", "edit.updated", i + 1);
    } else {
        say(UI_DIM, "•", "info.unchanged");
    }
    free(line);
}

static void change_style(MwDoc *d, size_t i, int *modified)
{
    char b[16];
    static const MwStyle map[] = { MW_NORMAL, MW_H1, MW_H2, MW_BOLD, MW_ITALIC };
    int k;
    ui_clear();
    ui_box_top();
    ui_box_line(UI_BOLD UI_WHITE, tr("style.title"), i + 1);
    ui_box_sep();
    for (k = 0; k < 5; k++) ui_menu_item((char)('1' + k), tr(mwstyle_key(map[k])));
    ui_box_bottom();
    if (ui_prompt(tr("prompt.type"), b, sizeof b) && b[0] >= '1' && b[0] <= '5') {
        d->items[i].style = map[b[0] - '1'];
        *modified = 1;
        say(UI_GREEN, "✔", "style.changed", i + 1, tr(mwstyle_key(d->items[i].style)));
    } else say(UI_DIM, "•", "info.unchanged");
}

static void view_all(const MwDoc *d)
{
    size_t i;
    ui_clear();
    ui_box_top();
    ui_box_line(UI_BOLD UI_WHITE, tr("view.title"), d->count);
    ui_box_bottom();
    fputc('\n', stdout);
    for (i = 0; i < d->count; i++) print_para(i, &d->items[i], 0);
    ui_pause();
}

static int do_save(const MwDoc *d, const char *path, int *modified)
{
    char err[512] = "";
    const char *name = strrchr(path, '/');
    if (mw_save(d, path, err, sizeof err) == 0) {
        *modified = 0;
        say(UI_GREEN, "✔", "save.ok", name ? name + 1 : path);
        return 1;
    }
    snprintf(flash, sizeof flash, "✘ %s", err);
    flash_style = UI_RED;
    return 0;
}

void editor_run(MwDoc *doc, const char *path, int modified)
{
    char cmd[64];
    flash[0] = '\0';

    for (;;) {
        char *arg;
        size_t idx;
        char c;

        draw(doc, path, modified);
        if (!ui_prompt(tr("prompt.command"), cmd, sizeof cmd)) cmd[0] = key_of("key.quit");  /* EOF */
        arg = mw_trim(cmd);
        c = *arg;
        arg = mw_trim(arg + (c ? 1 : 0));

        if (is_key(c, "key.add")) {
            add_mode(doc, doc->count, &modified);
        } else if (is_key(c, "key.insert")) {
            if (get_index(arg, doc, 1, &idx)) add_mode(doc, idx, &modified);
        } else if (is_key(c, "key.edit")) {
            if (get_index(arg, doc, 0, &idx)) edit_para(doc, idx, &modified);
        } else if (is_key(c, "key.delete")) {
            if (get_index(arg, doc, 0, &idx)) {
                mwdoc_remove(doc, idx);
                modified = 1;
                say(UI_GREEN, "✔", "del.done", idx + 1);
            }
        } else if (is_key(c, "key.type")) {
            if (get_index(arg, doc, 0, &idx)) change_style(doc, idx, &modified);
        } else if (is_key(c, "key.view")) {
            view_all(doc);
        } else if (is_key(c, "key.save")) {
            do_save(doc, path, &modified);
        } else if (is_key(c, "key.quit")) {
            if (modified) {
                char a[16];
                if (!ui_prompt(tr("exit.unsaved"), a, sizeof a)) a[0] = tr("confirm.no")[0];
                if (a[0] && strchr(tr("confirm.yes"), a[0])) {
                    if (!do_save(doc, path, &modified)) continue;     /* stay: show error */
                } else if (!(a[0] && strchr(tr("confirm.no"), a[0]))) {
                    continue;                                         /* cancel */
                }
            }
            return;
        } else {
            say(UI_RED, "✘", "ed.err_unknown");
        }
    }
}
