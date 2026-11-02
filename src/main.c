#include "core/document.h"
#include "core/paths.h"
#include "core/util.h"
#include "formats/format.h"
#include "i18n/i18n.h"
#include "platform/platform.h"
#include "ui/editor.h"
#include "ui/ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_LEN 600
#define NAME_LEN 256

/* ---- helpers ------------------------------------------------------ */

static void screen(const char *title)
{
    ui_banner();
    ui_box_top();
    ui_box_line(UI_BOLD UI_WHITE, "  %s", title);
    ui_box_bottom();
}

static int ensure_docs_dir(void)
{
    char dir[PATH_LEN];
    mw_path(dir, sizeof dir, MW_DOCS_DIR, NULL);
    if (mw_mkdir(dir) != 0) { ui_err(tr("err.docs_dir"), dir); return 0; }
    return 1;
}

/* Asks for a document name (never a folder: files live in MW_DOCS_DIR).
 * If it has no known extension: with `choose_fmt` asks which format to use,
 * otherwise looks for an existing .odt/.docx. */
static int ask_path(char *path, size_t pn, char *name, size_t nn, int choose_fmt)
{
    char b[PATH_LEN];
    char *in;

    if (!ui_prompt(tr("prompt.docname"), b, sizeof b)) return 0;
    in = mw_trim(b);
    if (!*in) { ui_err("%s", tr("err.name_empty")); return 0; }
    if (!mw_valid_docname(in)) { ui_err("%s", tr("err.name_chars")); return 0; }
    if (strlen(in) + 6 >= nn) { ui_err("%s", tr("err.name_long")); return 0; }

    if (mw_format_for_path(in)) {
        snprintf(name, nn, "%s", in);
    } else if (choose_fmt) {
        char f[16];
        ui_info("%s", tr("info.format_choice"));
        if (!ui_prompt(tr("prompt.format"), f, sizeof f)) return 0;
        snprintf(name, nn, "%s%s", in, f[0] == '2' ? ".docx" : ".odt");
    } else {
        snprintf(name, nn, "%s.odt", in);
        mw_path(path, pn, MW_DOCS_DIR, name);
        if (!mw_file_exists(path)) {
            snprintf(name, nn, "%s.docx", in);
            mw_path(path, pn, MW_DOCS_DIR, name);
            if (!mw_file_exists(path)) snprintf(name, nn, "%s", in);   /* caller reports "not found" */
        }
    }
    mw_path(path, pn, MW_DOCS_DIR, name);
    return 1;
}

/* ---- actions ------------------------------------------------------ */

static void act_create(void)
{
    char path[PATH_LEN], name[NAME_LEN];
    MwDoc doc;

    screen(tr("title.create"));
    if (!ensure_docs_dir()) { ui_pause(); return; }
    if (!ask_path(path, sizeof path, name, sizeof name, 1)) { ui_pause(); return; }
    if (mw_file_exists(path)) { ui_err(tr("err.exists"), name); ui_pause(); return; }

    mwdoc_init(&doc);
    editor_run(&doc, path, 0);
    mwdoc_free(&doc);
}

static void act_open(void)
{
    char path[PATH_LEN], name[NAME_LEN], err[512];
    MwDoc doc;

    screen(tr("title.open"));
    if (!ask_path(path, sizeof path, name, sizeof name, 0)) { ui_pause(); return; }
    if (!mw_file_exists(path)) { ui_err(tr("err.not_found"), name); ui_pause(); return; }

    mwdoc_init(&doc);
    if (mw_load(&doc, path, err, sizeof err) != 0) {
        ui_err("%s", err);
        ui_pause();
    } else {
        editor_run(&doc, path, 0);
    }
    mwdoc_free(&doc);
}

static void act_delete(void)
{
    char path[PATH_LEN], name[NAME_LEN];

    screen(tr("title.delete"));
    if (!ask_path(path, sizeof path, name, sizeof name, 0)) { ui_pause(); return; }
    if (!mw_file_exists(path)) { ui_err(tr("err.not_found"), name); ui_pause(); return; }
    if (!ui_confirm(tr("confirm.delete"))) { ui_info("%s", tr("info.cancelled")); ui_pause(); return; }

    if (remove(path) == 0) ui_ok(tr("ok.deleted"), name);
    else { ui_err(tr("err.delete_failed"), name); perror(tr("err.reason")); }
    ui_pause();
}

static void act_convert(void)
{
    char path[PATH_LEN], name[NAME_LEN], opath[PATH_LEN + 8], oname[NAME_LEN + 8], err[512] = "";
    const MwFormatOps *src, *dst;
    char *dot;
    MwDoc doc;

    screen(tr("title.convert"));
    if (!ask_path(path, sizeof path, name, sizeof name, 0)) { ui_pause(); return; }
    src = mw_format_for_path(name);
    if (!src || !mw_file_exists(path)) { ui_err(tr("err.not_found_type"), name); ui_pause(); return; }

    dst = (src == &mw_odt_ops) ? &mw_docx_ops : &mw_odt_ops;
    snprintf(oname, sizeof oname, "%s", name);
    dot = strrchr(oname, '.');
    strcpy(dot, dst->ext);
    mw_path(opath, sizeof opath, MW_DOCS_DIR, oname);
    if (mw_file_exists(opath) && !ui_confirm(tr("confirm.overwrite"))) {
        ui_info("%s", tr("info.cancelled")); ui_pause(); return;
    }

    mwdoc_init(&doc);
    if (mw_load(&doc, path, err, sizeof err) == 0 && mw_save(&doc, opath, err, sizeof err) == 0)
        ui_ok(tr("ok.converted"), name, oname);
    else
        ui_err("%s", err);
    mwdoc_free(&doc);
    ui_pause();
}

static void act_language(const char *lang_dir)
{
    char b[16], label[128];
    int i, n = i18n_count(), sel;

    screen(tr("title.language"));
    ui_box_top();
    for (i = 0; i < n && i < 9; i++) {
        snprintf(label, sizeof label, "%s%s", i18n_name_at(i), i == i18n_current() ? "  ●" : "");
        ui_menu_item((char)('1' + i), label);
    }
    ui_box_bottom();
    if (!ui_prompt(tr("prompt.language"), b, sizeof b)) return;
    sel = b[0] - '1';
    if (sel < 0 || sel >= n || sel >= 9) { ui_err("%s", tr("err.invalid_option")); ui_pause(); return; }
    i18n_select(i18n_id_at(sel));
    i18n_save_pref(lang_dir);
    ui_ok(tr("lang.changed"), i18n_name_at(sel));
    ui_pause();
}

/* ---- main --------------------------------------------------------- */

int main(int argc, char **argv)
{
    char opt[16], lang_dir[PATH_LEN];

    mw_platform_init();
    mw_paths_init(argc > 0 ? argv[0] : NULL);

    mw_path(lang_dir, sizeof lang_dir, MW_LANG_DIR, NULL);
    if (i18n_init(lang_dir) != 0) {
        /* no language files = no translated text: this one message is fixed */
        fprintf(stderr, "microWord: language files not found in '%s'.\n"
                        "Run the program from the project folder.\n", lang_dir);
        return 1;
    }
    i18n_detect();                    /* from LANG ...                    */
    i18n_load_pref(lang_dir);         /* ... unless the user chose one    */

    ui_init();
    ensure_docs_dir();

    for (;;) {
        ui_banner();
        ui_box_top();
        ui_box_line(UI_BOLD UI_WHITE, "  %s", tr("menu.title"));
        ui_box_sep();
        ui_menu_item('1', tr("menu.create"));
        ui_menu_item('2', tr("menu.open"));
        ui_menu_item('3', tr("menu.delete"));
        ui_menu_item('4', tr("menu.convert"));
        ui_menu_item('5', tr("menu.language"));
        ui_menu_item('0', tr("menu.exit"));
        ui_box_bottom();
        if (!mw_tools_available()) {
            fputc('\n', stdout);
            ui_warn("%s", tr(mw_tools_hint_key()));
        }

        if (!ui_prompt(tr("prompt.option"), opt, sizeof opt)) break;       /* EOF */
        switch (opt[0]) {
        case '1': act_create();  break;
        case '2': act_open();    break;
        case '3': act_delete();  break;
        case '4': act_convert(); break;
        case '5': act_language(lang_dir); break;
        case '0':
            ui_clear();
            ui_ok("%s", tr("bye"));
            return 0;
        default:
            ui_err("%s", tr("err.invalid_option"));
            ui_pause();
        }
    }
    return 0;
}
