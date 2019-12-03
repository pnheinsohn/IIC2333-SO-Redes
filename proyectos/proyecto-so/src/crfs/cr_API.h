#pragma once
#include "structs/structs.h"

//////////////////////////
//        Structs       //
//////////////////////////

/** Representa un archivo abierto */
typedef struct FILE {
    char *path;
    unsigned char mode;
    Index_block *iblock;
    Dir_parser *directory;
    unsigned int amount_read;
    unsigned int actual_offset;
    unsigned int *data_pointers;
    unsigned int actual_data_index;
} crFILE;

////////////////////////////////////
//        Public Functions        //
////////////////////////////////////

/** Monta el disco */
void cr_mount(char* diskname);

/** Printea el bitmap, la cantidad de 1's y 0's */
void cr_bitmap(void);

int cr_exists(char* path);

void cr_ls(char* path);

int cr_mkdir(char *foldername);

crFILE* cr_open(char* path, char mode);

int cr_read(crFILE* file_desc, void* buffer, int nbytes);

int cr_write(crFILE* file_desc, void* buffer, int nbytes);

int cr_close(crFILE* file_desc);

int cr_rm(char* path);

int cr_hardlink(char* orig, char* dest);

int cr_unload(char* orig, char* dest);

int cr_load(char* orig);

void crFILE_printer(crFILE *cr_file);

void crFILE_destroy(crFILE *cr_file);
