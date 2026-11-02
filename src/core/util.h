#ifndef MW_UTIL_H
#define MW_UTIL_H

#include <stddef.h>

/* Allocation helpers: abort with a message on out-of-memory. */
void *mw_xmalloc(size_t n);
void *mw_xrealloc(void *p, size_t n);
char *mw_xstrdup(const char *s);

/* Growable, always NUL-terminated string buffer. */
typedef struct { char *data; size_t len, cap; } MwBuf;
void mwbuf_init(MwBuf *b);
void mwbuf_free(MwBuf *b);
void mwbuf_append(MwBuf *b, const char *s);
void mwbuf_appendn(MwBuf *b, const char *s, size_t n);
void mwbuf_putc(MwBuf *b, char c);
void mwbuf_printf(MwBuf *b, const char *fmt, ...);
void mwbuf_xml(MwBuf *b, const char *s);          /* append XML-escaped text */

/* Text helpers */
char  *mw_trim(char *s);                           /* in place, returns s    */
size_t mw_utf8_len(const char *s);                 /* code points            */
size_t mw_utf8_width(const char *s);               /* terminal columns (CJK=2)*/
size_t mw_utf8_prefix(const char *s, size_t maxcols);/* bytes that fit maxcols */
int    mw_ieq(const char *a, const char *b);       /* ASCII case-insens ==   */
int    mw_ieq_ext(const char *path, const char *ext); /* case-insens suffix */

/* Tiny XML helpers (enough to read the files this program writes) */
void        mw_xml_unescape(char *s);              /* in place              */
const char *mw_xml_next_open(const char *s, const char *name);
const char *mw_xml_tag_end(const char *lt);        /* ptr to closing '>'    */
int         mw_xml_attr(const char *s, const char *e, const char *name,
                        char *out, size_t n);

#endif
