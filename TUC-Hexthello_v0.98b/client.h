#ifndef _CLIENT_H
#define _CLIENT_H

#include "global.h"
#include "move.h"
#include "board.h"

/**********************************************************/

#define MAX_DEPTH 5		// Max fixed depth used for simple cutoff function

int isCutoff( Position * pos, int depth );	// checks if a move is legal

float evaluateSimple( Position * pos);	// evaluates position for client with simple coin difference

float normalizeScore( float myScore, float oppScore); // util function for evaluation heuristics

void calcMobility( Position * pos, float * actualMobility, float * potentialMobility);
/*calculates the actual and potential mobility of the given position and stores  
them using the respective pointers*/

float calcCornerScore( Position * pos);
//calculates the corner score of a position (corners captured and potential corners)

float evaluate( Position * pos);
//evaluates position for client with weighted function (coin parity, mobility, corners)

float evaluate2( Position * pos);
//used for weight experimentations (same core as evaluate)

float maxValue( Position * pos, Move * bestMove, int depth, float (*evalFuncPtr)(Position *));	
//util function for minimax (maximizer)

float minValue( Position * pos, Move * bestMove, int depth, float (*evalFuncPtr)(Position *));	
//util function for minimax (minimizer)

float minimax( Position * pos, Move * bestMove, float (*evalFuncPtr)(Position *)); 
//minimax search (stores best move in bestMove)

float abMax( Position * pos, Move * bestMove, int depth, float a, float b, float (*evalFuncPtr)(Position *)); 
//util function for abSearch (maximizer)

float abMin( Position * pos, Move * bestMove, int depth, float a, float b, float (*evalFuncPtr)(Position *)); 
//util function for abSearch (minimizer)

float abSearch( Position * pos, Move * bestMove, float (*evalFuncPtr)(Position *)); //ab-pruning search

#endif