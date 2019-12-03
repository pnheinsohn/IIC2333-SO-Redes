# API de crfs

## Partes de la documentación

* Introducción
* Ejecución
* Estructuras
* Funciones

## Introducción

Los sistemas de archivos o de ficheros son una componente del sistema operativo que se encarga de controlar cómo los datos son almacenados y accedidos en memorias secundarias. Esta API se encarga de facilitar su uso mediante la abstracción de archivos para almacenar estos de manera ordenada.

## Ejecución

Para poder acceder a las funciones que se describirán a continuación es necesario implementar el header `"cr_API.h"`.

## Estructuras

1. crFILE: está compuesta por una estructura `Index_block` que representa su bloque índice y otra `Dir_parser`, que representa su bloque de directorio. Además contiene algunos valores que son útiles a la hora de realizar operaciones sobre el archivo.
2. Graph: Representa el árbol de directorios
3. Dir_parser: Representa un bloque de directorio.
4. Index_block: representa un bloque índice.

## Funciones

### Funciones Generales

1. <a name="cr_mount"></a>`void cr_mount(char* diskname)`

    * Se encarga de montar el disco, estableciendo como variable global la ruta local definida por `diskname` la que debe aludir al archivo .bin correspondiente al disco.
    * No retorna.
    * En caso de que el archivo no pueda abrirse mediante `FILE *fopen(const char *filename, const char *mode)` se imprime el `errno` correspondiente.

2. `void cr_bitmap()`
    * Imprime el bitmap en stdout. Para esto imprime el estado actual del disco de acuerdo al contenido del bitmap del disco montado [`cr_mount`](#cr_mount) (un 1 por cada bloque ocupado y un 0 por cada bloque libre). Luego procede a imprimir la cantidad de bloques ocupados, y en una tercera línea la cantidad de bloques libres.
    * No retorna.

3. `int cr_exists(char* path)`
    * Función para ver si un archivo o carpeta existe en la ruta especificada por `path`.
    * Retorna 0 si el archivo o carpeta existe.
    * En caso de que no exista, se retorna -1 junto con la impresión de `errno = 2` correspondiente a _"No such file or directory"_.

4. `void cr_ls(char* path)`
    * Lista los elementos de un directorio del disco, mostrando los nombres de todos los archivos y directorios contenidos en el directorio indicado por `path`.
    * En caso de que la ruta no exista, se retorna -1 junto con la impresión de `errno = 2` correspondiente a _"No such file or directory"_.
    * En caso de que la ruta sea un archivo, se retorna -1 junto con la impresión de `ENOTDIR` que corresponde a _"Not a directory"_.
    * No retorna.

5. `int cr_mkdir(char *foldername)`
    * Se encarga de crear un directorio vacío referido por `foldername`.
    * Retorna 0 si se creó el archivo.
    * En caso de que ya exista un directorio en el mismo `path` referido por `foldername` se imprime el `errno` correspondiente a `EEXIST` y se retorna -1. 
    * En caso, de que dicho `path` contenga una o más carpetas que no existen, entonces estas se crean recursivamente hasta crear el directorio pedido.

### Funciones de Manejo de Archivos

1. `crFILE* cr_open(char* path, char mode)`
    * Busca un archivo en la ruta `path` con el propósito especificado por `mode`, si éste es `r` entonces el acceso es solo de lectura, mientras que si es `w` el acceso es para escritura.
    * Retorna un `crFILE*` que lo representa.
    * En caso de que `mode = w` y el `path` contenga una o más carpetas que no existen, entonces estas se crean recursivamente hasta crear el archivo pedido.

2. `int cr_read(crFILE* file_desc, void* buffer, int nbytes)`
    * Función encargada de leer archivos, en donde, se leen los siguientes _nbytes_ desde el archivo descrito por `file_desc` y los guarda en la dirección apuntada por `buffer`. La lectura de `cr_read` se efectúa desde la posición del archivo inmediatamente posterior a la última posición leída por un llamado a esta función.
    * Retorna la cantidad de _bytes_ efectivamente leídos.
    *

3. `int cr_write(crFILE* file_desc, void* buffer, int nbytes)`
    * Función encargada de escribir archivos, en donde, se escribe en el archivo descrito por `file_desc` los _nbytes_ que se encuentren en la dirección indicada por `buffer`.
    * Retorna la cantidad de _Byte_ escritos en el archivo. Si se produjo un error porque no pudo seguir escribiendo, ya sea porque el disco se llenó o porque el archivo no puede crecer más, este número puede ser menor a _nbytes_ (incluso 0).
    *

4. `int cr_close(crFILE* file_desc)`
    * Cierra el archivo indicado por `file_desc`.
    * Retorna 0 cuando el archivo es cerrado correctamente.
    * Si no se pudo cerrar imprime el `errno` correspondiente a `EIO` y se retorna -1. 

5. `int cr_rm(char* path)`
    * Elimina el archivo referenciado por la ruta `path` del directorio correspondiente. Los bloques que estaban siendo usados por el archivo quedan libres si, y solo si, la cantidad de _hardlinks_ restante es igual a cero.
    * Retorna 0 cuando el archivo es eliminado correctamente.
    *

6. `int cr_hardlink(char* orig, char* dest)`
    * Se encarga de crear un _hardlink_ del archivo referenciado por `orig` en una nueva ruta `dest`, por lo que se aumentan la cantidad de referencias al archivo original.
    * Retorna 0 cuando el _hardlink_ se crea correctamente.
    * En caso, de que dicho `path` contenga una o más carpetas que no existen, entonces estas se crean recursivamente hasta crear el directorio pedido.


7. `int cr_unload(char* orig, char* dest)`
    * Función que se encarga de copiar un archivo o un árbol de directorios (es decir, un directorio y todos sus contenidos) del disco, referenciado por `orig`, hacia un nuevo archivo o directorio de ruta `dest` en su computador.
    * Retorna 0 cuando la copia se efectúa correctamente.
    * Si falla retorna -1.
    * En caso de que no exista `orig`, se retorna -1 junto con la impresión de `errno = 2` correspondiente a _"No such file or directory"_.

8. `int cr_load(char* orig)`
    * Función que se encarga de copiar un archivo o árbol de directorios, referenciado por `orig` al disco. En caso de que un archivo sea demasiado pesado para el disco, se escribe hasta acabar el espacio disponible.
    * Retorna 0 cuando la carga se efectúa correctamente.
