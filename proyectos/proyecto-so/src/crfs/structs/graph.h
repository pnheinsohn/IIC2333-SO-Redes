// Estructuras obtenidas del taller 0 de EDD
#pragma once

#include "structs.h"
#include <string.h>


//////////////////////////
//        Structs       //
//////////////////////////

/** Estructura de un nodo del grafo. Mantiene una referencia al nodo
siguiente y al nodo anterior ademas de mantener un numero */
typedef struct node
{
  // En este caso no podemos referirnos a la estructura como Node ya que aun
  // no esta completo el typedef

  // Lista de punteros de hijos
  struct node** childs;
  struct node* parent;
  char* name;
  char* path;
  // For BFS in case of needed: Never use free(node -> next);
  struct node* next;
  unsigned char type;
  unsigned int index;
  int count;
  int offset;
} Node;

/** Estructura de un grafo. Referencia a los extremos y mantiene un
contador de nodos en la lista */
typedef struct graph
{
  // Nodo inicial del grafo
  Node* root;
  // Contador de elementos del grafo
  int count;
  unsigned char* bytemap;
} Graph;

typedef struct queue {
  Node *head;
  Node *tail;
} Queue;

////////////////////////////////////
//        Public Functions        //
////////////////////////////////////

/** Inicializa un arbol vacio que sera la estructura que representa los directorios */
Graph* graph_init(unsigned char* bytemap);

/** Inicializa un nodo, le asigna su nombre respectivo y el path
 * Importante recibir el path del padre... Para el root se recibe NULL
 */
Node* node_init(Dir_parser* dir_parser, char *parent_path);

Queue* queue_init(Node* root);

/** Agrega un nodo (archivo o directorio) */
void graph_append(Graph* graph, Node* parent, Node* node);

/** Busca en forma BFS un archivo o directorio y lo retorna
 * Retorna NULL si no lo encuentra
*/
Node* graph_search(Node* root, char* path);

/** Funcion que destruyel grafo liberando la memoria utilizada */
void graph_destroy(Graph* graph);

/** imprime arbol de directorios. No implementada todavia*/
void graph_printer(Graph* graph);
