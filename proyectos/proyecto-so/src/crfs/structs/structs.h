#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

//////////////////////////
//        Structs       //
//////////////////////////

typedef struct dir_parser
{
  unsigned char type;
  char* name;
  unsigned int index;
  int offset;
} Dir_parser;

/** Representa un bloque de datos */
typedef struct data
{
  uint32_t size;
} Data;

/** Representa un bloque de indice */
typedef struct index_block
{
  unsigned int size;
  unsigned int n_hardlinks;
  unsigned int* data_pointers;
  unsigned int* indirect_blocks;
} Index_block;

////////////////////////////////////
//        Public Functions        //
////////////////////////////////////

Data* data_init();

void data_destroy(Data* data);

Index_block* iblock_init(unsigned int size, unsigned int n_hardlinks, unsigned int * data_pointers, unsigned int * indirect_pointers);

void iblock_destroy(Index_block* iblock);

Dir_parser* dir_parser_init(unsigned char type, char* name, unsigned int index, int offset);

void dir_parser_destroy(Dir_parser* dir_parser);
