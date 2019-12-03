#include "structs.h"

Data* data_init()
{
  Data* data = malloc(sizeof(Data));
  return data;
}

void data_destroy(Data* data)
{
  free(data);
}

Index_block* iblock_init(unsigned int size, unsigned int n_hardlinks, unsigned int * data_pointers, unsigned int * indirect_blocks)
{
  Index_block* iblock = malloc(sizeof(Index_block));
  iblock -> size = size;
  iblock -> n_hardlinks = n_hardlinks;
  
  if (data_pointers) iblock -> data_pointers = data_pointers;
  else iblock -> data_pointers = calloc(500, sizeof(unsigned int));
  
  if (indirect_blocks) iblock -> indirect_blocks = indirect_blocks;
  else iblock -> indirect_blocks = calloc(10, sizeof(unsigned int));

  return iblock;
}

void iblock_destroy(Index_block* iblock)
{
  free(iblock -> data_pointers);
  free(iblock -> indirect_blocks);
  free(iblock);
}

/** Inicializa un Dir Parser
 * Su funcionalidad esta mencionada en la definicion de la estructura
 */
Dir_parser* dir_parser_init(unsigned char type, char* name, unsigned int index, int offset)
{
  Dir_parser* dir_parser = malloc(sizeof(Dir_parser));

  dir_parser -> type = type;
  dir_parser -> name = name;
  dir_parser -> index = index;
  dir_parser -> offset = offset;

  return dir_parser;
}

void dir_parser_destroy(Dir_parser* dir_parser)
{
  free(dir_parser);
}
