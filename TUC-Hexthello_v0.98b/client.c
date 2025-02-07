#include "client.h"
#include "global.h"
#include "board.h"
#include "move.h"
#include "comm.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <float.h>

/**********************************************************/
Position gamePosition;		// Position we are going to use

Move moveReceived;			// temporary move to retrieve opponent's choice
Move myMove;				// move to save our choice and send it to the server

char myColor;				// to store our color
int mySocket;				// our socket
char msg;					// used to store the received message

char * agentName = "kbouros";		//default name.. change it! keep in mind MAX_NAME_LENGTH

char * ip = "127.0.0.1";	// default ip (local machine)

char * mode = "ab"; 		// default mode (ab-pruning)

float (*funcArr[])(Position *) = {evaluate, evaluateSimple, evaluate2}; // available evaluation functions

int evalFunction = 0;		// default evaluation function (as position in the above array)

/**********************************************************/

int countNodes;

int main( int argc, char ** argv )
{
	int c; 
	opterr = 0;

	while( ( c = getopt ( argc, argv, "i:p:m:e:n:h" ) ) != -1 )
		switch( c )
		{
			case 'h':
				printf( "[-i ip] [-p port] [-m mode] [-e eval function] [-n name]\n" );
				printf("Mode can take one of the following values:\n");
				printf("r : random, m : minimax, ab : ab-pruning\n");
				printf("Evaluation function options:\n");
				printf("0 : weighted function, 1 : coin parity\n");
				return 0;
			case 'i':
				ip = optarg;
				break;
			case 'p':
				port = optarg;
				break;
			case 'm':
				mode = optarg;
				if(strcmp(mode, "r") != 0 && strcmp(mode, "m") != 0 && strcmp(mode, "ab") != 0){
					printf("Invalid argument. \n");
					return 1;
				}
				break;
			case 'e':
				evalFunction = atoi(optarg);
				if(evalFunction > 2 || evalFunction < 0){
					printf("Invalid argument. [-e] requires an integer between 0 and 1. \n");
					return 1;
				}
				break;
			case 'n':
				agentName = optarg;
				break;
			case '?':
				if( optopt == 'i' || optopt == 'p' || optopt == 'm' || optopt == 'n')
					printf( "Option -%c requires an argument.\n", ( char ) optopt );
				else if( isprint( optopt ) )
					printf( "Unknown option -%c\n", ( char ) optopt );
				else
					printf( "Unknown option character -%c\n", ( char ) optopt );
				return 1;
			default:
			return 1;
		}

	connectToTarget( port, ip, &mySocket );

/**********************************************************/
// used in random
	srand( time( NULL ) );
	int i, j;
/**********************************************************/

	while( 1 )
	{

		msg = recvMsg( mySocket );

		switch ( msg )
		{
			case NM_REQUEST_NAME:		//server asks for our name
				sendName( agentName, mySocket );
				break;

			case NM_NEW_POSITION:		//server is trying to send us a new position
				getPosition( &gamePosition, mySocket );
				printPosition( &gamePosition );
				break;

			case NM_COLOR_W:			//server informs us that we have WHITE color
				myColor = WHITE;
				break;

			case NM_COLOR_B:			//server informs us that we have BLACK color
				myColor = BLACK;
				break;

			case NM_PREPARE_TO_RECEIVE_MOVE:	//server informs us that he will now send us opponent's move
				getMove( &moveReceived, mySocket );
				moveReceived.color = getOtherSide( myColor );
				doMove( &gamePosition, &moveReceived );		//play opponent's move on our position
				printPosition( &gamePosition );
				break;

			case NM_REQUEST_MOVE:		//server requests our move
				myMove.color = myColor;


				if( !canMove( &gamePosition, myColor ) )
				{
					myMove.tile[ 0 ] = NULL_MOVE;		// we have no move ..so send null move
				}
				else
				{


/**********************************************************/
// random player - not the most efficient implementation
					if( strcmp(mode, "r") == 0 ){


						while( 1 )
						{	
							i = rand() % ARRAY_BOARD_SIZE;
							j = rand() % ARRAY_BOARD_SIZE;

							if( gamePosition.board[ i ][ j ] == EMPTY )
							{
								myMove.tile[ 0 ] = i;
								myMove.tile[ 1 ] = j;
								if( isLegalMove( &gamePosition, &myMove ) )
									break;
							}
						}
					}	
// end of random
/**********************************************************/
// minimax player
					else if( strcmp(mode, "m") == 0 ){
						countNodes = 0;
						minimax( &gamePosition, &myMove, funcArr[evalFunction]);
						printf("\n Number of nodes : %d\n",countNodes);
					}

/**********************************************************/
// a-b pruning player
					else if( strcmp(mode, "ab") == 0 ){
						countNodes = 0;
						abSearch( &gamePosition, &myMove, funcArr[evalFunction]);
						printf("\n Number of nodes : %d\n",countNodes);	
					}
									

// end of choosing move
/**********************************************************/

				}

				sendMove( &myMove, mySocket );			//send our move
				doMove( &gamePosition, &myMove );		//play our move on our position
				printPosition( &gamePosition );
				break;

			case NM_QUIT:			//server wants us to quit...we shall obey
				close( mySocket );
				return 0;
		}

	} 

	return 0;
}

/**********************************************************/
int isCutoff( Position * pos, int depth )
{
	// check depth
	if( depth >= MAX_DEPTH)
		return TRUE;

	//check terminal condition
	if( !canMove( pos, WHITE ) && !canMove( pos, BLACK ) )	//if none can move..game ended
		return TRUE;

	return FALSE;
}

/**********************************************************/
float evaluateSimple( Position * pos)
{
	float coinParity = pos->score[ (int) myColor ] - pos->score[ (int) getOtherSide(myColor) ];

	return coinParity;
}

/**********************************************************/
float normalizeScore( float myScore, float oppScore){

	if( (myScore + oppScore) != 0 ){
		return 100*(myScore - oppScore)/(myScore + oppScore);
	}
	return 0;
}

/**********************************************************/
void calcMobility( Position * pos, float * actualMobility, float * potentialMobility)
{
	/* Calculates the actual and potential mobility of the given position and  
	stores them using the respective pointers */

	unsigned short int i,j;

	int myMobility = 0, oppMobility = 0;
	int myPotentialMobility = 0, oppPotentialMobility = 0;
	int flags[2]; //flag for having a white/black neighbor (indexes 0 and 1 respectively) 

	for(i=0 ; i <ARRAY_BOARD_SIZE ; i++){
		for (j=0 ; j <ARRAY_BOARD_SIZE ; j++)
		{
			if (isLegal(pos, i, j, myColor))
				myMobility++;

			if (isLegal(pos, i, j, getOtherSide(myColor)))
				oppMobility++;

			if(pos->board[i][j] == EMPTY)
			{
				// Check if a neighbor is colored
				flags[0] = flags[1] = 0;
				if(i+1 < ARRAY_BOARD_SIZE){
					// bottom neighbor
					if( pos->board[i+1][j] == BLACK || pos->board[i+1][j] == WHITE)
						flags[ (int) pos->board[i+1][j] ] = 1;

					// bottom left neighbor
					if(j-1 >= 0 && (pos->board[i+1][j-1] == BLACK || pos->board[i+1][j-1] == WHITE) )
						flags[ (int) pos->board[i+1][j-1] ] = 1;
				}

				if(j+1 < ARRAY_BOARD_SIZE){
					//right neighbor
					if( pos->board[i][j+1] == BLACK || pos->board[i][j+1] == WHITE)
						flags[ (int) pos->board[i][j+1] ] = 1;

					// top right neighbor
					if(i-1 >= 0 && (pos->board[i-1][j+1] == BLACK || pos->board[i-1][j+1] == WHITE) )
						flags[ (int) pos->board[i-1][j+1] ] = 1;
				}

				if(i-1 >= 0){
					// top neighbor
					if( pos->board[i-1][j] == BLACK || pos->board[i-1][j] == WHITE)
						flags[ (int) pos->board[i-1][j] ] = 1;
				}

				if(j-1 >= 0){
					// left neighbor
					if( pos->board[i][j-1] == BLACK || pos->board[i][j-1] == WHITE)
						flags[ (int) pos->board[i][j-1] ] = 1;
				}

				if(myColor == BLACK){
					myPotentialMobility += flags[0];
					oppPotentialMobility += flags[1];
				}
				else
				{
					myPotentialMobility += flags[1];
					oppPotentialMobility += flags[0];
				}
			}
		}
	}

	*actualMobility = normalizeScore( myMobility, oppMobility );
	*potentialMobility = normalizeScore( myPotentialMobility, oppPotentialMobility );
}

/**********************************************************/
float calcCornerScore( Position * pos){

	// Corners captured and potential corners (check 6 corners in total)
	float cornerScore, myScore, oppScore;
	int myCorners = 0, oppCorners = 0, myPotentialCorner = 0, oppPotentialCorner = 0;

	// Top left corner
	if( pos->board[0][HEX_BOARD_RADIUS] == myColor )
		myCorners++;
	else if( pos->board[0][HEX_BOARD_RADIUS] == getOtherSide(myColor))
		oppCorners++;
	else if( pos->board[0][HEX_BOARD_RADIUS] == EMPTY ){

		if( pos->board[0][HEX_BOARD_RADIUS+1] == myColor )
			oppPotentialCorner++;
		else if( pos->board[0][HEX_BOARD_RADIUS+1] == getOtherSide(myColor))
			myPotentialCorner++;

		if( pos->board[1][HEX_BOARD_RADIUS-1] == myColor )
			oppPotentialCorner++;
		else if( pos->board[1][HEX_BOARD_RADIUS-1] == getOtherSide(myColor))
			myPotentialCorner++;

		if( pos->board[1][HEX_BOARD_RADIUS] == myColor )
			oppPotentialCorner++;
		else if( pos->board[1][HEX_BOARD_RADIUS] == getOtherSide(myColor))
			myPotentialCorner++;
	}

	// Top right corner
	if( pos->board[0][2*HEX_BOARD_RADIUS] == myColor )
		myCorners++;
	else if( pos->board[0][2*HEX_BOARD_RADIUS] == getOtherSide(myColor))
		oppCorners++;
	else if( pos->board[0][2*HEX_BOARD_RADIUS] == EMPTY ){

		if( pos->board[0][2*HEX_BOARD_RADIUS-1] == myColor )
			oppPotentialCorner++;
		else if( pos->board[0][2*HEX_BOARD_RADIUS-1] == getOtherSide(myColor))
			myPotentialCorner++;

		if( pos->board[1][2*HEX_BOARD_RADIUS-1] == myColor )
			oppPotentialCorner++;
		else if( pos->board[1][2*HEX_BOARD_RADIUS-1] == getOtherSide(myColor))
			myPotentialCorner++;

		if( pos->board[1][2*HEX_BOARD_RADIUS] == myColor )
			oppPotentialCorner++;
		else if( pos->board[1][2*HEX_BOARD_RADIUS] == getOtherSide(myColor))
			myPotentialCorner++;
	}

	// Middle left corner
	if( pos->board[HEX_BOARD_RADIUS][0] == myColor )
		myCorners++;
	else if( pos->board[HEX_BOARD_RADIUS][0] == getOtherSide(myColor))
		oppCorners++;
	else if( pos->board[HEX_BOARD_RADIUS][0] == EMPTY ){

		if( pos->board[HEX_BOARD_RADIUS-1][1] == myColor )
			oppPotentialCorner++;
		else if( pos->board[HEX_BOARD_RADIUS-1][1] == getOtherSide(myColor))
			myPotentialCorner++;

		if( pos->board[HEX_BOARD_RADIUS][1] == myColor )
			oppPotentialCorner++;
		else if( pos->board[HEX_BOARD_RADIUS][1] == getOtherSide(myColor))
			myPotentialCorner++;

		if( pos->board[HEX_BOARD_RADIUS+1][0] == myColor )
			oppPotentialCorner++;
		else if( pos->board[HEX_BOARD_RADIUS+1][0] == getOtherSide(myColor))
			myPotentialCorner++;
	}

	// Middle right corner
	if( pos->board[HEX_BOARD_RADIUS][2*HEX_BOARD_RADIUS] == myColor )
		myCorners++;
	else if( pos->board[HEX_BOARD_RADIUS][2*HEX_BOARD_RADIUS] == getOtherSide(myColor))
		oppCorners++;
	else if( pos->board[HEX_BOARD_RADIUS][2*HEX_BOARD_RADIUS] == EMPTY ){

		if( pos->board[HEX_BOARD_RADIUS-1][2*HEX_BOARD_RADIUS] == myColor )
			oppPotentialCorner++;
		else if( pos->board[HEX_BOARD_RADIUS-1][2*HEX_BOARD_RADIUS] == getOtherSide(myColor))
			myPotentialCorner++;

		if( pos->board[HEX_BOARD_RADIUS][2*HEX_BOARD_RADIUS-1] == myColor )
			oppPotentialCorner++;
		else if( pos->board[HEX_BOARD_RADIUS][2*HEX_BOARD_RADIUS-1] == getOtherSide(myColor))
			myPotentialCorner++;

		if( pos->board[HEX_BOARD_RADIUS+1][2*HEX_BOARD_RADIUS-1] == myColor )
			oppPotentialCorner++;
		else if( pos->board[HEX_BOARD_RADIUS+1][2*HEX_BOARD_RADIUS-1] == getOtherSide(myColor))
			myPotentialCorner++;
	}

	// Bottom left corner
	if( pos->board[2*HEX_BOARD_RADIUS][0] == myColor )
		myCorners++;
	else if( pos->board[2*HEX_BOARD_RADIUS][0] == getOtherSide(myColor))
		oppCorners++;
	else if( pos->board[2*HEX_BOARD_RADIUS][0] == EMPTY ){

		if( pos->board[2*HEX_BOARD_RADIUS-1][0] == myColor )
			oppPotentialCorner++;
		else if( pos->board[2*HEX_BOARD_RADIUS-1][0] == getOtherSide(myColor))
			myPotentialCorner++;

		if( pos->board[2*HEX_BOARD_RADIUS-1][1] == myColor )
			oppPotentialCorner++;
		else if( pos->board[2*HEX_BOARD_RADIUS-1][1] == getOtherSide(myColor))
			myPotentialCorner++;

		if( pos->board[2*HEX_BOARD_RADIUS][1] == myColor )
			oppPotentialCorner++;
		else if( pos->board[2*HEX_BOARD_RADIUS][1] == getOtherSide(myColor))
			myPotentialCorner++;
	}

	// Bottom right corner
	if( pos->board[2*HEX_BOARD_RADIUS][HEX_BOARD_RADIUS] == myColor )
		myCorners++;
	else if( pos->board[2*HEX_BOARD_RADIUS][HEX_BOARD_RADIUS] == getOtherSide(myColor))
		oppCorners++;
	else if( pos->board[2*HEX_BOARD_RADIUS][HEX_BOARD_RADIUS] == EMPTY ){

		if( pos->board[2*HEX_BOARD_RADIUS][HEX_BOARD_RADIUS-1] == myColor )
			oppPotentialCorner++;
		else if( pos->board[2*HEX_BOARD_RADIUS][HEX_BOARD_RADIUS-1] == getOtherSide(myColor))
			myPotentialCorner++;

		if( pos->board[2*HEX_BOARD_RADIUS-1][HEX_BOARD_RADIUS] == myColor )
			oppPotentialCorner++;
		else if( pos->board[2*HEX_BOARD_RADIUS-1][HEX_BOARD_RADIUS] == getOtherSide(myColor))
			myPotentialCorner++;

		if( pos->board[2*HEX_BOARD_RADIUS-1][HEX_BOARD_RADIUS+1] == myColor )
			oppPotentialCorner++;
		else if( pos->board[2*HEX_BOARD_RADIUS-1][HEX_BOARD_RADIUS+1] == getOtherSide(myColor))
			myPotentialCorner++;
	}
	myScore = 2*myCorners + myPotentialCorner;
	oppScore = 2*oppCorners + oppPotentialCorner;
	cornerScore = normalizeScore( myScore, oppScore );

	return cornerScore;
}

/**********************************************************/
float evaluate( Position * pos)
{
	// Coin parity heuristic
	float coinParity = 100*(pos->score[(int) myColor] - pos->score[(int) getOtherSide(myColor)]);
	coinParity = coinParity / (pos->score[(int) myColor] + pos->score[(int) getOtherSide(myColor)]);

	// Mobility heuristic (actual and potential)
	float actualMobility, potentialMobility;
	calcMobility(pos, &actualMobility, &potentialMobility);

	float cornerScore = calcCornerScore(pos);
	
	return 20*coinParity + 5*actualMobility + 4*potentialMobility + 30*cornerScore;
}

/**********************************************************/
float evaluate2( Position * pos)
{	
	// Additional function used for weight experimentations
	// Coin parity heuristic
	float coinParity = 100*(pos->score[(int) myColor] - pos->score[(int) getOtherSide(myColor)]);
	coinParity = coinParity / (pos->score[(int) myColor] + pos->score[(int) getOtherSide(myColor)]);

	// Mobility heuristic (actual and potential)
	float actualMobility, potentialMobility;
	calcMobility(pos, &actualMobility, &potentialMobility);

	float cornerScore = calcCornerScore(pos);
	
	return 20*coinParity + 5*actualMobility + 5*potentialMobility + 30*cornerScore;
}

/**********************************************************/
float maxValue( Position * pos, Move * bestMove, int depth, float (*evalFuncPtr)(Position *))
{
	countNodes++;
	if( isCutoff( pos, depth ) )
		return evalFuncPtr(pos);

	float max = -FLT_MAX;	// best value
	float val; 				// current value	

	Move nextMove; 		// action
	Position childPos;	// resulting state

	// Not cutoff but can't move. Max player loses his turn
	if( !canMove( pos, myColor ) )
	{
		childPos = *pos;
		childPos.turn = getOtherSide( myColor ); // do NULL move
		/* We didnt need to search at this level. 
		Therefore, we can avoid increasing the depth counter, which effectively increases the max depth*/
		val = minValue(&childPos, NULL, depth, evalFuncPtr);

		if( bestMove != NULL){
			bestMove->tile[0] = NULL_MOVE;
			bestMove->color = myColor;
		}

		return val; // dont need to compare with max
	}

	unsigned short int i,j;

	for(i=0 ; i <ARRAY_BOARD_SIZE ; i++){
		for (j=0 ; j <ARRAY_BOARD_SIZE ; j++)
		{
			if (isLegal(pos, i, j, myColor))
			{	
				childPos = *pos;
				nextMove.tile[ 0 ] = i;
				nextMove.tile[ 1 ] = j;
				nextMove.color = myColor;

				doMove( &childPos, &nextMove ); 
				val = minValue(&childPos, NULL, depth+1, evalFuncPtr); // we don't need the best move

				if( val > max ){
					max = val;
					if( bestMove != NULL){
						bestMove->tile[0] = i;
						bestMove->tile[1] = j;
						bestMove->color = myColor;
					}
				}
			}
		}
	}

	return max;
}

/**********************************************************/
float minValue( Position * pos, Move * bestMove, int depth, float (*evalFuncPtr)(Position *))
{
	countNodes++;
	if( isCutoff( pos, depth ) )
		return evalFuncPtr(pos);

	float min = FLT_MAX; 	// best value
	float val;				// current value

	Move nextMove; 		// action
	Position childPos;	// resulting state

	// Not cutoff but can't move. Min player loses his turn
	if( !canMove( pos, getOtherSide(myColor) ) )
	{
		childPos = *pos;
		childPos.turn = myColor; // do NULL move
		/* We didnt need to search at this level. 
		Therefore, we can avoid increasing the depth counter, which effectively increases the max depth*/
		val = maxValue(&childPos, NULL, depth, evalFuncPtr);

		if( bestMove != NULL){
			bestMove->tile[0] = NULL_MOVE;
			bestMove->color = getOtherSide(myColor);
		}

		return val; // dont need to compare with min
	}

	unsigned short int i,j;

	for(i=0 ; i <ARRAY_BOARD_SIZE ; i++){
		for (j=0 ; j <ARRAY_BOARD_SIZE ; j++)
		{
			if (isLegal(pos, i, j, getOtherSide(myColor)))
			{	
				childPos = *pos;
				nextMove.tile[ 0 ] = i;
				nextMove.tile[ 1 ] = j;
				nextMove.color = getOtherSide(myColor);

				doMove( &childPos, &nextMove ); 
				val = maxValue(&childPos, NULL, depth+1, evalFuncPtr); // we don't need the best move

				if( val < min ){
					min = val;
					if( bestMove != NULL){
						bestMove->tile[0] = i;
						bestMove->tile[1] = j;
						bestMove->color = getOtherSide(myColor);
					}
				}
			}
		}
	}

	return min;
}
/**********************************************************/
float minimax( Position * pos, Move * bestMove, float (*evalFuncPtr)(Position *))
{
	return maxValue(pos, bestMove, 0, evalFuncPtr);
}

/**********************************************************/
float abMax( Position * pos, Move * bestMove, int depth, float a, float b, float (*evalFuncPtr)(Position *))
{
	countNodes++;
	if( isCutoff( pos, depth ) )
		return evalFuncPtr(pos);

	float max = -FLT_MAX; 	// best value
	float val;				// current value

	Move nextMove; 		// action
	Position childPos;	// resulting state

	// Not cutoff but can't move. Max player loses his turn
	if( !canMove( pos, myColor ) )
	{
		childPos = *pos;
		childPos.turn = getOtherSide( myColor ); // do NULL move

		/* We didnt need to search at this level. 
		Therefore, we can avoid increasing the depth counter, which effectively increases the max depth*/
		val = abMin(&childPos, NULL, depth, a, b, evalFuncPtr);

		if( bestMove != NULL){
			bestMove->tile[0] = NULL_MOVE;
			bestMove->color = myColor;
		}

		/* Note that NULL_MOVE is the only child node.
		Therefore we dont need to compare val with max or a*/
		return val; 
	}

	unsigned short int i,j;

	for(i=0 ; i <ARRAY_BOARD_SIZE ; i++){
		for (j=0 ; j <ARRAY_BOARD_SIZE ; j++)
		{
			if (isLegal(pos, i, j, myColor))
			{	
				childPos = *pos;
				nextMove.tile[ 0 ] = i;
				nextMove.tile[ 1 ] = j;
				nextMove.color = myColor;

				doMove( &childPos, &nextMove ); 
				val = abMin(&childPos, NULL, depth+1, a, b, evalFuncPtr); // we don't need the best move

				if( val > max ){
					max = val;
					if( a < val)
						a = val;

					if( bestMove != NULL ){
						bestMove->tile[0] = i;
						bestMove->tile[1] = j;
						bestMove->color = myColor;
					}
				}

				if( max >= b)
					return max;
			}
		}
	}

	return max;
}

/**********************************************************/
float abMin( Position * pos, Move * bestMove, int depth, float a, float b, float (*evalFuncPtr)(Position *))
{
	countNodes++;
	if( isCutoff( pos, depth ) )
		return evalFuncPtr(pos);

	float min = FLT_MAX; 	// best value
	float val;				// current value

	Move nextMove; 		// action
	Position childPos;	// resulting state

	// Not cutoff but can't move. Min player loses his turn
	if( !canMove( pos, getOtherSide(myColor) ) )
	{
		childPos = *pos;
		childPos.turn = myColor; // do NULL move

		/* We didnt need to search at this level. 
		Therefore, we can avoid increasing the depth counter, which effectively increases the max depth*/
		val = abMax(&childPos, NULL, depth, a, b, evalFuncPtr);

		if( bestMove != NULL){
			bestMove->tile[0] = NULL_MOVE;
			bestMove->color = getOtherSide(myColor);
		}

		/* Note that NULL_MOVE is the only child node.
		Therefore we dont need to compare val with min or b*/
		return val;
	}

	unsigned short int i,j;

	for(i=0 ; i <ARRAY_BOARD_SIZE ; i++){
		for (j=0 ; j <ARRAY_BOARD_SIZE ; j++)
		{
			if (isLegal(pos, i, j, getOtherSide(myColor)))
			{	
				childPos = *pos;
				nextMove.tile[ 0 ] = i;
				nextMove.tile[ 1 ] = j;
				nextMove.color = getOtherSide(myColor);

				doMove( &childPos, &nextMove ); 
				val = abMax(&childPos, NULL, depth+1, a, b, evalFuncPtr); // we don't need the best move

				if( val < min ){
					min = val;
					if( b > val)
						b = val;

					if( bestMove != NULL){
						bestMove->tile[0] = i;
						bestMove->tile[1] = j;
						bestMove->color = getOtherSide(myColor);
					}
				}
				if( min <= a)
					return min;
			}
		}
	}

	return min;
}

/**********************************************************/
float abSearch( Position * pos, Move * bestMove, float (*evalFuncPtr)(Position *) )
{
	return abMax(pos, bestMove, 0, -FLT_MAX, FLT_MAX, evalFuncPtr);
}








