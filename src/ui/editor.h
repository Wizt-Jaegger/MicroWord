#ifndef MW_EDITOR_H
#define MW_EDITOR_H

#include "core/document.h"

/* Interactive editor. `path` is where [G]uardar writes (.odt or .docx). */
void editor_run(MwDoc *doc, const char *path, int modified);

#endif
