#ifndef _BOARD_H
#define _BOARD_H

#include "global.h"
#include "move.h"
/**********************************************************/

/* Position struct to store board, score and player's turn */
typedef struct
{
	char board[ ARRAY_BOARD_SIZE ][ ARRAY_BOARD_SIZE ];
	int score[ 2 ];
	char turn;												//stores the color of the player that has the turn
} Position;


/**********************************************************/
void initPosition( Position * pos );
//initializes position

void printBoard( char board[ ARRAY_BOARD_SIZE ][ ARRAY_BOARD_SIZE ] );
//prints board

void printPosition( Position * pos );
// Prints board along with Player's turn and score

int doOneDirection( Position * pos, Move * moveToPlay, int do_move, signed char inc_row, signed char inc_col );
//flips pieces across a single direction

int doAllDirections( Position * pos, Move * moveToPlay, int do_move );
//flips pieces across all "legal" directions or checks legality. Its functionality depends on do_moves' value
//if do_move is TRUE then doAllDirections == doMove, if do_move is FALSE then we check legality

void doMove( Position * pos, Move * moveToDo );
//playes the move on position

int isLegalMove( Position * pos, Move * moveToCheck );
//checks if a move is legal

int isLegal( Position * pos, short int row, short int col, char color);
//same as isLegalMove() but without move struct

int canMove( Position * pos, char color);
//checks if player (color) can move on that specific position.

#endif
