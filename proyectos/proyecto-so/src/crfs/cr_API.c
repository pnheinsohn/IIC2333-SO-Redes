#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include "cr_API.h"
#include "functions/functions.h"
#include "structs/graph.h"

extern int errno;
int errnum;

char* DISK_PATH;

/////////////////////////////////////
//        Private Functions        //
/////////////////////////////////////

static crFILE *crFILE_init(Dir_parser *directory, Index_block *iblock, unsigned char mode, char *path)
{
  crFILE *cr_file = malloc(sizeof(crFILE));


  cr_file -> mode = mode;
  cr_file -> iblock = iblock;
  cr_file -> amount_read = 0;
  cr_file -> actual_offset = 0;
  cr_file -> actual_data_index = 0;
  cr_file -> path = malloc(sizeof(char) * (strlen(path) + 1));
  strcpy(cr_file -> path, path);

  cr_file -> directory = directory;
  char *dir_name = strrchr(cr_file -> path, '/');
  dir_name++;
  directory -> name = dir_name;

  cr_file -> data_pointers = calloc(5620, sizeof(unsigned int));

  for (int i = 0; i < 500; i++) cr_file -> data_pointers[i] = cr_file -> iblock -> data_pointers[i];

  unsigned int *data_buffer;
  for (int i = 0; i < 10; i++) {
    data_buffer = read_data_block(cr_file -> iblock -> indirect_blocks[i]);
    for (int j = 0; j < 512; j++) cr_file -> data_pointers[500 + i * 512 + j] = data_buffer[j];
    free(data_buffer);
  }


  return cr_file;
}

////////////////////////////////////
//        Public Functions        //
////////////////////////////////////

/** Monta el disco */
void cr_mount(char* diskname)
{
  FILE * pf;
  pf = fopen (diskname, "rb");
  if (pf == NULL) {
    errnum = errno;
    fprintf(stderr, "Error mounting disk: %s\n", strerror(errnum));
    exit(errnum);
  } else {
    DISK_PATH = diskname;
    fclose (pf);
  }
}

/** Printea el bitmap, la cantidad de 1's y 0's */
void cr_bitmap(void)
{
  Graph* graph = load_disk();
  int count_ones = 0;
  for (int i = 0; i < 8192; i++) {
    count_ones += byteToBits(graph -> bytemap[i]);
  }
  printf("\n%i\n", count_ones);
  printf("%i\n", 8192 * 8 - count_ones);
  graph_destroy(graph);
}

int cr_exists(char* path)
{
  Graph* graph = load_disk();
  // graph_printer(graph);
  Node *entry = graph_search(graph -> root, path);
  if (!entry) {
    errno = 2;
    fprintf(stderr, "Error opening file: %s\n", strerror(errno));
    graph_destroy(graph);
    return -1;
  }
  else {
    graph_destroy(graph);
    return 0;
  }
}

/** Recibe un path de la forma "/carpeta/file_or_directory" */
void cr_ls(char* path)
{
  Graph* graph = load_disk();
  // graph_printer(graph);
  Node *entry = graph_search(graph -> root, path);
  if (entry) {
    if (entry -> type == (unsigned char) 4) {
      errnum = ENOTDIR;
      fprintf(stderr, "Error reading %s: %s\n", path, strerror(errnum));
    }
    else {
      printf("Directorios y archivos en %s:\n", path);
      for (int i = 0; i < entry -> count; i++)
      {
        if (i != entry -> count - 1) printf("%s, ", entry->childs[i] -> name);
        else printf("%s\n", entry->childs[i] -> name);
      }
    }
  } else {
    errno = 2;
    fprintf(stderr, "Error opening file: %s\n", strerror(errno));
  }
  graph_destroy(graph);
}

int cr_mkdir(char *foldername)
{
  Graph* graph = load_disk();

  Node *aux = graph_search(graph -> root, foldername);
  // Si ya existe le directorio
  if (aux) {
    errnum = EEXIST;
    fprintf(stderr, "Error: %s: %s\n", foldername, strerror(errnum));
    graph_destroy(graph);
    return -1;
  }

  // Copia del str para poder modificarlo
  char dir_copy[1000];
	strcpy(dir_copy, foldername);

  // Nombre del nuevo directorio
  char *dir_name = strrchr(foldername, '/');
  dir_name++; // Le saco el / del comienzo
  int len_name = strlen(dir_name) - 1;

  // Se guarda path del padre en dir_copy
  trim_end(dir_copy, len_name + 2);

  // Verifica si existe el path
  Node *parent = graph_search(graph -> root, dir_copy);

  if (!parent) {
    graph_destroy(graph);
    cr_mkdir(dir_copy);
    graph = load_disk();
    parent = graph_search(graph -> root, dir_copy);
  }

  // Si el arbol encuentra un archivo y no un directorio
  if (parent -> type == (unsigned char) 4) {
    errnum = ENOTDIR;
    fprintf(stderr, "Error reading: %s\n", strerror(errnum));
  }
  else {
    // Busco el siguiente bloque libre
    unsigned int index = next_free_block(graph -> bytemap);
    if (index == 0) return -1;

    // Escribo el directorio en el disco
    Dir_parser* dir_block = dir_parser_init(2, dir_name, index, 0);
    write_dir_block(parent -> index, dir_block);
    write_bitmap(index, 1);

    dir_parser_destroy(dir_block);

    graph_destroy(graph);

    return 0;
  }

  graph_destroy(graph);

  return -1;
}

crFILE* cr_open(char* path, char mode)
{
  if (mode != 'r' && mode != 'w') {
    fprintf(stderr, "Invalid char '%c' for mode: must be 'r' or 'w'\n", mode);
    return NULL;
  }

  crFILE *cr_file;
  Index_block *iblock;
  Dir_parser *directory;
  Graph* graph = load_disk();
  Node *entry = graph_search(graph -> root, path);

  if (!entry && mode == 'r') {  // Cannot read an inexisting FILE
    errno = 2;
    fprintf(stderr, "Error opening file: %s\n", strerror(errno));
    graph_destroy(graph);
    return NULL;
  } else if (entry && entry -> type != (unsigned char) 4) {  // Cannot open a directory
    fprintf(stderr, "Error opening file: %s is a directory, not a file\n", path);
    graph_destroy(graph);
    return NULL;
  } else if (mode == 'w') {  // Create a crFILE
    if (entry) {  // Delete FILE only if exists
      cr_rm(path);
    }
    char dir_copy[1000];
    strcpy(dir_copy, path);
    char *dir_name = strrchr(path, '/');
    dir_name++;  // Le saco el / del comienzo
    int len_name = strlen(dir_name) - 1;
    trim_end(dir_copy, len_name + 2);

    Node *parent = graph_search(graph -> root, dir_copy);
    if (!parent) {
      graph_destroy(graph);
      cr_mkdir(dir_copy);
      graph = load_disk();
      parent = graph_search(graph -> root, dir_copy);
    } else if (parent -> type == (unsigned char) 4) {
      errnum = ENOTDIR;
      fprintf(stderr, "Error reading: %s\n", strerror(errnum));
      graph_destroy(graph);
      return NULL;
    }

    unsigned int index = next_free_block(graph -> bytemap);
    if (index == 0) {
      graph_destroy(graph);
      return NULL;
    }

    directory = dir_parser_init(4, dir_name, index, 0);
    write_dir_block(parent -> index, directory);

    iblock = iblock_init(0, 1, NULL, NULL);
    write_index_block(directory -> index, iblock, directory -> offset);
    write_bitmap(directory -> index, 1);

  } else {  // Find the FILE and instance a crFILE
    directory = malloc(sizeof(Dir_parser));
    directory -> type = entry -> type;
    directory -> offset = entry -> offset;
    directory -> index = entry -> index;
    iblock = read_index_block(directory -> index);
  }

  cr_file = crFILE_init(directory, iblock, (unsigned char) mode, path);
  graph_destroy(graph);
  return cr_file;
}

int cr_read(crFILE* file_desc, void *buffer, int nbytes)
{
  if (!file_desc) {
    errno = 2;
    fprintf(stderr, "Error reading file: %s\n", strerror(errno));
    return -1;
  } else if (file_desc -> mode != 'r') {
    errno = EACCES;
    fprintf(stderr, "Error reading file: %s\n", strerror(errno));
    return -1;
  }

  if (file_desc -> amount_read + nbytes > file_desc -> iblock -> size) {
    unsigned int aux_read = file_desc -> iblock -> size - file_desc -> amount_read;
    read_file_to_buffer(aux_read, file_desc, buffer);
    return aux_read;
  }
  read_file_to_buffer(nbytes, file_desc, buffer);
  return nbytes;
}

int cr_write(crFILE* file_desc, void* buffer, int nbytes)
{
  Graph* graph = load_disk();
  // graph_printer(graph);
  /** Work Here */
  graph_destroy(graph);
}

int cr_close(crFILE* file_desc)
{
  if (file_desc) {
    crFILE_destroy(file_desc);
    return 0;
  }
  errno = 2;
  fprintf(stderr, "Error closing file: %s\n", strerror(errno));
  return -1;
}

int cr_rm(char* path)
{
  Graph* graph = load_disk();

  Node *entry = graph_search(graph -> root, path);
  // El archivo no existe
  if (!entry) {
    errno = 2;
    fprintf(stderr, "Error opening file: %s\n", strerror(errno));
    graph_destroy(graph);
    return -1;
  }

  Index_block *iblock = read_index_block(entry -> index);

  // Su entrada queda invalida
  write_byte(entry -> parent -> index, entry -> offset, (unsigned char) 1);

  // Si tiene mas hardlinks
  if (iblock -> n_hardlinks > 1) {
    // Restamos un hardlink
    write_4bytes(entry -> index, 4, iblock -> n_hardlinks - 1);
  } else {
    // Lo borro del bitmap
    write_bitmap(entry -> index, 0);
    // Borramos el directorio y los bloques de datos
    for (int i = 0; i < 500; i++) write_bitmap(iblock -> data_pointers[i], 0);
    for (int i = 0; i < 10; i++) write_bitmap(iblock -> indirect_blocks[i], 0);
  }

  iblock_destroy(iblock);
  graph_destroy(graph);

  return 0;
}

int cr_hardlink(char* orig, char* dest)
{
  Graph* graph = load_disk();

  Node *entry = graph_search(graph -> root, orig);
  Node *aux_err = graph_search(graph -> root, dest);

  // El destino ya existe
  if (aux_err) {
    errno = EEXIST;
    fprintf(stderr, "Error creating hardlink: %s\n", strerror(errno));
    graph_destroy(graph);
    return -1;
  }
  // No existe el origen
  if (!entry) {
    errno = 2;
    fprintf(stderr, "Error creating hardlink: %s\n", strerror(errno));
    graph_destroy(graph);
    return -1;
  }
  // El origen es un directorio
  else if (entry -> type == (unsigned char) 2) {
    errno = EISDIR;
    fprintf(stderr, "Error creating hardlink: %s\n", strerror(errno));
    graph_destroy(graph);
    return -1;
  }
  else {
    // Copia del str para poder modificarlo
    char dir_copy[1000];
    strcpy(dir_copy, dest);

    // Nombre del nuevo directorio
    char *dir_name = strrchr(dest, '/');
    dir_name++; // Le saco el / del comienzo
    int len_name = strlen(dir_name) - 1;

    // Se guarda path del padre en dir_copy
    trim_end(dir_copy, len_name + 2);

    Node *dest_parent = graph_search(graph -> root, dir_copy);

    // Si existen carpetas que aun no se han creado las creo recursivamente
    if (!dest_parent) {
      graph_destroy(graph);
      cr_mkdir(dir_copy);
      graph = load_disk();
      dest_parent = graph_search(graph -> root, dir_copy);
      entry = graph_search(graph -> root, orig);
      aux_err = graph_search(graph -> root, dest);
    }

    // Nuevo hardlink
    Dir_parser* hl_block = dir_parser_init(4, dir_name, entry -> index, 0);
    write_dir_block(dest_parent -> index, hl_block);

    // Se lee el bloque indice
    Index_block *iblock = read_index_block(entry -> index);
    // Sumarle 1 a los hardlinks
    write_4bytes(entry -> index, 4, iblock -> n_hardlinks + 1);

    iblock_destroy(iblock);

    dir_parser_destroy(hl_block);
    graph_destroy(graph);
    return 0;
  }
}

int cr_unload(char* orig, char* dest)
{
  Graph* graph = load_disk();

  // Meto todo a una carpeta Downloads
  char *path = malloc(1000 * sizeof(char));
  strcpy(path, dest);
  mkdir(dest, 0777);
  strcat(path, "/");

  Node *dir = graph_search(graph -> root, orig);

  if (!dir) {
    errno = 2;
    fprintf(stderr, "Error opening file: %s\n", strerror(errno));
    graph_destroy(graph);
    free(path);
    return -1;
  }

  // Escribo recursivmente
  write_file(path, dir);
  free(path);
  graph_destroy(graph);

  return 0;
}

int cr_load(char* orig)
{
  Graph* graph = load_disk();
  // graph_printer(graph);
  /** Work Here */
  graph_destroy(graph);
}

////////////////////////////
//          Frees         //
////////////////////////////

void crFILE_destroy(crFILE *cr_file)
{
  dir_parser_destroy(cr_file -> directory);
  iblock_destroy(cr_file -> iblock);
  free(cr_file -> data_pointers);
  free(cr_file -> path);
  free(cr_file);
}

///////////////////////////////
//          Printers         //
///////////////////////////////

void crFILE_printer(crFILE *cr_file)
{
  if (!cr_file) {
    puts("--------");
    puts("No file");
    puts("--------");
    return;
  }
  puts("--------------");
  printf("Path: %s | Mode: %c\n", cr_file->path, cr_file->mode);
  puts("Directory");
  printf("Index: %u | Name: %s | Offset: %u | Type: %u\n",
    cr_file->directory->index, cr_file->directory->name,
    cr_file->directory->offset, cr_file->directory->type);

  puts("iblock");
  printf("nh: %u | Size: %u | Data Pointers: ",
          cr_file->iblock->n_hardlinks, cr_file->iblock->size);
  for (int i = 0; i < 5620; i++) {
    if (i != 5619) printf("%u, ", cr_file->data_pointers[i]);
    else printf("%u | Indirect Blocks: ", cr_file->data_pointers[i]);
  }
  puts("\n--------------");
}