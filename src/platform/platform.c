#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "platform/platform.h"
#include "core/util.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef MW_OS_WINDOWS
#  include <windows.h>
#  include <direct.h>
#  include <io.h>
#  include <process.h>
#  define MW_ISATTY _isatty
#  define MW_FILENO _fileno
#  define MW_NULL   ">nul 2>&1"
#else
#  include <unistd.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#  define MW_ISATTY isatty
#  define MW_FILENO fileno
#  define MW_NULL   ">/dev/null 2>&1"
#endif

/* ---------------------------------------------------------------- */

void mw_platform_init(void)
{
#ifdef MW_OS_WINDOWS
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    if (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode))
        SetConsoleMode(h, mode | 0x0004 /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */);
#endif
}

int mw_stdout_is_tty(void)
{
    return MW_ISATTY(MW_FILENO(stdout)) ? 1 : 0;
}

void mw_clear(void)
{
    fflush(stdout);
#ifdef MW_CLEAR_CMD
    if (mw_stdout_is_tty() && system(MW_CLEAR_CMD) == 0)
        return;
#endif
    fputs(MW_CLEAR_FALLBACK, stdout);   /* OS undetected or command failed */
    fflush(stdout);
}

/* ---------------------------------------------------------------- */

static int run(const char *cmd)
{
    fflush(stdout);
    return system(cmd) == 0 ? 0 : -1;
}

int mw_file_exists(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

int mw_path_is_shell_safe(const char *p)
{
    if (!p || !*p || p[0] == '-') return 0;
    for (; *p; p++) {
        unsigned char c = (unsigned char)*p;
        if (c < 0x20) return 0;
        if (strchr("\"`$%&|;<>!^*?", c)) return 0;
#ifndef MW_OS_WINDOWS
        if (c == '\\') return 0;
#endif
    }
    return 1;
}

int mw_tools_available(void)
{
    static int checked = 0, ok = 0;
    if (!checked) {
#ifdef MW_OS_WINDOWS
        ok = run("tar --version " MW_NULL) == 0;
#else
        ok = run("zip -v " MW_NULL) == 0 && run("unzip -v " MW_NULL) == 0;
#endif
        checked = 1;
    }
    return ok;
}

const char *mw_tools_hint_key(void)
{
#ifdef MW_OS_WINDOWS
    return "err.tools_win";
#else
    return "err.tools_posix";
#endif
}

/* ---------------------------------------------------------------- */

int mw_mkdir(const char *path)
{
#ifdef MW_OS_WINDOWS
    int r = _mkdir(path);
#else
    int r = mkdir(path, 0700);
#endif
    if (r == 0 || errno == EEXIST) return 0;
    return -1;
}

int mw_make_tempdir(char *out, size_t n)
{
    const char *base = NULL;
#ifdef MW_OS_WINDOWS
    base = getenv("TEMP");
    if (!base) base = getenv("TMP");
    if (!base || !mw_path_is_shell_safe(base)) base = ".";
    snprintf(out, n, "%s\\microword_%d_%lu", base, (int)_getpid(),
             (unsigned long)GetTickCount());
    return _mkdir(out) == 0 ? 0 : -1;
#else
    base = getenv("TMPDIR");
    if (!base || !mw_path_is_shell_safe(base)) base = "/tmp";
    snprintf(out, n, "%s/microword_XXXXXX", base);
    return mkdtemp(out) ? 0 : -1;
#endif
}

int mw_rmtree(const char *dir)
{
    char cmd[1024];
    if (!dir || !strstr(dir, "microword_") || !mw_path_is_shell_safe(dir))
        return -1;                       /* never delete anything we didn't create */
#ifdef MW_OS_WINDOWS
    snprintf(cmd, sizeof cmd, "rmdir /s /q \"%s\" " MW_NULL, dir);
#else
    snprintf(cmd, sizeof cmd, "rm -rf \"%s\" " MW_NULL, dir);
#endif
    return run(cmd);
}

/* ---------------------------------------------------------------- */

int mw_zip_create(const char *dir, const char *zipname,
                  const char *stored_first, const char *const *entries)
{
    MwBuf c;
    int i, r;
    if (!mw_path_is_shell_safe(dir)) return -1;
    mwbuf_init(&c);

#ifdef MW_OS_WINDOWS
    /* bsdtar ships with Windows 10+; "-a" picks the zip format from the name */
    mwbuf_printf(&c, "tar -a -c -f \"%s\\%s\" -C \"%s\"", dir, zipname, dir);
    if (stored_first) mwbuf_printf(&c, " \"%s\"", stored_first);
    for (i = 0; entries[i]; i++) mwbuf_printf(&c, " \"%s\"", entries[i]);
    mwbuf_append(&c, " " MW_NULL);
#else
    mwbuf_printf(&c, "cd \"%s\" && ", dir);
    if (stored_first)   /* -0 = store, -X = no extra file attributes */
        mwbuf_printf(&c, "zip -q -X -D -0 -nw \"%s\" \"%s\" && ", zipname, stored_first);
    mwbuf_printf(&c, "zip -q -X -D -r -nw \"%s\"", zipname);
    for (i = 0; entries[i]; i++) mwbuf_printf(&c, " \"%s\"", entries[i]);
    mwbuf_append(&c, " " MW_NULL);
#endif
    r = run(c.data);
    mwbuf_free(&c);
    return r;
}

int mw_zip_extract(const char *zipfile, const char *dir)
{
    char cmd[2048];
    if (!mw_path_is_shell_safe(zipfile) || !mw_path_is_shell_safe(dir)) return -1;
#ifdef MW_OS_WINDOWS
    snprintf(cmd, sizeof cmd, "tar -x -f \"%s\" -C \"%s\" " MW_NULL, zipfile, dir);
#else
    snprintf(cmd, sizeof cmd, "unzip -q -o \"%s\" -d \"%s\" " MW_NULL, zipfile, dir);
#endif
    return run(cmd);
}

int mw_move_file(const char *from, const char *to)
{
    FILE *a, *b;
    char buf[8192];
    size_t n;
    remove(to);
    if (rename(from, to) == 0) return 0;
    /* rename() fails across filesystems: copy instead */
    a = fopen(from, "rb");
    if (!a) return -1;
    b = fopen(to, "wb");
    if (!b) { fclose(a); return -1; }
    while ((n = fread(buf, 1, sizeof buf, a)) > 0)
        if (fwrite(buf, 1, n, b) != n) { fclose(a); fclose(b); return -1; }
    fclose(a);
    if (fclose(b) != 0) return -1;
    remove(from);
    return 0;
}
