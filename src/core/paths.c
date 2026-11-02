#include "core/paths.h"
#include "platform/platform.h"

#include <stdio.h>
#include <string.h>

static char base[512] = ".";

void mw_paths_init(const char *argv0)
{
    const char *slash = NULL;
    strcpy(base, ".");
    if (!argv0) return;
    slash = strrchr(argv0, '/');
#ifdef MW_OS_WINDOWS
    { const char *b = strrchr(argv0, '\\'); if (b && (!slash || b > slash)) slash = b; }
#endif
    if (slash) {
        char cand[512], probe[600];
        size_t n = (size_t)(slash - argv0);
        if (n == 0) n = 1;                              /* "/microword" -> "/" */
        if (n < sizeof cand) {
            memcpy(cand, argv0, n);
            cand[n] = '\0';
            snprintf(probe, sizeof probe, "%s/%s/english.txt", cand, MW_LANG_DIR);
            if (mw_file_exists(probe)) strcpy(base, cand);
        }
    }
}

void mw_path(char *out, size_t n, const char *sub, const char *name)
{
    if (name) snprintf(out, n, "%s/%s/%s", base, sub, name);
    else      snprintf(out, n, "%s/%s", base, sub);
}

int mw_valid_docname(const char *name)
{
    if (!name || !*name) return 0;
    if (!strcmp(name, ".") || !strcmp(name, "..")) return 0;
    if (strchr(name, '/') || strchr(name, '\\')) return 0;
    return mw_path_is_shell_safe(name);
}
