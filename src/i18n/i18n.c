#include "i18n/i18n.h"
#include "core/paths.h"
#include "core/util.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LANGS 16

typedef struct { char *key, *val; } Entry;
typedef struct { Entry *e; size_t n, cap; } Table;
typedef struct { char id[40]; Table t; } Lang;

static Lang langs[MAX_LANGS];
static int  nlangs = 0, cur = 0;

/* ---- tables ------------------------------------------------------- */

static const char *tbl_get(const Table *t, const char *key)
{
    size_t i;
    for (i = 0; i < t->n; i++)
        if (!strcmp(t->e[i].key, key)) return t->e[i].val;
    return NULL;
}

static void tbl_put(Table *t, const char *key, const char *val)
{
    size_t i;
    for (i = 0; i < t->n; i++)
        if (!strcmp(t->e[i].key, key)) {
            free(t->e[i].val);
            t->e[i].val = mw_xstrdup(val);
            return;
        }
    if (t->n == t->cap) {
        t->cap = t->cap ? t->cap * 2 : 128;
        t->e = mw_xrealloc(t->e, t->cap * sizeof *t->e);
    }
    t->e[t->n].key = mw_xstrdup(key);
    t->e[t->n].val = mw_xstrdup(val);
    t->n++;
}

static void tbl_free(Table *t)
{
    size_t i;
    for (i = 0; i < t->n; i++) { free(t->e[i].key); free(t->e[i].val); }
    free(t->e);
    t->e = NULL;
    t->n = t->cap = 0;
}

/* ---- file parsing ------------------------------------------------- */

/* The conversions of a printf format ("%s %zu %%" -> "su"). A translation is
 * only accepted if its conversions match the English text exactly, so a typo
 * in a language file can never crash the program. */
static void specs(const char *s, char *out, size_t n)
{
    size_t k = 0;
    while (*s) {
        if (*s != '%') { s++; continue; }
        s++;
        if (*s == '%') { s++; continue; }
        while (*s && strchr("-+ #0123456789.lhzjt", *s)) s++;
        if (*s) { if (k + 1 < n) out[k++] = *s; s++; }
    }
    out[k] = '\0';
}

static void unescape(char *s)
{
    char *w = s;
    while (*s) {
        if (*s == '\\' && s[1] == 'n')       { *w++ = '\n'; s += 2; }
        else if (*s == '\\' && s[1] == 't')  { *w++ = '\t'; s += 2; }
        else if (*s == '\\' && s[1] == '\\') { *w++ = '\\'; s += 2; }
        else *w++ = *s++;
    }
    *w = '\0';
}

static int load_file(const char *path, Table *t, const Table *ref)
{
    FILE *f = fopen(path, "rb");
    char line[4096];
    int first = 1;
    if (!f) return -1;
    while (fgets(line, sizeof line, f)) {
        char *p = line, *eq, *key, *val;
        size_t len = strlen(line);
        if (len && line[len - 1] != '\n' && !feof(f)) {         /* over-long line */
            int c;
            while ((c = fgetc(f)) != '\n' && c != EOF) { }
        }
        while (len && (line[len - 1] == '\n' || line[len - 1] == '\r')) line[--len] = '\0';
        if (first && !strncmp(p, "\xEF\xBB\xBF", 3)) p += 3;   /* UTF-8 BOM */
        first = 0;
        if (!*p || *p == '#') continue;
        eq = strchr(p, '=');
        if (!eq) continue;
        *eq = '\0';
        key = mw_trim(p);
        val = eq + 1;
        unescape(val);
        if (ref) {
            char a[16], b[16];
            const char *rv = tbl_get(ref, key);
            if (!rv) continue;                                   /* unknown key */
            specs(rv, a, sizeof a);
            specs(val, b, sizeof b);
            if (strcmp(a, b)) continue;                          /* format mismatch */
        }
        tbl_put(t, key, val);
    }
    fclose(f);
    return 0;
}

static int id_ok(const char *id)
{
    if (!*id || strlen(id) >= sizeof langs[0].id) return 0;
    for (; *id; id++)
        if (!isalnum((unsigned char)*id) && *id != '_' && *id != '-') return 0;
    return 1;
}

static void add_lang(const char *dir, const char *id)
{
    char path[700];
    int i;
    if (nlangs >= MAX_LANGS || !id_ok(id)) return;
    for (i = 0; i < nlangs; i++) if (!strcmp(langs[i].id, id)) return;
    snprintf(path, sizeof path, "%s/%s.txt", dir, id);
    memset(&langs[nlangs], 0, sizeof langs[nlangs]);
    snprintf(langs[nlangs].id, sizeof langs[nlangs].id, "%s", id);
    if (load_file(path, &langs[nlangs].t, nlangs ? &langs[0].t : NULL) == 0) nlangs++;
    else tbl_free(&langs[nlangs].t);
}

/* ---- public API --------------------------------------------------- */

int i18n_init(const char *dir)
{
    static const char *const defaults[] = { "spanish", "japanese", "german", NULL };
    char path[700], line[128];
    FILE *f;

    add_lang(dir, "english");                    /* must be first: it is the reference */
    if (nlangs == 0) return -1;

    snprintf(path, sizeof path, "%s/idiomas.txt", dir);
    f = fopen(path, "rb");
    if (f) {
        while (fgets(line, sizeof line, f)) {
            char *id = mw_trim(line);
            if (*id && *id != '#') add_lang(dir, id);
        }
        fclose(f);
    } else {
        int i;
        for (i = 0; defaults[i]; i++) add_lang(dir, defaults[i]);
    }
    cur = 0;
    return 0;
}

const char *tr(const char *key)
{
    const char *v = tbl_get(&langs[cur].t, key);
    if (!v && cur != 0) v = tbl_get(&langs[0].t, key);
    return v ? v : key;
}

void i18n_fmt(char *out, size_t n, const char *key, ...)
{
    va_list ap;
    va_start(ap, key);
    vsnprintf(out, n, tr(key), ap);
    va_end(ap);
}

int         i18n_count(void)       { return nlangs; }
int         i18n_current(void)     { return cur; }
const char *i18n_id_at(int i)      { return (i >= 0 && i < nlangs) ? langs[i].id : ""; }

const char *i18n_name_at(int i)
{
    const char *n;
    if (i < 0 || i >= nlangs) return "";
    n = tbl_get(&langs[i].t, "lang.name");
    return n ? n : langs[i].id;
}

int i18n_select(const char *id)
{
    int i;
    for (i = 0; i < nlangs; i++)
        if (!strcmp(langs[i].id, id)) { cur = i; return 0; }
    return -1;
}

void i18n_detect(void)
{
    static const char *const map[][2] = {
        { "es", "spanish" }, { "ja", "japanese" }, { "de", "german" }, { "en", "english" }
    };
    const char *e = getenv("LC_ALL");
    size_t i;
    if (!e || !*e) e = getenv("LC_MESSAGES");
    if (!e || !*e) e = getenv("LANG");
    if (!e || strlen(e) < 2) return;
    for (i = 0; i < sizeof map / sizeof map[0]; i++)
        if (!strncmp(e, map[i][0], 2)) { i18n_select(map[i][1]); return; }
}

int i18n_load_pref(const char *dir)
{
    char path[700], line[128];
    FILE *f;
    int r = -1;
    snprintf(path, sizeof path, "%s/%s", dir, MW_LANG_PREF);
    f = fopen(path, "rb");
    if (!f) return -1;
    while (fgets(line, sizeof line, f))
        if (!strncmp(line, "language=", 9)) { r = i18n_select(mw_trim(line + 9)); break; }
    fclose(f);
    return r;
}

int i18n_save_pref(const char *dir)
{
    char path[700];
    FILE *f;
    snprintf(path, sizeof path, "%s/%s", dir, MW_LANG_PREF);
    f = fopen(path, "wb");
    if (!f) return -1;
    fprintf(f, "language=%s\n", langs[cur].id);
    return fclose(f) == 0 ? 0 : -1;
}
