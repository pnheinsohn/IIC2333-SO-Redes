#pragma once

#include <stdbool.h>

typedef enum direction {
  ESI,
  ESD,
  EII,
  EID,
} Direction;

typedef struct board
{
  int n_white;
  int n_black;
  int no_kills;
  char cells[8][8];
} Board;

typedef struct game
{
  int score1;
  int score2;
  Board *actualBoard;
} Game;

// Inicializa el juego con el tablero inicial
Game *game_init(void);

// Incializa el tablero
Board *board_init(void);

// Imprime el board
void board_printer(Board *board);

/** Revisa si el movimiento del usuario es valido
 * En caso de ser valido retorna true y actualiza el board
 * En caso contrario retorna false y deja el board como estaba
 */
bool check_movement(Board *board, char origin[], char destiny[], int turn);

// Server recibe el tablero en formato char[] para actualizar el board
void board_update(Board *board, char charBoard[]);

// Pasa el tablero a formato char[]
char *board_to_char(Board *board, char *charBoard);

// Pasa un char[] a formato Board
Board* char_to_board(char *charBoard);

// Retorna al ganador o empate o nulo si no hay juego o se esta jugando
int get_winner(Board *board);

// Retorna true si el juego acabo
bool game_ended(Board *board);

// Libera la memoria asignada al board
void board_destroy(Board *board);

// Libera la memoria asignada al game
void game_destroy(Game *game);
