#ifndef MW_I18N_H
#define MW_I18N_H

#include <stddef.h>

/* Loads <dir>/english.txt (required, the reference) plus every language
 * listed in <dir>/idiomas.txt. Returns 0 on success. */
int  i18n_init(const char *dir);

/* tr("menu.exit") -> text in the current language; falls back to English,
 * then to the key itself. Never returns NULL. */
const char *tr(const char *key);

/* snprintf using the translated string as the format. */
void i18n_fmt(char *out, size_t n, const char *key, ...);

/* Languages */
int         i18n_count(void);
const char *i18n_id_at(int i);       /* "spanish"           */
const char *i18n_name_at(int i);     /* "Español"           */
int         i18n_current(void);      /* index               */
int         i18n_select(const char *id);     /* 0 on success */
void        i18n_detect(void);               /* from LC_ALL / LANG   */
int         i18n_load_pref(const char *dir); /* <dir>/idioma.cfg     */
int         i18n_save_pref(const char *dir);

#endif
