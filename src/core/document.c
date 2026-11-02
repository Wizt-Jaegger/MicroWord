#include "core/document.h"
#include "core/util.h"

#include <stdlib.h>
#include <string.h>

void mwdoc_init(MwDoc *d) { d->items = NULL; d->count = d->cap = 0; }

void mwdoc_clear(MwDoc *d)
{
    size_t i;
    for (i = 0; i < d->count; i++) free(d->items[i].text);
    d->count = 0;
}

void mwdoc_free(MwDoc *d)
{
    mwdoc_clear(d);
    free(d->items);
    mwdoc_init(d);
}

int mwdoc_insert(MwDoc *d, size_t at, const char *text, MwStyle st)
{
    if (at > d->count) return -1;
    if (d->count == d->cap) {
        d->cap = d->cap ? d->cap * 2 : 16;
        d->items = mw_xrealloc(d->items, d->cap * sizeof *d->items);
    }
    memmove(&d->items[at + 1], &d->items[at], (d->count - at) * sizeof *d->items);
    d->items[at].text = mw_xstrdup(text);
    d->items[at].style = st;
    d->count++;
    return 0;
}

int mwdoc_add(MwDoc *d, const char *text, MwStyle st)
{
    return mwdoc_insert(d, d->count, text, st);
}

int mwdoc_remove(MwDoc *d, size_t at)
{
    if (at >= d->count) return -1;
    free(d->items[at].text);
    memmove(&d->items[at], &d->items[at + 1], (d->count - at - 1) * sizeof *d->items);
    d->count--;
    return 0;
}

int mwdoc_set_text(MwDoc *d, size_t at, const char *text)
{
    if (at >= d->count) return -1;
    free(d->items[at].text);
    d->items[at].text = mw_xstrdup(text);
    return 0;
}

const char *mwstyle_key(MwStyle s)
{
    switch (s) {
    case MW_H1:     return "style.h1";
    case MW_H2:     return "style.h2";
    case MW_BOLD:   return "style.bold";
    case MW_ITALIC: return "style.italic";
    default:        return "style.normal";
    }
}

char *mwdoc_parse_markup(char *line, MwStyle *st)
{
    size_t n = strlen(line);
    if (n >= 3 && !strncmp(line, "## ", 3)) { *st = MW_H2; return line + 3; }
    if (n >= 2 && !strncmp(line, "# ", 2))   { *st = MW_H1; return line + 2; }
    if (n >= 5 && !strncmp(line, "**", 2) && !strcmp(line + n - 2, "**")) {
        line[n - 2] = '\0';
        *st = MW_BOLD;
        return line + 2;
    }
    if (n >= 3 && line[0] == '*' && line[n - 1] == '*' && line[1] != '*') {
        line[n - 1] = '\0';
        *st = MW_ITALIC;
        return line + 1;
    }
    return NULL;
}
