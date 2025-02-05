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
#include <limits.h>

/**********************************************************/
Position gamePosition;		// Position we are going to use

Move moveReceived;			// temporary move to retrieve opponent's choice
Move myMove;				// move to save our choice and send it to the server

char myColor;				// to store our color
int mySocket;				// our socket
char msg;					// used to store the received message

char * agentName = "kbouros";		//default name.. change it! keep in mind MAX_NAME_LENGTH

char * ip = "127.0.0.1";	// default ip (local machine)

char * mode = "r"; 			// default mode (random)

/**********************************************************/

int countNodes;

int main( int argc, char ** argv )
{
	int c;
	opterr = 0;

	while( ( c = getopt ( argc, argv, "i:p:m:n:h" ) ) != -1 )
		switch( c )
		{
			case 'h':
				printf( "[-i ip] [-p port] [-m mode] [-n name]\n" );
				printf("Mode can take one of the following values:\n");
				printf("r : random, m : minimax, ab : ab-pruning\n");
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
						minimax( &gamePosition, &myMove);
						printf("\n Number of nodes : %d\n",countNodes);
					}

/**********************************************************/
// a-b pruning player
					else if( strcmp(mode, "ab") == 0 ){
						countNodes = 0;
						abSearch( &gamePosition, &myMove);
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
int evaluate( Position * pos)
{
	int coinParity = pos->score[ myColor ] - pos->score[ getOtherSide(myColor) ];

	return coinParity;
}

/**********************************************************/
int evaluate2( Position * pos)
{
	int coinParity = pos->score[ myColor ] - pos->score[ getOtherSide(myColor) ];

	return coinParity;
}

/**********************************************************/
int maxValue( Position * pos, Move * bestMove, int depth)
{
	countNodes++;
	if( isCutoff( pos, depth ) )
		return evaluate(pos);

	int max = INT_MIN; // current best value

	Move nextMove; 		// action
	Position childPos;	// resulting state

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
				int val = minValue(&childPos, NULL, depth+1); // we don't need the best move

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
int minValue( Position * pos, Move * bestMove, int depth)
{
	countNodes++;
	if( isCutoff( pos, depth ) )
		return evaluate(pos);

	int min = INT_MAX; // current best value

	Move nextMove; 		// action
	Position childPos;	// resulting state

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
				int val = maxValue(&childPos, NULL, depth+1); // we don't need the best move

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
int minimax( Position * pos, Move * bestMove)
{
	return maxValue(pos, bestMove, 0);
}

/**********************************************************/
int abMax( Position * pos, Move * bestMove, int depth, int a, int b)
{
	countNodes++;
	if( isCutoff( pos, depth ) )
		return evaluate(pos);

	int max = INT_MIN; // current best value

	Move nextMove; 		// action
	Position childPos;	// resulting state

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
				int val = abMin(&childPos, NULL, depth+1, a, b); // we don't need the best move

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
int abMin( Position * pos, Move * bestMove, int depth, int a, int b)
{
	countNodes++;
	if( isCutoff( pos, depth ) )
		return evaluate(pos);

	int min = INT_MAX; // current best value

	Move nextMove; 		// action
	Position childPos;	// resulting state

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
				int val = abMax(&childPos, NULL, depth+1, a, b); // we don't need the best move

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
int abSearch( Position * pos, Move * bestMove)
{
	return abMax(pos, bestMove, 0, INT_MIN, INT_MAX);
}








