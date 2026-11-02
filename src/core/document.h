#ifndef MW_DOCUMENT_H
#define MW_DOCUMENT_H

#include <stddef.h>

typedef enum { MW_NORMAL, MW_H1, MW_H2, MW_BOLD, MW_ITALIC } MwStyle;

typedef struct { char *text; MwStyle style; } MwPara;

typedef struct { MwPara *items; size_t count, cap; } MwDoc;

void mwdoc_init(MwDoc *d);
void mwdoc_free(MwDoc *d);
void mwdoc_clear(MwDoc *d);
int  mwdoc_insert(MwDoc *d, size_t at, const char *text, MwStyle st); /* 0 ok */
int  mwdoc_add(MwDoc *d, const char *text, MwStyle st);
int  mwdoc_remove(MwDoc *d, size_t at);
int  mwdoc_set_text(MwDoc *d, size_t at, const char *text);

/* Translation key of a style's display name ("style.h1", ...). */
const char *mwstyle_key(MwStyle s);

/* Markdown-ish shorthand typed by the user:
 *   "# Title" -> H1, "## Sub" -> H2, "**x**" -> bold, "*x*" -> italic.
 * Modifies `line` in place; returns pointer to the text and sets *st.
 * Returns NULL (st untouched) when no marker was found. */
char *mwdoc_parse_markup(char *line, MwStyle *st);

#endif
