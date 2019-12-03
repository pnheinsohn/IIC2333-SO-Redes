#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../structs/graph.h"
#include "../structs/structs.h"
#include "../cr_API.h"
#include "functions.h"

unsigned int BLOCK_SIZE = 2048;

extern int errno;
int errnum;

extern char *DISK_PATH;

/** Convierte un Byte a Bits, retorna la cantidad de 1's
 * Printea el número binario: Necesario para cr_bitmap()
 */
int byteToBits(unsigned char byte)
{
	// Cuenta la cantidad de 1's
	int ones_counter = 0;

	int aux;
	int counter = 0;
	char *binary_number = malloc(sizeof(unsigned char) * 9);
	for (int bit = (sizeof(unsigned char) * 7); bit >= 0; bit--) {
		aux = byte >> bit;
		if (aux & 1) {
			binary_number[counter] = 1 + '0';
			ones_counter++;
		}
		else binary_number[counter] = 0 + '0';
		counter++;
	}
	binary_number[counter] = '\0';
	printf("%s", binary_number);
	free(binary_number);
	return ones_counter;
}

/** true si el bloque es valido en el bitmap
 * Aun no esta en uso, pero es mejor dejarla por si acaso
 */
bool valid(int block_index, unsigned char *bytemap)
{
	unsigned char byte = bytemap[block_index / 8];
	int position = 7 - block_index % 8;

	return (byte >> position) & 0x1;
}

/** Carga el bitmap en bytes */
static unsigned char *load_bitmap(void)
{
	FILE *file = fopen(DISK_PATH, "rb");
	fseek(file, BLOCK_SIZE, SEEK_SET);
	unsigned char *bytemap = malloc(sizeof(unsigned char) * BLOCK_SIZE * 4);
	fread(bytemap, sizeof(unsigned char), BLOCK_SIZE * 4, file);
	fclose(file);
	return bytemap;
}

/** Encargado de leer una entrada de un bloque y verificar el tipo de entrada */
static Dir_parser *read_entry(unsigned char *buffer, unsigned char *bytemap, int offset)
{
	unsigned char type = buffer[0];
	if ((type != (unsigned char) 2) && (type != (unsigned char) 4)) return NULL;

	char *aux_name = (char *) (buffer + 1);
	char *name = malloc(sizeof(char) * (strlen(aux_name) + 1));
	strcpy(name, aux_name);

	unsigned int index = (unsigned int)buffer[30] * 256 + (unsigned int)buffer[31];

	// printf("Reading Entry N° %u, %s, %i\n", type, name, index);
	// if (type == (unsigned char) 2) printf("DIR %s index: %u\n", name, index);
	// else if (type == (unsigned char) 4) printf("FILE %s index: %u\n", name, index);

	return dir_parser_init(type, name, index, offset);
}

/** Lee un bloque de directorio en el indice index
 * Retorna un array de Dir_parser que representan los archivos en este directorio
 */
static Dir_parser **read_dir_block(unsigned int index, unsigned char *bytemap)
{
	// printf("\n---------------------\nREADING BLOCK %i\n", index);
	FILE *file = fopen(DISK_PATH, "rb");

	unsigned char *buffer = malloc(sizeof(unsigned char) * 32);

	Dir_parser **dir_parser = malloc(sizeof(Dir_parser *) * 64);

	// Lee un bloque completo
	for (int i = 0; i < 64; i++) {
		fseek(file, (32 * i) + ((unsigned int) ((index) * BLOCK_SIZE)), SEEK_SET);
		fread(buffer, sizeof(unsigned char), 32, file);
		dir_parser[i] = read_entry(buffer, bytemap, i * 32);
	};

	free(buffer);
	fclose(file);
	return dir_parser;
}

/** Escribe un bloque de directorio en el disco */
void write_dir_block(unsigned int index, Dir_parser *dir)
{
	int offset = next_free_entry(index);
	if (offset == -1) return;
	dir -> offset = offset;

	unsigned char zero = '\0';
	unsigned char high = (unsigned char)(dir -> index >> 8);
	unsigned char low = dir -> index & 0xff;

	FILE *file = fopen(DISK_PATH, "rb+");
	fseek(file, (unsigned int)((BLOCK_SIZE * index) + offset), SEEK_SET);
	fwrite(&dir -> type, sizeof(unsigned char), 1, file);
	fwrite(dir -> name, sizeof(unsigned char) * strlen(dir -> name), 1, file);
	for (int i = 0; i < 29 - strlen(dir -> name); i++) fwrite(&zero, sizeof(unsigned char), 1, file);
	fwrite(&high, sizeof(unsigned char), 1, file);
	fwrite(&low, sizeof(unsigned char), 1, file);
	fclose(file);
}

/** Lee los directorios de manera recursiva DFS */
static void load_dir(Graph *graph, Node *parent)
{
	Dir_parser **dir_entries;
	// Primera llamada crea al root
	if (!parent)
	{
		char *root_name = malloc(sizeof(char) * 5);
		strcpy(root_name, "root");
		dir_entries = read_dir_block(0, graph->bytemap);
		Dir_parser *root_entry = dir_parser_init((unsigned char) 2, root_name, 0, 0);
		parent = node_init(root_entry, NULL);
		dir_parser_destroy(root_entry);
		// Agrego el nodo raiz
		graph_append(graph, NULL, parent);
	}
	// Llamadas restantes se llaman con un directorio padre
	else dir_entries = read_dir_block(parent->index, graph->bytemap);

	Dir_parser *entry;
	// Recorremos todas las entradas de ese directorio
	for (int i = 0; i < 64; i++)
	{
		// Entrada invalida
		entry = dir_entries[i];
		if (entry == NULL) continue;

		// Creo un nuevo nodo
		Node *node = node_init(dir_entries[i], parent -> path);
		dir_parser_destroy(dir_entries[i]);

		// Llamado recursivo solo si es directorio y no es el padre
		if (node -> type == (unsigned char) 2) load_dir(graph, node);
		// Agregamos la entrada al directorio padre
		if (parent) graph_append(graph, parent, node);
	};
	free(dir_entries);
}

/** Arma el arbol de directorios y lo retorna */
Graph *load_disk(void)
{
	// Cargamos el bitmap
	unsigned char *bytemap = load_bitmap();
	// Creamos el arbol de directorios
	Graph *graph = graph_init(bytemap);
	// Cargamos los directorios al arbol
	load_dir(graph, NULL);
	return graph;
}

/** Recorta un string insertando un caracter de termino
 * Obtenido de: https://stackoverflow.com/questions/27414696/remove-last-four-characters-from-a-string-in-c
*/
void trim_end(char *str, int n)
{
	n = strlen(str) - n;

	if (n < 0) n = 0;

	str[n] = '\0';
}

/** Busca el siguiente bloque libre en el bitmap */
int next_free_block(unsigned char *bytemap)
{
	for (unsigned int index = 0; index < 8192; index++) {
		int aux;
		for (int bit = (sizeof(unsigned char) * 7); bit >= 0; bit--) {
			aux = bytemap[index] >> bit;
			if (aux & 1) continue;
			else return (index * 8) + 7 - bit;
		}
	}

	// No queda espacio en disco
	errnum = ENOSPC;
	fprintf(stderr, "Error writing to disk: %s\n", strerror(errnum));
	return -1;
}

/** Busca la siguiente entrada valida en un bloque de directorio */
int next_free_entry(unsigned int index)
{
	FILE *file = fopen(DISK_PATH, "rb");
	unsigned char *buffer = malloc(sizeof(unsigned char) * 32);
	int offset = -1;
	unsigned char type;
	// Lee un bloque completo
	for (int i = 0; i < 64; i++) {
		fseek(file, (32 * i) + ((unsigned int)((index)*BLOCK_SIZE)), SEEK_SET);
		fread(buffer, sizeof(unsigned char), 32, file);
		type = buffer[0];

		// Si es un bloque invalido lo guardo
		if (type != (unsigned char) 2 && type != (unsigned char) 4) {
			offset = i * 32;
			break;
		}
	};

	// No encontro ningun bloque libre
	if (offset == -1) fprintf(stderr, "Error: No space extra in directory\n");

	free(buffer);
	fclose(file);

	return offset;
}

void write_bitmap(unsigned int index, unsigned int value)
{
	unsigned char *byte = malloc(sizeof(unsigned char));
	int offset = 7 - index % 8;

	FILE *file = fopen(DISK_PATH, "rb+");
	fseek(file, BLOCK_SIZE + index / 8, SEEK_SET);
	fread(byte, sizeof(unsigned char), 1, file);

	if (value == 1) byte[0] |= (1u << offset);
	else byte[0] &= ~(1u << offset);

	fseek(file, BLOCK_SIZE + index / 8, SEEK_SET);
	fwrite(byte, sizeof(unsigned char), 1, file);

	free(byte);
	fclose(file);
}

/** Escribe un byte en el bloque index y el offset dado */
void write_byte(unsigned int index, int offset, unsigned char value)
{
	unsigned char *byte = malloc(sizeof(unsigned char));
	byte[0] = value;

	FILE *file = fopen(DISK_PATH, "rb+");
	fseek(file, (unsigned int) ((BLOCK_SIZE * index) + offset), SEEK_SET);

	fwrite(byte, sizeof(unsigned char), 1, file);

	free(byte);
	fclose(file);
}

/** Escribe 4 bytes en el bloque index y el offset dado */
void write_4bytes(unsigned int index, int offset, unsigned int value)
{
	unsigned char bytes[4];
	bytes[0] = (value >> 24) & 0xFF;
	bytes[1] = (value >> 16) & 0xFF;
	bytes[2] = (value >> 8) & 0xFF;
	bytes[3] = value & 0xFF;

	FILE *file = fopen(DISK_PATH, "rb+");
	fseek(file, (unsigned int) ((BLOCK_SIZE * index) + offset), SEEK_SET);

	fwrite(bytes, sizeof(unsigned char), 4, file);

	fclose(file);
}

/** Lee y retorna un bloque indice */
Index_block *read_index_block(unsigned int index)
{
	FILE *file = fopen(DISK_PATH, "rb");

	unsigned char *buffer = malloc(sizeof(unsigned char) * 2048);

	fseek(file, (unsigned int)(index * BLOCK_SIZE), SEEK_SET);
	fread(buffer, sizeof(unsigned char), 2048, file);

	unsigned int p0 = (unsigned int) pow(2, 32);
	unsigned int p1 = (unsigned int) pow(2, 16);
	unsigned int p2 = (unsigned int) pow(2, 8);

	unsigned int size = (unsigned int)buffer[0] * p0 + (unsigned int)buffer[1] * p1 + (unsigned int)buffer[2] * p2 + (unsigned int)buffer[3];
	unsigned int n_hardlinks = (unsigned int)buffer[4] * p0 + (unsigned int)buffer[5] * p1 + (unsigned int)buffer[6] * p2 + (unsigned int)buffer[7];

	unsigned int *data_pointers = calloc(500, sizeof(unsigned int));
	unsigned int *indirect_blocks = calloc(10, sizeof(unsigned int));
	unsigned int ptr;

	// Lee los 500 punteros de direccionamiento directo
	for (int i = 8; i < 2008; i += 4) {
		ptr = (unsigned int) buffer[i + 2] * p2 + (unsigned int) buffer[i + 3];
		data_pointers[(i - 8) / 4] = ptr;
	};

	// Lee los 10 punteros de direccionamiento indirecto
	for (int i = 2008; i < 2048; i += 4) {
		ptr = (unsigned int) buffer[i + 2] * p2 + (unsigned int) buffer[i + 3];
		indirect_blocks[(i - 2008) / 4] = ptr;
	};

	Index_block *index_block = iblock_init(size, n_hardlinks, data_pointers, indirect_blocks);

	free(buffer);
	fclose(file);

	return index_block;
}

/** Escribe en un bloque indice */
void write_index_block(unsigned int index, Index_block *iblock, unsigned int offset)
{
	write_4bytes(index, offset, iblock -> size);
	write_4bytes(index, offset + 4, iblock -> n_hardlinks);

	unsigned char *buffer = calloc(2040, sizeof(unsigned char));

	for (int i = 8, j = 0; i < 2008; i += 4, j++) write_4bytes(index, offset + i, iblock->data_pointers[j]);
	for (int i = 2008, j = 0; i < 2040; i += 4, j++) write_4bytes(index, offset, iblock->indirect_blocks[j]);
	free(buffer);
}

/** Reads nbytes from index and offset statements and saves it in buffer */
void read_file_to_buffer(unsigned int nbytes, crFILE *cr_file, void *buffer)
{
	FILE *file = fopen(DISK_PATH, "r");
	fflush(stdout);
	buffer += cr_file -> amount_read;
	int aux = 0;
	for (int i = cr_file -> actual_data_index; i < 5620; i++) {
		fseek(file, (unsigned int) ((BLOCK_SIZE * cr_file -> data_pointers[cr_file -> actual_data_index]) + cr_file -> actual_offset), SEEK_SET);
		while (aux < nbytes && cr_file -> amount_read < cr_file -> iblock -> size &&
				cr_file -> actual_offset < 2048) {
			fread(buffer, sizeof(char), 1, file);
			cr_file -> actual_offset++;
			cr_file -> amount_read++;
			buffer++;
			aux++;
		}
		if (cr_file -> actual_offset == 2048) {
			cr_file -> actual_offset = 0;
			cr_file -> actual_data_index++;
		}
		if (cr_file -> amount_read == nbytes || cr_file -> amount_read == cr_file -> iblock -> size) break;
	}
	fclose(file);
}

/** Lee 512 bloques de datos para archivos */
unsigned int *read_data_block(unsigned int index)
{
	FILE *file = fopen(DISK_PATH, "rb");

	unsigned char *buffer = malloc(sizeof(unsigned char) * 2048);

	fseek(file, (unsigned int) (index * BLOCK_SIZE), SEEK_SET);
	fread(buffer, sizeof(unsigned char), 2048, file);

	unsigned int p2 = (unsigned int) pow(2, 8);

	unsigned int *data_pointers = calloc(512, sizeof(unsigned int));
	unsigned int ptr;

	// Lee los 500 punteros de direccionamiento directo
	for (int i = 0; i < 2048; i += 4) {
		ptr = (unsigned int) buffer[i + 2] * p2 + (unsigned int) buffer[i + 3];
		data_pointers[i/ 4] = ptr;
	};

	free(buffer);
	fclose(file);

	return data_pointers;
}

/** Escribe un archivo o directorio en nuestro computador */
void write_file(char * path, Node *file) {
  char *new_path = malloc(1000 * sizeof(char));
	strcpy(new_path, path);
	strcat(new_path, file -> name);

  // Si es un directorio hago un llamado recursivo de los hijos
  if (file -> type == (unsigned char) 2) {
		// Creo un subdirectorio
		mkdir(new_path, 0777);
		strcat(new_path, "/");
		// Llamado recursivo a cada hijo
		for (int i = 0; i < file -> count; i++) {
			write_file(new_path, file -> childs[i]);
    }
		free(new_path);
  } // Si es un archivo lo copio al disco
	else if (file -> type == (unsigned char) 4) {
		crFILE *cr_file = cr_open(file -> path, 'r');

		int nbytes = cr_file -> iblock -> size;
		char* buffer = calloc(nbytes, sizeof(char));

		// Leo el archivo completo
		cr_read(cr_file, buffer, nbytes);

		// Archivo de salida
		FILE * output = fopen(new_path, "wb");

		fwrite(buffer, sizeof(char), nbytes, output);
		cr_close(cr_file);
		fclose(output);
		free(buffer);
		free(new_path);
	}
}