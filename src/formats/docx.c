/* Office Open XML (.docx) writer / reader */
#include "formats/format.h"
#include "core/util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XML_HDR "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
#define W_NS "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\""

static const char CONTENT_TYPES[] =
    XML_HDR
    "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
    "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
    "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
    "<Override PartName=\"/word/document.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>"
    "<Override PartName=\"/word/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml\"/>"
    "</Types>";

static const char ROOT_RELS[] =
    XML_HDR
    "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
    "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"word/document.xml\"/>"
    "</Relationships>";

static const char DOC_RELS[] =
    XML_HDR
    "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
    "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" Target=\"styles.xml\"/>"
    "</Relationships>";

static const char STYLES[] =
    XML_HDR
    "<w:styles " W_NS ">"
    "<w:docDefaults>"
    "<w:rPrDefault><w:rPr><w:rFonts w:ascii=\"Times New Roman\" w:hAnsi=\"Times New Roman\" w:cs=\"Times New Roman\"/>"
    "<w:sz w:val=\"24\"/><w:szCs w:val=\"24\"/><w:lang w:val=\"es-MX\"/></w:rPr></w:rPrDefault>"
    "<w:pPrDefault><w:pPr><w:spacing w:after=\"120\" w:line=\"276\" w:lineRule=\"auto\"/></w:pPr></w:pPrDefault>"
    "</w:docDefaults>"
    "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/><w:qFormat/></w:style>"
    "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/><w:basedOn w:val=\"Normal\"/><w:next w:val=\"Normal\"/><w:qFormat/>"
    "<w:pPr><w:keepNext/><w:spacing w:before=\"240\" w:after=\"120\"/><w:outlineLvl w:val=\"0\"/></w:pPr>"
    "<w:rPr><w:b/><w:sz w:val=\"36\"/><w:szCs w:val=\"36\"/></w:rPr></w:style>"
    "<w:style w:type=\"paragraph\" w:styleId=\"Heading2\"><w:name w:val=\"heading 2\"/><w:basedOn w:val=\"Normal\"/><w:next w:val=\"Normal\"/><w:qFormat/>"
    "<w:pPr><w:keepNext/><w:spacing w:before=\"200\" w:after=\"120\"/><w:outlineLvl w:val=\"1\"/></w:pPr>"
    "<w:rPr><w:b/><w:sz w:val=\"28\"/><w:szCs w:val=\"28\"/></w:rPr></w:style>"
    "<w:style w:type=\"paragraph\" w:customStyle=\"1\" w:styleId=\"MWBold\"><w:name w:val=\"MW Bold\"/><w:basedOn w:val=\"Normal\"/><w:qFormat/>"
    "<w:rPr><w:b/><w:bCs/></w:rPr></w:style>"
    "<w:style w:type=\"paragraph\" w:customStyle=\"1\" w:styleId=\"MWItalic\"><w:name w:val=\"MW Italic\"/><w:basedOn w:val=\"Normal\"/><w:qFormat/>"
    "<w:rPr><w:i/><w:iCs/></w:rPr></w:style>"
    "</w:styles>";

static const char *style_id(MwStyle s)
{
    switch (s) {
    case MW_H1:     return "Heading1";
    case MW_H2:     return "Heading2";
    case MW_BOLD:   return "MWBold";
    case MW_ITALIC: return "MWItalic";
    default:        return NULL;
    }
}

/* Text run: tabs become <w:tab/>, everything else stays in <w:t> */
static void put_runs(MwBuf *b, const char *s)
{
    const char *seg = s, *p;
    mwbuf_append(b, "<w:r>");
    for (p = s;; p++) {
        if (*p == '\t' || *p == '\0') {
            if (p > seg) {
                MwBuf tmp;
                mwbuf_init(&tmp);
                mwbuf_appendn(&tmp, seg, (size_t)(p - seg));
                mwbuf_append(b, "<w:t xml:space=\"preserve\">");
                mwbuf_xml(b, tmp.data);
                mwbuf_append(b, "</w:t>");
                mwbuf_free(&tmp);
            }
            if (*p == '\0') break;
            mwbuf_append(b, "<w:tab/>");
            seg = p + 1;
        }
    }
    mwbuf_append(b, "</w:r>");
}

static int write_parts(const MwDoc *d, const char *dir, const char *title)
{
    MwBuf c;
    size_t i;
    int r = 0;
    (void)title;

    mwbuf_init(&c);
    mwbuf_append(&c, XML_HDR "<w:document " W_NS "><w:body>");
    for (i = 0; i < d->count; i++) {
        const MwPara *p = &d->items[i];
        const char *id = style_id(p->style);
        mwbuf_append(&c, "<w:p>");
        if (id) mwbuf_printf(&c, "<w:pPr><w:pStyle w:val=\"%s\"/></w:pPr>", id);
        if (p->text[0]) put_runs(&c, p->text);
        mwbuf_append(&c, "</w:p>");
    }
    mwbuf_append(&c, "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                     "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" "
                     "w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>"
                     "</w:body></w:document>");

    r |= mw_write_part(dir, "[Content_Types].xml", CONTENT_TYPES);
    r |= mw_write_part(dir, "_rels/.rels", ROOT_RELS);
    r |= mw_write_part(dir, "word/_rels/document.xml.rels", DOC_RELS);
    r |= mw_write_part(dir, "word/styles.xml", STYLES);
    r |= mw_write_part(dir, "word/document.xml", c.data);
    mwbuf_free(&c);
    return r ? -1 : 0;
}

/* ---- reader ------------------------------------------------------- */

static MwStyle style_from(const char *id)
{
    if (!strcmp(id, "Heading1") || !strcmp(id, "Title")) return MW_H1;
    if (!strcmp(id, "Heading2")) return MW_H2;
    if (!strcmp(id, "MWBold"))   return MW_BOLD;
    if (!strcmp(id, "MWItalic")) return MW_ITALIC;
    return MW_NORMAL;
}

static int is_tag(const char *s, const char *name)   /* "<w:t" + [ >/] */
{
    size_t n = strlen(name);
    char c;
    if (strncmp(s, name, n)) return 0;
    c = s[n];
    return c == ' ' || c == '>' || c == '/';
}

static int parse(MwDoc *doc, char *xml)
{
    const char *p = xml;
    for (;;) {
        const char *s = mw_xml_next_open(p, "w:p");
        const char *te, *close, *ps, *pe, *q;
        char style[64] = "";
        MwBuf t;

        if (!s) break;
        te = mw_xml_tag_end(s);
        if (!te) break;
        if (te[-1] == '/') { mwdoc_add(doc, "", MW_NORMAL); p = te + 1; continue; }
        close = strstr(te + 1, "</w:p>");
        if (!close) break;

        ps = mw_xml_next_open(te + 1, "w:pStyle");
        if (ps && ps < close) {
            const char *pte = mw_xml_tag_end(ps);
            if (pte) mw_xml_attr(ps, pte, "w:val", style, sizeof style);
        }

        q = te + 1;
        pe = strstr(q, "</w:pPr>");              /* skip paragraph properties */
        if (pe && pe < close) q = pe + 8;

        mwbuf_init(&t);
        while (q < close) {
            if (*q == '<') {
                const char *tt = mw_xml_tag_end(q);
                if (!tt) break;
                if (is_tag(q, "<w:t") && tt[-1] != '/') {
                    const char *end = strstr(tt + 1, "</w:t>");
                    if (!end || end > close) end = close;
                    mwbuf_appendn(&t, tt + 1, (size_t)(end - tt - 1));
                    q = end;
                    continue;
                }
                if (is_tag(q, "<w:tab")) mwbuf_putc(&t, '\t');
                else if (is_tag(q, "<w:br")) mwbuf_putc(&t, ' ');
                q = tt + 1;
            } else q++;
        }
        mw_xml_unescape(t.data);
        mwdoc_add(doc, t.data, style_from(style));
        mwbuf_free(&t);
        p = close + 6;
    }
    return 0;
}

static const char *const ENTRIES[] = { "[Content_Types].xml", "_rels", "word", NULL };

const MwFormatOps mw_docx_ops = {
    ".docx", "Office Open XML", write_parts,
    NULL, ENTRIES, "word/document.xml", parse
};
