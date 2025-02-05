#ifndef _CLIENT_H
#define _CLIENT_H

#include "global.h"
#include "move.h"
#include "board.h"

/**********************************************************/

#define MAX_DEPTH 5		// Max fixed depth used for simple cutoff function

int isCutoff( Position * pos, int depth );	//checks if a move is legal

int evaluate( Position * pos);	//evaluates position for client (player mycolor)

int maxValue( Position * pos, Move * bestMove, int depth );	//util function for minimax (maximizer)

int minValue( Position * pos, Move * bestMove, int depth );	//util function for minimax (minimizer)

int minimax( Position * pos, Move * bestMove); //minimax search (stores best move in bestMove)

int abMax( Position * pos, Move * bestMove, int depth, int a, int b); //util function for abSearch (maximizer)

int abMin( Position * pos, Move * bestMove, int depth, int a, int b); //util function for abSearch (minimizer)

int abSearch( Position * pos, Move * bestMove); //ab-pruning search

#endif