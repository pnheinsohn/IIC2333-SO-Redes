#pragma once

#include "../structs/graph.h"

////////////////////////////////////
//        Public Functions        //
////////////////////////////////////

/** Convierte un Byte a Bits, retorna la cantidad de 1's
 * Printea el número binario: Necesario para cr_bitmap()
 */
int byteToBits(unsigned char byte);

Graph *load_disk(void);

/** Recorta un string insertando un caracter de termino
 * Obtenido de: https://stackoverflow.com/questions/27414696/remove-last-four-characters-from-a-string-in-c
*/
void trim_end(char *str, int n);

int next_free_block(unsigned char *bytemap);

/** Busca la siguiente entrada valida en un bloque de directorio */
int next_free_entry(unsigned int index);

/** Escribe un bloque de directorio en el disco */
void write_dir_block(unsigned int index, Dir_parser* dir);

/** Escribe en el bitmap el estado de un bloque dado */
void write_bitmap(unsigned int index, unsigned int value);

/** Escribe un byte en el bloque index y el offset dado */
void write_byte(unsigned int index, int offset, unsigned char value);

/** Escribe 4 bytes en el bloque index y el offset dado */
void write_4bytes(unsigned int index, int offset, unsigned int value);

/** Lee y retorna un bloque indice */
Index_block *read_index_block(unsigned int index);

/** Escribe en un bloque indice */
void write_index_block(unsigned int index, Index_block *iblock, unsigned int offset);

/** Reads nbytes from index and offset statements and saves it in buffer */
void read_file_to_buffer(unsigned int nbytes, crFILE *file_desc, void *buffer);

/** Lee 512 bloques de datos para archivos */
unsigned int *read_data_block(unsigned int index);

/** Escribe un archivo o directorio recursivamente */
void write_file(char * path, Node *file);