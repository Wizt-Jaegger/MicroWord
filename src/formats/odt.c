/* OpenDocument Text (.odt) writer / reader */
#include "formats/format.h"
#include "core/util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NS_ALL \
    "xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" " \
    "xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\" " \
    "xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" " \
    "xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" " \
    "xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\" " \
    "xmlns:dc=\"http://purl.org/dc/elements/1.1/\" "

#define XML_HDR "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"

static const char MIMETYPE[] = "application/vnd.oasis.opendocument.text";

static const char MANIFEST[] =
    XML_HDR
    "<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.2\">\n"
    " <manifest:file-entry manifest:full-path=\"/\" manifest:version=\"1.2\" manifest:media-type=\"application/vnd.oasis.opendocument.text\"/>\n"
    " <manifest:file-entry manifest:full-path=\"content.xml\" manifest:media-type=\"text/xml\"/>\n"
    " <manifest:file-entry manifest:full-path=\"styles.xml\" manifest:media-type=\"text/xml\"/>\n"
    " <manifest:file-entry manifest:full-path=\"meta.xml\" manifest:media-type=\"text/xml\"/>\n"
    "</manifest:manifest>\n";

static const char STYLES[] =
    XML_HDR
    "<office:document-styles " NS_ALL "office:version=\"1.2\">\n"
    "<office:styles>\n"
    " <style:style style:name=\"Standard\" style:family=\"paragraph\" style:class=\"text\">\n"
    "  <style:paragraph-properties fo:margin-top=\"0cm\" fo:margin-bottom=\"0.212cm\" fo:line-height=\"115%\"/>\n"
    "  <style:text-properties fo:font-family=\"'Times New Roman'\" fo:font-size=\"12pt\"/>\n"
    " </style:style>\n"
    " <style:style style:name=\"Heading_20_1\" style:display-name=\"Heading 1\" style:family=\"paragraph\" style:parent-style-name=\"Standard\" style:next-style-name=\"Standard\" style:default-outline-level=\"1\" style:class=\"text\">\n"
    "  <style:paragraph-properties fo:margin-top=\"0.423cm\" fo:margin-bottom=\"0.212cm\" fo:keep-with-next=\"always\"/>\n"
    "  <style:text-properties fo:font-size=\"18pt\" fo:font-weight=\"bold\"/>\n"
    " </style:style>\n"
    " <style:style style:name=\"Heading_20_2\" style:display-name=\"Heading 2\" style:family=\"paragraph\" style:parent-style-name=\"Standard\" style:next-style-name=\"Standard\" style:default-outline-level=\"2\" style:class=\"text\">\n"
    "  <style:paragraph-properties fo:margin-top=\"0.353cm\" fo:margin-bottom=\"0.212cm\" fo:keep-with-next=\"always\"/>\n"
    "  <style:text-properties fo:font-size=\"14pt\" fo:font-weight=\"bold\"/>\n"
    " </style:style>\n"
    " <style:style style:name=\"MW_Bold\" style:display-name=\"MW Bold\" style:family=\"paragraph\" style:parent-style-name=\"Standard\">\n"
    "  <style:text-properties fo:font-weight=\"bold\"/>\n"
    " </style:style>\n"
    " <style:style style:name=\"MW_Italic\" style:display-name=\"MW Italic\" style:family=\"paragraph\" style:parent-style-name=\"Standard\">\n"
    "  <style:text-properties fo:font-style=\"italic\"/>\n"
    " </style:style>\n"
    "</office:styles>\n"
    "<office:automatic-styles>\n"
    " <style:page-layout style:name=\"pm1\">\n"
    "  <style:page-layout-properties fo:page-width=\"21.59cm\" fo:page-height=\"27.94cm\" fo:margin-top=\"2.54cm\" fo:margin-bottom=\"2.54cm\" fo:margin-left=\"2.54cm\" fo:margin-right=\"2.54cm\"/>\n"
    " </style:page-layout>\n"
    "</office:automatic-styles>\n"
    "<office:master-styles>\n"
    " <style:master-page style:name=\"Standard\" style:page-layout-name=\"pm1\"/>\n"
    "</office:master-styles>\n"
    "</office:document-styles>\n";

/* Text with ODF whitespace rules: runs of spaces -> <text:s/>, tab -> <text:tab/> */
static void put_text(MwBuf *b, const char *s)
{
    const char *p;
    char one[2] = {0, 0};
    for (p = s; *p; p++) {
        if (*p == ' ') {
            if (p == s || p[-1] == ' ') {
                int run = 0;
                while (p[run] == ' ') run++;
                mwbuf_printf(b, "<text:s text:c=\"%d\"/>", run);
                p += run - 1;
            } else mwbuf_putc(b, ' ');
        } else if (*p == '\t') {
            mwbuf_append(b, "<text:tab/>");
        } else {
            one[0] = *p;
            mwbuf_xml(b, one);
        }
    }
}

static int write_parts(const MwDoc *d, const char *dir, const char *title)
{
    MwBuf c;
    char date[32];
    time_t now = time(NULL);
    size_t i;
    int r = 0;

    mwbuf_init(&c);
    mwbuf_append(&c, XML_HDR "<office:document-content " NS_ALL "office:version=\"1.2\">\n"
                     "<office:body><office:text>\n");
    for (i = 0; i < d->count; i++) {
        const MwPara *p = &d->items[i];
        const char *tag = "text:p", *style = "Standard", *extra = "";
        switch (p->style) {
        case MW_H1:     tag = "text:h"; style = "Heading_20_1"; extra = " text:outline-level=\"1\""; break;
        case MW_H2:     tag = "text:h"; style = "Heading_20_2"; extra = " text:outline-level=\"2\""; break;
        case MW_BOLD:   style = "MW_Bold";   break;
        case MW_ITALIC: style = "MW_Italic"; break;
        default: break;
        }
        mwbuf_printf(&c, "<%s text:style-name=\"%s\"%s>", tag, style, extra);
        put_text(&c, p->text);
        mwbuf_printf(&c, "</%s>\n", tag);
    }
    mwbuf_append(&c, "</office:text></office:body></office:document-content>\n");

    strftime(date, sizeof date, "%Y-%m-%dT%H:%M:%S", gmtime(&now));
    {
        MwBuf m;
        mwbuf_init(&m);
        mwbuf_append(&m, XML_HDR "<office:document-meta " NS_ALL "office:version=\"1.2\">"
                         "<office:meta><meta:generator>microWord</meta:generator><dc:title>");
        mwbuf_xml(&m, title);
        mwbuf_printf(&m, "</dc:title><meta:creation-date>%s</meta:creation-date>"
                         "</office:meta></office:document-meta>\n", date);
        r |= mw_write_part(dir, "meta.xml", m.data);
        mwbuf_free(&m);
    }

    r |= mw_write_part(dir, "mimetype", MIMETYPE);          /* no trailing newline */
    r |= mw_write_part(dir, "META-INF/manifest.xml", MANIFEST);
    r |= mw_write_part(dir, "styles.xml", STYLES);
    r |= mw_write_part(dir, "content.xml", c.data);
    mwbuf_free(&c);
    return r ? -1 : 0;
}

/* ---- reader ------------------------------------------------------- */

static void inner_text(const char *s, const char *e, MwBuf *out)
{
    while (s < e) {
        if (*s == '<') {
            const char *te = mw_xml_tag_end(s);
            if (!te || te >= e) break;
            if (!strncmp(s, "<text:s", 7) && (s[7] == ' ' || s[7] == '/' || s[7] == '>')) {
                char v[16];
                int n = 1;
                if (mw_xml_attr(s, te, "text:c", v, sizeof v)) n = atoi(v);
                while (n-- > 0 && out->len < (1u << 24)) mwbuf_putc(out, ' ');
            } else if (!strncmp(s, "<text:tab", 9)) {
                mwbuf_putc(out, '\t');
            } else if (!strncmp(s, "<text:line-break", 16)) {
                mwbuf_putc(out, ' ');
            }
            s = te + 1;
        } else mwbuf_putc(out, *s++);
    }
}

static MwStyle style_from(const char *name, int is_heading, int level)
{
    if (!strcmp(name, "Heading_20_1") || !strcmp(name, "Title")) return MW_H1;
    if (!strcmp(name, "Heading_20_2")) return MW_H2;
    if (!strcmp(name, "MW_Bold"))   return MW_BOLD;
    if (!strcmp(name, "MW_Italic")) return MW_ITALIC;
    if (is_heading) return level <= 1 ? MW_H1 : MW_H2;
    return MW_NORMAL;
}

static int parse(MwDoc *doc, char *xml)
{
    const char *p = strstr(xml, "<office:text");
    if (!p) p = xml;
    for (;;) {
        const char *a = mw_xml_next_open(p, "text:p");
        const char *b = mw_xml_next_open(p, "text:h");
        const char *s, *te, *close, *closetag;
        char style[128] = "", level[16] = "";
        int is_h;
        MwStyle st;
        MwBuf t;

        if (!a && !b) break;
        if (a && (!b || a < b)) { s = a; is_h = 0; } else { s = b; is_h = 1; }
        te = mw_xml_tag_end(s);
        if (!te) break;
        mw_xml_attr(s, te, "text:style-name", style, sizeof style);
        mw_xml_attr(s, te, "text:outline-level", level, sizeof level);
        st = style_from(style, is_h, atoi(level));

        if (te[-1] == '/') { mwdoc_add(doc, "", st); p = te + 1; continue; }

        closetag = is_h ? "</text:h>" : "</text:p>";
        close = strstr(te + 1, closetag);
        if (!close) break;

        mwbuf_init(&t);
        inner_text(te + 1, close, &t);
        mw_xml_unescape(t.data);
        mwdoc_add(doc, t.data, st);
        mwbuf_free(&t);
        p = close + strlen(closetag);
    }
    return 0;
}

static const char *const ENTRIES[] = { "META-INF", "content.xml", "styles.xml", "meta.xml", NULL };

const MwFormatOps mw_odt_ops = {
    ".odt", "OpenDocument Text", write_parts,
    "mimetype", ENTRIES, "content.xml", parse
};
