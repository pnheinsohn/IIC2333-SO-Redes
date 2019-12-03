#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "graph.h"
#include "structs.h"

extern int errno;
int errnum;

////////////////////////////
//          Inits         //
////////////////////////////

/** Inicializa un nodo, le asigna su nombre respectivo y el path
 * Importante recibir el path del padre... Para el root se recibe NULL
 */
Node* node_init(Dir_parser* dir_parser, char *parent_path)
{
  Node* node = malloc(sizeof(Node));

  node -> childs = malloc(sizeof(Node*));
  node -> count = 0;
  node -> type = dir_parser -> type;
  node -> name = dir_parser -> name;
  node -> next = NULL;

  if (!parent_path) {
    node -> path = malloc(sizeof(char) * (strlen(node->name) + 2));
    strcpy(node -> path, "/");
    strcat(node -> path, node -> name);
  } else {
    node -> path = malloc(sizeof(char) * (strlen(node->name) + strlen(parent_path) + 2));
    strcpy(node -> path, parent_path);
    strcat(node -> path, "/");
    strcat(node -> path, node -> name);
    node -> offset = dir_parser -> offset;
  }
  node -> index = dir_parser -> index;

  return node;
}

/** Inicializa un arbol vacio que sera la estructura que representa los directorios */
Graph* graph_init(unsigned char* bytemap)
{
  Graph* graph = malloc(sizeof(Graph));

  graph -> count = 0;
  graph -> root = NULL;
  graph -> bytemap = bytemap;

  return graph;
}

Queue* queue_init(Node* root)
{
  Queue* queue = malloc(sizeof(Queue));
  if (!queue) return NULL;

  queue -> head = root;
  queue -> tail = root;
  return queue;
}

////////////////////////////
//        Functions       //
////////////////////////////

/** Agrega un nodo (archivo o directorio) */
void graph_append(Graph* graph, Node* parent, Node* node)
{
  if ((node -> index == 0) & !graph -> root) {
    graph -> root = node;
  } else {
    parent -> childs[parent -> count] = node;
    parent -> count++;
    parent -> childs = (Node**) realloc(parent -> childs, sizeof(Node*) * (parent -> count + 1));
    node -> parent = parent;
  }

  // Sumo 1 al numero de nodos
  graph -> count++;
}

/** Retorna el primer elemento de la cola
 * NULL en caso de una cola vacia
 */
static Node* deque(Queue* queue)
{
  if (!queue -> head) return NULL;

  Node *node = queue -> head;
  queue -> head = node -> next;
  return node;
}

/** Agrega un nodo a una cola */
static void queue_append(Queue* queue, Node* node)
{
  if (!queue -> head) {
    queue -> head = node;
    queue -> tail = node;
  } else {
    queue -> tail -> next = node;
    queue -> tail = node;
  }
}

/** Busca en forma BFS un archivo o directorio y lo retorna */
Node *graph_search(Node* root, char* path)
{
  Node* actual;
  Queue* queue = queue_init(root);

  while (queue -> head) {
    actual = deque(queue);
    if (strcmp(actual -> path, path) == 0) {
      free(queue);
      return actual;
    }
    for (int i = 0; i < actual -> count; i++) queue_append(queue, actual -> childs[i]);
  }
  free(queue);
  return NULL;
}

////////////////////////////
//          Frees         //
////////////////////////////

/** Funcion que libera recursivamente la memoria de la lista ligada */
static void nodes_destroy(Node* node)
{
  if (node) {
    for (int i = 0; i < node -> count; i++) {
      nodes_destroy(node -> childs[i]);
    }
    free(node -> name);
    free(node -> path);
    free(node -> childs);
    free(node);
  }
}

/** Funcion que destruye la lista ligada liberando la memoria utilizada */
void graph_destroy(Graph* graph)
{
  nodes_destroy(graph -> root);
  free(graph -> bytemap);
  free(graph);
}

////////////////////////////////
//          Printers          //
////////////////////////////////

/** Imprime los nodos para aludir un arbol */
static void node_printer(Node *node, int depth)
{
  if (node) {
    for (int i = 0; i < node -> count; i++) {
      for (int j = 0; j < depth; j++) {
        printf("--");
      }
      printf("> ");
      printf("%s\n", node -> childs[i] -> path);
      node_printer(node -> childs[i], depth + 1);
    }
  }
}

/** imprime arbol de directorios */
void graph_printer(Graph* graph)
{
  int counter = 1;
  printf("%s\n", graph -> root -> path);
  node_printer(graph->root, counter);
}
