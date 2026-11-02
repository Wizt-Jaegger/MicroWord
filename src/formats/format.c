#include "formats/format.h"
#include "platform/platform.h"
#include "core/util.h"
#include "i18n/i18n.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const MwFormatOps *mw_format_for_path(const char *path)
{
    if (mw_ieq_ext(path, mw_odt_ops.ext))  return &mw_odt_ops;
    if (mw_ieq_ext(path, mw_docx_ops.ext)) return &mw_docx_ops;
    return NULL;
}

int mw_write_part(const char *dir, const char *rel, const char *data)
{
    char path[1024];
    char *p;
    FILE *f;
    snprintf(path, sizeof path, "%s/%s", dir, rel);
    for (p = path + strlen(dir) + 1; *p; p++) {     /* make parent dirs */
        if (*p == '/') {
            *p = '\0';
            if (mw_mkdir(path) != 0) return -1;
            *p = '/';
        }
    }
    f = fopen(path, "wb");
    if (!f) return -1;
    if (fputs(data, f) < 0) { fclose(f); return -1; }
    return fclose(f) == 0 ? 0 : -1;
}

static char *read_all(const char *path)
{
    FILE *f = fopen(path, "rb");
    long sz;
    char *buf;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0) { fclose(f); return NULL; }
    buf = mw_xmalloc((size_t)sz + 1);
    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) { free(buf); fclose(f); return NULL; }
    buf[sz] = '\0';
    fclose(f);
    return buf;
}

int mw_save(const MwDoc *d, const char *path, char *err, size_t en)
{
    const MwFormatOps *ops = mw_format_for_path(path);
    char tmp[512], zip[600], title[256];
    const char *base, *dot;
    size_t n;

    if (!ops) { i18n_fmt(err, en, "err.fmt_unsupported"); return -1; }
    if (!mw_tools_available()) { i18n_fmt(err, en, mw_tools_hint_key()); return -1; }
    if (mw_make_tempdir(tmp, sizeof tmp) != 0) {
        i18n_fmt(err, en, "err.tmp_failed");
        return -1;
    }

    /* document title = file name without directory and extension */
    base = strrchr(path, '/');
#ifdef MW_OS_WINDOWS
    { const char *b2 = strrchr(path, '\\'); if (b2 && (!base || b2 > base)) base = b2; }
#endif
    base = base ? base + 1 : path;
    dot = strrchr(base, '.');
    n = dot ? (size_t)(dot - base) : strlen(base);
    if (n >= sizeof title) n = sizeof title - 1;
    memcpy(title, base, n);
    title[n] = '\0';

    if (ops->write_parts(d, tmp, title) != 0) {
        i18n_fmt(err, en, "err.write_xml");
        mw_rmtree(tmp);
        return -1;
    }
    if (mw_zip_create(tmp, "out.zip", ops->stored_first, ops->entries) != 0) {
        i18n_fmt(err, en, "err.zip_failed");
        mw_rmtree(tmp);
        return -1;
    }
    snprintf(zip, sizeof zip, "%s/out.zip", tmp);
    if (mw_move_file(zip, path) != 0) {
        i18n_fmt(err, en, "err.write_file", path);
        mw_rmtree(tmp);
        return -1;
    }
    mw_rmtree(tmp);
    return 0;
}

int mw_load(MwDoc *d, const char *path, char *err, size_t en)
{
    const MwFormatOps *ops = mw_format_for_path(path);
    char tmp[512], part[600];
    unsigned char magic[2] = {0, 0};
    FILE *f;
    char *xml;
    int r;

    if (!ops) { i18n_fmt(err, en, "err.fmt_unsupported"); return -1; }
    f = fopen(path, "rb");
    if (!f) { i18n_fmt(err, en, "err.cannot_open", path); return -1; }
    if (fread(magic, 1, 2, f) != 2) magic[0] = 0;
    fclose(f);
    if (magic[0] != 'P' || magic[1] != 'K') {
        i18n_fmt(err, en, "err.not_zip", path);
        return -1;
    }
    if (!mw_tools_available()) { i18n_fmt(err, en, mw_tools_hint_key()); return -1; }
    if (mw_make_tempdir(tmp, sizeof tmp) != 0) {
        i18n_fmt(err, en, "err.tmp_failed");
        return -1;
    }
    if (mw_zip_extract(path, tmp) != 0) {
        i18n_fmt(err, en, "err.unzip_failed", path);
        mw_rmtree(tmp);
        return -1;
    }
    snprintf(part, sizeof part, "%s/%s", tmp, ops->main_part);
    xml = read_all(part);
    mw_rmtree(tmp);
    if (!xml) { i18n_fmt(err, en, "err.missing_part", ops->main_part); return -1; }

    mwdoc_clear(d);
    r = ops->parse(d, xml);
    free(xml);
    if (r != 0) i18n_fmt(err, en, "err.parse_failed");
    return r;
}
