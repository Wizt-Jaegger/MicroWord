#include "core/util.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *mw_xmalloc(size_t n)
{
    void *p = malloc(n ? n : 1);
    if (!p) { fputs("microWord: sin memoria\n", stderr); exit(1); }
    return p;
}

void *mw_xrealloc(void *p, size_t n)
{
    p = realloc(p, n ? n : 1);
    if (!p) { fputs("microWord: sin memoria\n", stderr); exit(1); }
    return p;
}

char *mw_xstrdup(const char *s)
{
    size_t n = strlen(s) + 1;
    return memcpy(mw_xmalloc(n), s, n);
}

/* ---- MwBuf -------------------------------------------------------- */

void mwbuf_init(MwBuf *b)
{
    b->cap = 256;
    b->len = 0;
    b->data = mw_xmalloc(b->cap);
    b->data[0] = '\0';
}

void mwbuf_free(MwBuf *b)
{
    free(b->data);
    b->data = NULL;
    b->len = b->cap = 0;
}

static void reserve(MwBuf *b, size_t extra)
{
    if (b->len + extra + 1 <= b->cap) return;
    while (b->len + extra + 1 > b->cap) b->cap *= 2;
    b->data = mw_xrealloc(b->data, b->cap);
}

void mwbuf_appendn(MwBuf *b, const char *s, size_t n)
{
    reserve(b, n);
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
}

void mwbuf_append(MwBuf *b, const char *s) { mwbuf_appendn(b, s, strlen(s)); }
void mwbuf_putc(MwBuf *b, char c)          { mwbuf_appendn(b, &c, 1); }

void mwbuf_printf(MwBuf *b, const char *fmt, ...)
{
    va_list ap, ap2;
    int n;
    va_start(ap, fmt);
    va_copy(ap2, ap);
    n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (n > 0) {
        reserve(b, (size_t)n);
        vsnprintf(b->data + b->len, (size_t)n + 1, fmt, ap2);
        b->len += (size_t)n;
    }
    va_end(ap2);
}

void mwbuf_xml(MwBuf *b, const char *s)
{
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        switch (c) {
        case '&':  mwbuf_append(b, "&amp;");  break;
        case '<':  mwbuf_append(b, "&lt;");   break;
        case '>':  mwbuf_append(b, "&gt;");   break;
        case '"':  mwbuf_append(b, "&quot;"); break;
        case '\'': mwbuf_append(b, "&apos;"); break;
        default:
            if (c >= 0x20 || c == '\t' || c == '\n') mwbuf_putc(b, (char)c);
            /* other control chars are illegal in XML 1.0: dropped */
        }
    }
}

/* ---- text --------------------------------------------------------- */

char *mw_trim(char *s)
{
    char *e;
    while (*s && isspace((unsigned char)*s)) s++;
    e = s + strlen(s);
    while (e > s && isspace((unsigned char)e[-1])) *--e = '\0';
    return s;
}

size_t mw_utf8_len(const char *s)
{
    size_t n = 0;
    for (; *s; s++)
        if (((unsigned char)*s & 0xC0) != 0x80) n++;
    return n;
}

static unsigned long next_cp(const unsigned char **pp)
{
    const unsigned char *p = *pp;
    unsigned long cp;
    int extra;
    if (p[0] < 0x80)              { cp = p[0];        extra = 0; }
    else if ((p[0] & 0xE0) == 0xC0) { cp = p[0] & 0x1F; extra = 1; }
    else if ((p[0] & 0xF0) == 0xE0) { cp = p[0] & 0x0F; extra = 2; }
    else if ((p[0] & 0xF8) == 0xF0) { cp = p[0] & 0x07; extra = 3; }
    else                          { cp = 0xFFFD;      extra = 0; }
    p++;
    while (extra-- > 0 && (*p & 0xC0) == 0x80) cp = (cp << 6) | (*p++ & 0x3F);
    *pp = p;
    return cp;
}

static int cp_width(unsigned long cp)
{
    if (cp < 0x300) return 1;
    if (cp <= 0x36F) return 0;                          /* combining marks */
    if ((cp >= 0x1100 && cp <= 0x115F) || (cp >= 0x2E80 && cp <= 0xA4CF) ||
        (cp >= 0xAC00 && cp <= 0xD7A3) || (cp >= 0xF900 && cp <= 0xFAFF) ||
        (cp >= 0xFE30 && cp <= 0xFE6F) || (cp >= 0xFF00 && cp <= 0xFF60) ||
        (cp >= 0xFFE0 && cp <= 0xFFE6) || (cp >= 0x20000 && cp <= 0x3FFFD))
        return 2;                                       /* wide: CJK, kana, fullwidth */
    return 1;
}

size_t mw_utf8_width(const char *s)
{
    const unsigned char *p = (const unsigned char *)s;
    size_t w = 0;
    while (*p) w += (size_t)cp_width(next_cp(&p));
    return w;
}

size_t mw_utf8_prefix(const char *s, size_t maxcols)
{
    const unsigned char *start = (const unsigned char *)s, *p = start;
    size_t w = 0;
    while (*p) {
        const unsigned char *q = p;
        size_t cw = (size_t)cp_width(next_cp(&q));
        if (w + cw > maxcols) break;
        w += cw;
        p = q;
    }
    return (size_t)(p - start);
}

int mw_ieq(const char *a, const char *b)
{
    for (; *a && *b; a++, b++)
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
    return *a == *b;
}

int mw_ieq_ext(const char *path, const char *ext)
{
    size_t lp = strlen(path), le = strlen(ext);
    size_t i;
    if (lp < le) return 0;
    for (i = 0; i < le; i++)
        if (tolower((unsigned char)path[lp - le + i]) != tolower((unsigned char)ext[i]))
            return 0;
    return 1;
}

/* ---- XML ---------------------------------------------------------- */

static char *put_utf8(char *w, unsigned long cp)
{
    if (cp < 0x80)         { *w++ = (char)cp; }
    else if (cp < 0x800)   { *w++ = (char)(0xC0 | (cp >> 6));
                             *w++ = (char)(0x80 | (cp & 0x3F)); }
    else if (cp < 0x10000) { *w++ = (char)(0xE0 | (cp >> 12));
                             *w++ = (char)(0x80 | ((cp >> 6) & 0x3F));
                             *w++ = (char)(0x80 | (cp & 0x3F)); }
    else                   { *w++ = (char)(0xF0 | (cp >> 18));
                             *w++ = (char)(0x80 | ((cp >> 12) & 0x3F));
                             *w++ = (char)(0x80 | ((cp >> 6) & 0x3F));
                             *w++ = (char)(0x80 | (cp & 0x3F)); }
    return w;
}

void mw_xml_unescape(char *s)
{
    char *w = s;
    while (*s) {
        if (*s != '&') { *w++ = *s++; continue; }
        if      (!strncmp(s, "&amp;", 5))  { *w++ = '&';  s += 5; }
        else if (!strncmp(s, "&lt;", 4))   { *w++ = '<';  s += 4; }
        else if (!strncmp(s, "&gt;", 4))   { *w++ = '>';  s += 4; }
        else if (!strncmp(s, "&quot;", 6)) { *w++ = '"';  s += 6; }
        else if (!strncmp(s, "&apos;", 6)) { *w++ = '\''; s += 6; }
        else if (s[1] == '#') {
            char *end;
            unsigned long cp = (s[2] == 'x' || s[2] == 'X')
                             ? strtoul(s + 3, &end, 16)
                             : strtoul(s + 2, &end, 10);
            if (*end == ';' && cp > 0 && cp <= 0x10FFFF) {
                w = put_utf8(w, cp);   /* output never outruns input here */
                s = end + 1;
            } else *w++ = *s++;
        } else *w++ = *s++;
    }
    *w = '\0';
}

const char *mw_xml_next_open(const char *s, const char *name)
{
    size_t n = strlen(name);
    while ((s = strchr(s, '<')) != NULL) {
        if (strncmp(s + 1, name, n) == 0) {
            char c = s[1 + n];
            if (c == ' ' || c == '>' || c == '/' || c == '\t' || c == '\n' || c == '\r')
                return s;
        }
        s++;
    }
    return NULL;
}

const char *mw_xml_tag_end(const char *lt)
{
    char q = 0;
    for (lt++; *lt; lt++) {
        if (q) { if (*lt == q) q = 0; }
        else if (*lt == '"' || *lt == '\'') q = *lt;
        else if (*lt == '>') return lt;
    }
    return NULL;
}

int mw_xml_attr(const char *s, const char *e, const char *name, char *out, size_t n)
{
    size_t ln = strlen(name);
    const char *p;
    for (p = s + 1; p + ln + 2 < e; p++) {
        if (isspace((unsigned char)p[-1]) && !strncmp(p, name, ln) && p[ln] == '=') {
            char q = p[ln + 1];
            const char *v, *ve;
            size_t l;
            if (q != '"' && q != '\'') continue;
            v = p + ln + 2;
            ve = memchr(v, q, (size_t)(e - v));
            if (!ve) return 0;
            l = (size_t)(ve - v);
            if (l >= n) l = n - 1;
            memcpy(out, v, l);
            out[l] = '\0';
            return 1;
        }
    }
    return 0;
}
