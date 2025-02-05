#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>



/**********************************************************/
void initPosition( Position * pos )		//to add illegal tiles change ONLY this function! ...everything else will work...hopefully
{
	int i, j;
	int illegalPerRow = HEX_BOARD_RADIUS;

	for( i = 0; i < ARRAY_BOARD_SIZE; i++ )
	{
		for( j = 0; j < ARRAY_BOARD_SIZE; j++ )
		{
			if( illegalPerRow > j || (illegalPerRow < 0 && (j >= illegalPerRow + ARRAY_BOARD_SIZE)) )
				pos->board[ i ][ j ] = OUT_OF_BOUND;
			else if( (j == HEX_BOARD_RADIUS && (i == HEX_BOARD_RADIUS || i == HEX_BOARD_RADIUS - 1)) || (i == HEX_BOARD_RADIUS && j == HEX_BOARD_RADIUS + 1 ) || ( i == HEX_BOARD_RADIUS + 1 && j == HEX_BOARD_RADIUS - 1 ) )
					pos->board[ i ][ j ] = WHITE;
			else if( (i == HEX_BOARD_RADIUS && j == HEX_BOARD_RADIUS - 1 ) || (i == HEX_BOARD_RADIUS - 1 && j == HEX_BOARD_RADIUS + 1) || (i == HEX_BOARD_RADIUS + 1 && j == HEX_BOARD_RADIUS ) )
					pos->board[ i ][ j ] = BLACK;
			else
				pos->board[ i ][ j ] = EMPTY;
		}
		illegalPerRow--;
	}


	/* Reset score */
	pos->score[ WHITE ] = 4;
	pos->score[ BLACK ] = 3;

	/* Black plays first */
	pos->turn = BLACK;

}


/**********************************************************/
void printBoard( char board[ ARRAY_BOARD_SIZE ][ ARRAY_BOARD_SIZE ] )
{
	int i, j;
	int spacePerRow = HEX_BOARD_RADIUS;
	int spaceCounter;


	/* Print board */
	for( i = 0; i < ARRAY_BOARD_SIZE; i++ )
	{
		printf(" ");
		for( spaceCounter = 0; spaceCounter < abs(spacePerRow); spaceCounter++ )
			printf(" ");

		for( j = 0; j < ARRAY_BOARD_SIZE; j++ )
			switch( board[ i ][ j ] )
			{
				case OUT_OF_BOUND:
					break;
				case WHITE:
					printf( "W " );
					break;
				case BLACK:
					printf( "B " );
					break;
				case EMPTY:
					printf( ". " );
					break;
				case ILLEGAL:
					printf( "X " );
					break;
				default:
					printf("ERROR: Unknown character in board (printBoard)\n");
					exit( 1 );
			}
		printf( "\n" );
		spacePerRow--;
	}

}


/**********************************************************/
void printPosition( Position * pos )
{
	//board
	printBoard( pos->board );

	//turn
	if( pos->turn == WHITE )
		printf( "Turn: WHITE" );
	else if( pos->turn == BLACK )
		printf( "Turn: BLACK" );
	else
		printf( "Turn: -" );

	printf( "\n" );

	//score
	printf( "Score is  W: %d  B: %d\n" , pos->score[ WHITE ], pos->score[ BLACK ] );

}

/**********************************************************/
int doOneDirection( Position * pos, Move * moveToPlay, int do_move, signed char inc_row, signed char inc_col )
{
	unsigned short int i,j;
	unsigned short int opponent;

	opponent = getOtherSide( moveToPlay->color );

	/* Quick rejects - are we too close to the borders? */
	if((inc_row == -1 && moveToPlay->tile[ 0 ] < 2) || (inc_row == 1  && moveToPlay->tile[ 0 ] >= ARRAY_BOARD_SIZE-2))
		return FALSE;
	if((inc_col == -1 && moveToPlay->tile[ 1 ] < 2) || (inc_col == 1  && moveToPlay->tile[ 1 ] >= ARRAY_BOARD_SIZE-2))
		return FALSE;

	i = moveToPlay->tile[ 0 ] + inc_row; 
	j = moveToPlay->tile[ 1 ] + inc_col;

	/* The adjacent square must be in the opposite color. */
	if( pos->board[ i ][ j ] != opponent )
		return FALSE;

	/* Go over all the consecutive squares in the opponents color */
	do
	{
		i += inc_row; 
		j += inc_col;
		if( i<0 || i>=ARRAY_BOARD_SIZE || j<0 || j>=ARRAY_BOARD_SIZE)
			break;
		if(pos->board[ i ][ j ] !=opponent)
			break;
	}
	while(1);

	/* Have we reached the end of the board or not a piece of ours (failure)? */
	if(i < 0 || i >= ARRAY_BOARD_SIZE || j < 0 || j >= ARRAY_BOARD_SIZE || pos->board[ i ][ j ]!=moveToPlay->color)
		return FALSE;

	/* If we have reached here, then we have a legal move !!! */
	if (!do_move)
		return TRUE;

	/* Flip the other side pieces */
	for (i-=inc_row, j-=inc_col; i != moveToPlay->tile[ 0 ] || j != moveToPlay->tile[ 1 ]; i-=inc_row, j-=inc_col) 
	{
		pos->board[ i ][ j ] = moveToPlay->color;
		pos->score[ (int) moveToPlay->color ]++;
		pos->score[ opponent ]--;
	}

	return TRUE;

}

/**********************************************************/
int doAllDirections( Position * pos, Move * moveToPlay, int do_move )
{
	short int i,j;
	int legal = FALSE; /* Was the move found to be legal already? */


	/*null move?*/
	if( ( moveToPlay->tile[ 0 ] == NULL_MOVE ) && do_move)
	{
		pos->turn = getOtherSide( pos->turn );
		return TRUE;
	}

	for (i=-1 ; i <= 1 ; i++)
		for (j=-1 ; j <= 1 ; j++)
		{
			if (j == i)
				continue;
			if(doOneDirection( pos, moveToPlay, do_move, i, j))
				legal = TRUE;

			/* If we found something legal it may be enough ... */
			if (legal && !do_move)
				return TRUE;
		}

	/* Put the piece in its place */
	if(legal && do_move) {
		pos->board[ moveToPlay->tile[ 0 ] ][ moveToPlay->tile[ 1 ] ] = moveToPlay->color;
		pos->score[ (int) moveToPlay->color ]++;
		pos->turn = getOtherSide( pos->turn );
	}

	return legal;
}

/**********************************************************/
void doMove( Position * pos, Move * moveToDo )
{
	doAllDirections(pos, moveToDo, TRUE);
}

/**********************************************************/
int isLegalMove( Position * pos, Move * moveToCheck )
{

	if(moveToCheck->tile[ 0 ] < 0 || moveToCheck->tile[ 0 ] >= ARRAY_BOARD_SIZE || moveToCheck->tile[ 1 ] < 0 || moveToCheck->tile[ 1 ] >= ARRAY_BOARD_SIZE)
		return FALSE;

	if(pos->board[ moveToCheck->tile[ 0 ] ][ moveToCheck->tile[ 1 ] ] != EMPTY)
		return FALSE;

	return doAllDirections(pos, moveToCheck, FALSE);	//try to make the move without actually doing it
}

/**********************************************************/
int isLegal( Position * pos, short int row, short int col, char color)
{
	Move moveToCheck;
	moveToCheck.tile[ 0 ] = row;
	moveToCheck.tile[ 1 ] = col;
	moveToCheck.color = color;
	return isLegalMove(pos, &moveToCheck);
}

/**********************************************************/
int canMove( Position * pos, char color)
{
	unsigned short int i,j;

	for(i=0 ; i <ARRAY_BOARD_SIZE ; i++)
		for (j=0 ; j <ARRAY_BOARD_SIZE ; j++)
		{
			if (isLegal(pos, i, j, color))
				return TRUE;
		}
	return FALSE;
}





