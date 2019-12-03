#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "structs/structs.h"
#include "cr_API.h"


int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Modo de uso: ./crfs simdisk.bin\n");
    return 0;
  }

  int ret;

  // Se monta el disco.
  cr_mount(argv[1]);

  // 1
  cr_bitmap();
  // ------

  // 2
  puts(" ---- TESTING LS & EXISTS----");
  cr_ls("/root/memes");
  ret = cr_exists("/root/memes");
  printf("Ret value %i\n", ret);
  cr_ls("/root/unexisting.txt");
  ret = cr_exists("/root/unexisting.txt");
  printf("Ret value %i\n", ret);
  // ------

  // 3
  puts(" ---- TESTING MKDIR ----");
  cr_ls("/root/memes/testing/germy");
  cr_mkdir("/root/memes/testing/germy/test.txt");
  cr_ls("/root/memes/testing/germy");
  cr_rm("/root/memes/testing/germy/test.txt");
  cr_ls("/root/memes/testing/germy");
  // ------

  // 4
  puts(" ---- TESTING HARDLINK && UNLOAD ----");
  ret = cr_hardlink("/root/unexisting.mkv", "/root/memes/ruz/video_fail.mkv");
  printf("Ret hardlink fail: %i\n", ret);
  ret = cr_hardlink("/root/Program in C.mkv", "/root/memes/ruz/video.mkv");
  printf("Ret hardlink success: %i\n", ret);
  ret = cr_unload("/root/memes/unexisting", "Downloads");
  printf("Ret unload: %i\n", ret);
  ret = cr_unload("/root/memes/ruz", "Downloads");
  printf("Ret unload: %i\n", ret);
  // ------

  // 5
  crFILE* file;
  puts(" ---- OPEN LS & READ----");
  cr_ls("/root");
  file = cr_open("/root/unexisting.txt", 'r');
  cr_close(file);
  file = cr_open("/root/tester.txt", 'w');
  if (file) puts("File opened");
  cr_close(file);
  cr_ls("/root");
  char* buffer = calloc(10000, sizeof(char));
  file = cr_open("/root/germy.txt", 'r');
  ret = cr_read(file, buffer, 100);
  printf("Ret value %i\n", ret);
  printf("String actual: %s\n", buffer);
  ret = cr_read(file, buffer, 1000);
  printf("Ret value %i\n", ret);
  printf("String actual: %s\n", buffer);
  cr_close(file);
  free(buffer);
  // ------

  /** DONE
   * cr_mount(char *diskname);
   * cr_bitmap();
   * cr_ls("path");
   * cr_exists(char *path);
   * cr_mkdir(char *foldername);
   * cr_rm(char *path);
   * cr_open(char *path, char mode);
   * cr_close(crFILE *file_desc);
   * cr_read(crFILE *file_desc, void *buffer, int nbytes);
   * cr_unload(char *orig, char *dest);
  */

  /** TO DO
   * cr_write(crFILE *file_desc, void *buffer, int nbytes);
   * cr_load(char *orig);
  */

  // crFILE* file_desc = cr_open("../../files/test.txt", 'w');
  // Suponga que abrio y leyo un archivo desde su computador
  // almacenando su contenido en un arreglo f, de 300 byte. cr_write(file_desc, f, 300);
  // cr_cp("test.txt", "copy.txt");
  // cr_close(file_desc);
  return 0;
}
