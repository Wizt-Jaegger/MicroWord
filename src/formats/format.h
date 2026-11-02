#ifndef MW_FORMAT_H
#define MW_FORMAT_H

#include "core/document.h"

/* A document format = how to write its XML parts, which entries go into the
 * zip container, and how to read its main part back. */
typedef struct {
    const char *ext;                 /* ".odt"                               */
    const char *name;                /* "OpenDocument Text"                  */
    int  (*write_parts)(const MwDoc *d, const char *dir, const char *title);
    const char *stored_first;        /* zipped first, uncompressed (or NULL) */
    const char *const *entries;      /* top-level zip entries, NULL-ended    */
    const char *main_part;           /* part to parse when loading           */
    int  (*parse)(MwDoc *d, char *xml);
} MwFormatOps;

extern const MwFormatOps mw_odt_ops;
extern const MwFormatOps mw_docx_ops;

const MwFormatOps *mw_format_for_path(const char *path);   /* NULL if unknown */

/* Both return 0 on success; on failure write a message to err. */
int mw_save(const MwDoc *d, const char *path, char *err, size_t en);
int mw_load(MwDoc *d, const char *path, char *err, size_t en);

/* Writes `data` to dir/rel, creating intermediate directories. */
int mw_write_part(const char *dir, const char *rel, const char *data);

#endif
