#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"

//////////////////////////////
//          Inits           //
//////////////////////////////

// Incializa el tablero
Board *board_init(void) {
  Board *new = malloc(sizeof(Board));
  new->n_white = 12;
  new->n_black = 12;
  new->no_kills = 0;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if ((i % 2 == 0 && j % 2 != 0) || (i % 2 != 0 && j % 2 == 0)) {
        if (i < 3) new->cells[i][j] = 'x';
        else if (i > 4) new->cells[i][j] = 'o';
        else new->cells[i][j] = 'b';
      }
      else new->cells[i][j] = 'n';
    }
  }
  return new;
}

// Inicializa el juego con el tablero inicial
Game *game_init(void) {
  Game *new = malloc(sizeof(Game));
  new->actualBoard = board_init();
  new->score1 = 0;
  new->score2 = 0;
  return new;
}

//////////////////////////
//      Functions       //
//////////////////////////

// Revisa la sintaxis del input del usuario para ver su validez
static void parse_input(char origin[], char destiny[], int coordenates[]) {
  coordenates[0] = (int) origin[1] - '0';
  coordenates[2] = (int) destiny[1] - '0';
  if (origin[0] == 'A') coordenates[1] = 0;
  if (origin[0] == 'B') coordenates[1] = 1;
  if (origin[0] == 'C') coordenates[1] = 2;
  if (origin[0] == 'D') coordenates[1] = 3;
  if (origin[0] == 'E') coordenates[1] = 4;
  if (origin[0] == 'F') coordenates[1] = 5;
  if (origin[0] == 'G') coordenates[1] = 6;
  if (origin[0] == 'H') coordenates[1] = 7;
  if (destiny[0] == 'A') coordenates[3] = 0;
  if (destiny[0] == 'B') coordenates[3] = 1;
  if (destiny[0] == 'C') coordenates[3] = 2;
  if (destiny[0] == 'D') coordenates[3] = 3;
  if (destiny[0] == 'E') coordenates[3] = 4;
  if (destiny[0] == 'F') coordenates[3] = 5;
  if (destiny[0] == 'G') coordenates[3] = 6;
  if (destiny[0] == 'H') coordenates[3] = 7;
}

// Server recibe el tablero en formato char[] para actualizar el board
void board_update(Board *board, char charBoard[]) {
  board->n_white = 0;
  board->n_black = 0;
  for (int i = 0; i < 64; i++) {
    board->cells[i / 8][i % 8] = charBoard[i];
    if (charBoard[i] == 'x' || charBoard[i] == 'X') board->n_black++;
    if (charBoard[i] == 'o' || charBoard[i] == 'O') board->n_white++;
  }
}

// Retorna al ganador o empate o nulo si no hay juego o se esta jugando
int get_winner(Board *board) {
  if (board->no_kills == 10) return 0;
  if (board->n_black == 0) return 1;
  if (board->n_black == 0) return 2;
  else return -1;
}

// Retorna true si el juego acabo
bool game_ended(Board *board) {
  if (board->n_black == 0) return true;
  if (board->n_white == 0) return true;
  if (board->no_kills == 10) return true;
  return false;
}

// Pasa el tablero a formato char[]
char *board_to_char(Board *board, char *charBoard) {
  for (int i = 0; i < 64; i++) charBoard[i] = board -> cells[i / 8][i % 8];
  charBoard[64] = '\0';
  return charBoard;
}

Board* char_to_board(char *charBoard) {
  Board *board = malloc(sizeof(Board));
  for (int i = 0; i < 64; i++) board -> cells[i / 8][i % 8] = charBoard[i + 2];
  return board;
}

// Revisa si una celda esta vacia
static bool is_empty(Board *board, int row, int col) {
  if (board->cells[row][col] == 'b') return true;
  return false;
}

// Revisa la validez de un movimiento del backtracking
static bool is_valid(Board *board, int row_a, int col_a, char cell, Direction dir) {
  // Dentro del tablero
  if (col_a < 0 || col_a > 7 || row_a < 0 || row_a > 7) return false;

  // Revisa los movimientos
  if (cell == 'x' && (dir != EII && dir != EID)) return false;
  if (cell == 'o' && (dir != ESI && dir != ESD)) return false;

  /** ESI || ESD || EII || EID
   * Reviso que el destino este en el tablero y sea vacio
   * Reviso que hay un oponente entre medio de la pos actual y destino
   */
  if (dir == ESI && row_a - 2 >= 0 && col_a - 2 >= 0 && is_empty(board, row_a - 2, col_a - 2) &&
    board->cells[row_a - 1][col_a - 1] != cell && board->cells[row_a - 1][col_a - 1] != 'b') return true;

  if (dir == ESD && row_a - 2 >= 0 && col_a + 2 <= 7 && is_empty(board, row_a - 2, col_a + 2) &&
    board->cells[row_a - 1][col_a + 1] != cell && board->cells[row_a - 1][col_a + 1] != 'b') return true;

  if (dir == EII && row_a + 2 <= 7 && col_a - 2 >= 0 && is_empty(board, row_a + 2, col_a - 2) &&
    board->cells[row_a + 1][col_a - 1] != cell && board->cells[row_a + 1][col_a - 1] != 'b') return true;

  if (dir == EID && row_a + 2 <= 7 && col_a + 2 <=7 && is_empty(board, row_a + 2, col_a + 2) &&
    board->cells[row_a + 1][col_a + 1] != cell && board->cells[row_a + 1][col_a + 1] != 'b') return true;

  return false;
}

/** Revisa los movimientos del usuario en caso de una o multiples capturas
 * En caso de un movimiento valido retorna 1
 * En caso contrario retorna 0
 */
static int backtracking(Board *board, int row_a, int col_a, int row_f, int col_f, char cell, int level) {
  if (col_a == col_f && row_a == row_f) return 1;
  Direction dirs[4] = {ESI, ESD, EII, EID};
  for (int i = 0; i < 4; i++) {
    // Reviso la validez del movimiento
    if (!is_valid(board, row_a, col_a, cell, dirs[i])) continue;
    // Clave dejar estas variables en la funcion para mantenerlos en el stack y simplificar el 'UNDO'
    char killed_cell;
    char new_cell = cell;
    int prev_no_kills = board->no_kills;
    int new_row, new_col, killed_row, killed_col;
    if (dirs[i] == ESI) {
      killed_cell = board->cells[row_a - 1][col_a - 1];
      killed_row = row_a - 1;
      killed_col = col_a - 1;
      new_row = row_a - 2;
      new_col = col_a - 2;
    } else if (dirs[i] == ESD) {
      killed_cell = board->cells[row_a - 1][col_a + 1];
      killed_row = row_a - 1;
      killed_col = col_a + 1;
      new_row = row_a - 2;
      new_col = col_a + 2;
    } else if (dirs[i] == EII) {
      killed_cell = board->cells[row_a + 1][col_a - 1];
      killed_row = row_a + 1;
      killed_col = col_a - 1;
      new_row = row_a + 2;
      new_col = col_a - 2;
    } else {  // EID
      killed_cell = board->cells[row_a + 1][col_a + 1];
      killed_row = row_a + 1;
      killed_col = col_a + 1;
      new_row = row_a + 2;
      new_col = col_a + 2;
    }
    /** 'DO': Hago los cambios dado el movimiento */
    // Actualizo la cantidad de no kills
    board->no_kills = 0;
    // Resto las cantidades de fichas
    if (killed_cell == 'x' || killed_cell == 'X') board->n_black--;
    else if (killed_cell == 'o' || killed_cell == 'O') board->n_white--;
    // Convierto en reina en caso de cumplir las condiciones
    if (new_row == 7 && cell == 'x') new_cell = 'X';
    else if (new_row == 0 && cell == 'o') new_cell = 'O';
    // Actualizo las celdas del board
    board->cells[row_a][col_a] = 'b';
    board->cells[killed_row][killed_col] = 'b';
    board->cells[new_row][new_col] = new_cell;
    // Retorno 'verdadero' si el movimiento es valido
    if (backtracking(board, new_row, new_col, row_f, col_f, new_cell, level + 1)) return 1;
    /** 'UNDO': Revierto los cambios hechos en caso de ser invalido */
    // Revierto la cantidad de no kills
    board->no_kills = prev_no_kills;
    // Aumento las cantidades de fichas
    if (killed_cell == 'x' || killed_cell == 'X') board->n_black++;
    else if (killed_cell == 'o' || killed_cell == 'O') board->n_white++;
    // Revierto los cambios en las celdas del board
    board->cells[row_a][col_a] = cell;
    board->cells[killed_row][killed_col] = killed_cell;
    board->cells[new_row][new_cell] = 'b';
  }
  return 0;
}

/** Revisa si el movimiento del usuario es valido
 * En caso de ser valido retorna true y actualiza el board
 * En caso contrario retorna false y deja el board como estaba
 */
bool check_movement(Board *board, char origin[], char destiny[], int turn) {
  if (origin[0] != 'A' && origin[0] != 'B' && origin[0] != 'C' && origin[0] != 'D' && origin[0] != 'E' && origin[0] != 'F' && origin[0] != 'G' && origin[0] != 'H') return false;
  if (destiny[0] != 'A' && destiny[0] != 'B' && destiny[0] != 'C' && destiny[0] != 'D' && destiny[0] != 'E' && destiny[0] != 'F' && destiny[0] != 'G' && destiny[0] != 'H') return false;

  int coordenates[4];
  parse_input(origin, destiny, coordenates);
  int row_o = coordenates[0];
  int col_o = coordenates[1];
  int row_f = coordenates[2];
  int col_f = coordenates[3];

  // Inputs dentro del tablero
  if (row_o < 0 || row_o > 7 || col_o < 0 || col_o > 7 || row_f < 0 || row_f > 7 || col_f < 0 || col_f > 7) return false;
  char cell = board->cells[row_o][col_o];
  // Ficha a mover existe en esa posicion
  if (turn == 1 && (cell != 'o' && cell != 'O')) return false;
  if (turn == 2 && (cell != 'x' && cell != 'X')) return false;
  // Celda final esta vacia
  if (!is_empty(board, row_f, col_f)) return false;
  // Caso 1: Moverse sin captura
  if (abs(row_o - row_f) == 1 && abs(col_o - col_f) == 1) {
    char new_cell = cell;
    bool valid = false;
    // Reinas
    if (cell == 'X' || cell == 'O') valid = true;
    // Ficha negra
    else if (cell == 'x' && row_o < row_f) {
      valid = true;
      if (row_f == 7) new_cell = 'X';
    }
    // Ficha blanca
    else if (cell == 'o' && row_o > row_f) {
      valid = true;
      if (row_f == 0) new_cell = 'O';
    }
    if (valid) {
      board->cells[row_o][col_o] = 'b';
      board->cells[row_f][col_f] = new_cell;
      board->no_kills++;
      return true;
    }
    return false;
  }
  // Caso 2: Moverse con captura
  return backtracking(board, row_o, col_o, row_f, col_f, cell, 0);
}

////////////////////////////
//        Printers        //
////////////////////////////

// Imprime el board
void board_printer(Board *board) {
  // Fuente Consola: NSlimSun Tamaño 14
  puts("\n      A |  B |  C |  D |  E |  F |  G |  H");
  puts("   .---------------------------------------.");
  for (int i = 0; i < 8; i++) {
    printf(" %i ", i);
    for (int j = 0; j < 8; j++) {
      if (board->cells[i][j] == 'b') {
        if (j == 7) printf("| \u25A0 | %i\n", i);
        else printf("| \u25A0 ");
      } else if (board->cells[i][j] == 'n') {
        if (j == 7) printf("| \u25A1 | %i\n", i);
        else printf("| \u25A1 ");
      } else if (board->cells[i][j] == 'o') {
        if (j == 7) printf("| \u25C9 | %i\n", i);
        else printf("| \u25C9 ");
      } else if (board->cells[i][j] == 'O') {
        if (j == 7) printf("| \u265B | %i\n", i);
        else printf("| \u265B ");
      } else if (board->cells[i][j] == 'x') {
        if (j == 7) printf("| \u25CE | %i\n", i);
        else printf("| \u25CE ");
      } else if (board->cells[i][j] == 'X') {
        if (j == 7) printf("| \u2655 | %i\n", i);
        else printf("| \u2655 ");
      }
    }
    if (i != 7) {
      for (int j = 0; j < 11; j++) printf("----");
      printf("---\n");
    }
  }
  puts("   '---------------------------------------'");
  puts("\n      A |  B |  C |  D |  E |  F |  G |  H");

  // printf("\nWhites: %i | Blacks: %i\n", board->n_white, board->n_black);
}

//////////////////////////////
//          Frees           //
//////////////////////////////

// Libera la memoria asignada al board
void board_destroy(Board *board) {
  if (board) free(board);
}

// Libera la memoria asignada al game
void game_destroy(Game *game) {
  board_destroy(game->actualBoard);
  free(game);
}
