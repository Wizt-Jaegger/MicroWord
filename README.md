# microWord

Editor de documentos en consola, escrito en C. Crea, abre, edita y convierte
archivos **.odt** (OpenDocument) y **.docx** (Word) reales — contenedores zip
con XML valido, que abren en LibreOffice, Word, Google Docs, etc.

## Compilar

    make            # Linux / macOS / BSD
    ./microword

Windows (MinGW): `gcc -std=c99 -Isrc src/main.c src/*/*.c -o microword.exe` (no probado).

## Uso

Menu principal: crear, abrir/editar, eliminar, convertir `.odt <-> .docx`.

Comandos del editor: `A` agregar · `I n` insertar antes de n · `E n` editar ·
`B n` borrar · `T n` cambiar tipo · `V` ver todo · `G` guardar · `S` salir.

Al escribir, cada linea es un parrafo. Atajos: `# Titulo`, `## Subtitulo`,
`**negrita**`, `*cursiva*` (aplican al parrafo completo). `FIN` termina.

## Carpetas

    Documentos/         aqui se crean y abren los documentos (se crea sola)
    Idiomas/            todo el texto que ve el usuario, un .txt por idioma
    src/
      main.c            menu principal
      core/             util (strings, XML, UTF-8) · document (modelo) · paths (carpetas)
      i18n/             i18n.[ch]  (carga de Idiomas/*.txt, tr("clave"))
      platform/         platform.[ch]  (SO, clear, zip/unzip, temporales)
      formats/          format.[ch] (guardar/cargar) · odt.c · docx.c
      ui/               ui.[ch] (colores, cajas) · editor.[ch]
    build/              objetos compilados (se crea solo)

Los `.c` nuevos dentro de cualquier subcarpeta de `src/` se compilan solos;
los includes se escriben asi: `#include "core/util.h"`.
`Documentos/` e `Idiomas/` se buscan junto al ejecutable (o en la carpeta actual).

## Idiomas

Incluidos: English, Español, 日本語, Deutsch. Se elige solo segun `LANG`;
con la opcion 5 del menu se cambia y la eleccion se recuerda en `Idiomas/idioma.cfg`.

Formato de cada archivo (`Idiomas/<id>.txt`): `clave=texto`, `#` para comentarios,
`\n` salto de linea, `\\` barra invertida. Hay que conservar los marcadores
`%s` `%zu` `%c` y su orden (si no coinciden con english.txt, esa linea se ignora
y se usa el ingles). `english.txt` es la referencia: si falta una clave en otro
idioma se muestra en ingles.

Cada idioma tambien define las letras de los comandos del editor (`key.add`,
`key.save`, ...), las respuestas si/no (`confirm.yes`) y la palabra para
terminar de escribir (`end.word`).

**Agregar un idioma:** copia `english.txt` a `Idiomas/<id>.txt`, traducelo y agrega
`<id>` en `Idiomas/idiomas.txt`. No hay que recompilar.
`make check-lang` muestra que claves faltan o sobran en cada idioma.

## Compresion con herramientas del sistema

* Linux/macOS/BSD/desconocido: `zip` y `unzip` (`sudo apt install zip unzip`).
* Windows: `tar` (incluido en Windows 10+).

Los nombres de archivo se validan antes de llegar al shell.
Se puede forzar el comando de limpiar pantalla con `-DMW_CLEAR_CMD='"clear"'`.
