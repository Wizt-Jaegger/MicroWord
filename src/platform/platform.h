#ifndef MW_PLATFORM_H
#define MW_PLATFORM_H

#include <stddef.h>

/* ------------------------------------------------------------------ *
 *  OS detection (compile time). Every OS-specific choice lives here.
 *  You can override any of these from the compiler, e.g.
 *      cc -DMW_CLEAR_CMD='"clear"' ...
 * ------------------------------------------------------------------ */
#if defined(_WIN32) || defined(_WIN64)
#  define MW_OS_WINDOWS 1
#  define MW_OS_NAME "Windows"
#  ifndef MW_CLEAR_CMD
#    define MW_CLEAR_CMD "cls"
#  endif
#elif defined(__APPLE__) && defined(__MACH__)
#  define MW_OS_MACOS 1
#  define MW_OS_NAME "macOS"
#  ifndef MW_CLEAR_CMD
#    define MW_CLEAR_CMD "clear"
#  endif
#elif defined(__linux__)
#  define MW_OS_LINUX 1
#  define MW_OS_NAME "Linux"
#  ifndef MW_CLEAR_CMD
#    define MW_CLEAR_CMD "clear"
#  endif
#elif defined(__unix__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#  define MW_OS_UNIX 1
#  define MW_OS_NAME "Unix"
#  ifndef MW_CLEAR_CMD
#    define MW_CLEAR_CMD "clear"
#  endif
#else
#  define MW_OS_UNKNOWN 1
#  define MW_OS_NAME "desconocido"
   /* no MW_CLEAR_CMD: mw_clear() will use the newline fallback */
#endif

/* Printed when the OS is undetected (or the clear command fails). */
#ifndef MW_CLEAR_FALLBACK
#  define MW_CLEAR_FALLBACK "\n\n\n"
#endif

/* Everything that is not Windows uses the POSIX zip/unzip tools. */
#ifdef MW_OS_WINDOWS
#  define MW_PATH_SEP '\\'
#else
#  define MW_PATH_SEP '/'
#endif

/* One-time setup (UTF-8 console + ANSI colors on Windows). */
void mw_platform_init(void);

/* Clear the screen using the OS command chosen at compile time. */
void mw_clear(void);

/* 1 if stdout is an interactive terminal. */
int  mw_stdout_is_tty(void);

/* Are the external zip/unzip (tar on Windows) tools present? */
int  mw_tools_available(void);
/* Translation key of the "please install the tools" hint for this OS. */
const char *mw_tools_hint_key(void);

/* Temp dirs: created with a "microword_" prefix; mw_rmtree refuses others. */
int  mw_make_tempdir(char *out, size_t n);
int  mw_mkdir(const char *path);              /* 0 on success or if it exists */
int  mw_rmtree(const char *dir);              /* 0 on success */

/* Compress: zip "dir"/<entries...> into dir/zipname.
 * If stored_first != NULL it is added first with no compression
 * (needed by ODF: the "mimetype" file must be first and uncompressed). */
int  mw_zip_create(const char *dir, const char *zipname,
                   const char *stored_first, const char *const *entries);
/* Uncompress zipfile into dir. */
int  mw_zip_extract(const char *zipfile, const char *dir);

int  mw_move_file(const char *from, const char *to);
int  mw_file_exists(const char *path);

/* Paths are passed to the shell inside double quotes: reject anything that
 * could escape them (and a leading '-', which zip/unzip read as an option). */
int  mw_path_is_shell_safe(const char *path);

#endif
