#ifndef MW_PATHS_H
#define MW_PATHS_H

#include <stddef.h>

#define MW_DOCS_DIR  "Documentos"      /* where documents are created/opened */
#define MW_LANG_DIR  "Idiomas"         /* language files (english.txt, ...)  */
#define MW_LANG_PREF "idioma.cfg"      /* remembered language, inside MW_LANG_DIR */

/* Finds the project folder: the folder of the executable if it contains
 * Idiomas/english.txt, otherwise the current directory. */
void mw_paths_init(const char *argv0);

/* out = <base>/<sub>[/<name>] */
void mw_path(char *out, size_t n, const char *sub, const char *name);

/* A document name is a plain file name: no folders, no shell-unsafe chars. */
int  mw_valid_docname(const char *name);

#endif
