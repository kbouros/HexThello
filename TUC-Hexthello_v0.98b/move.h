#ifndef _MOVE_H
#define _MOVE_H

#include "global.h"

/**********************************************************/

/* Struct to store tile and color of the player */
typedef struct
{
	signed char tile[ 2 ];		//Coordinates for the empty tile we chose for our move (we use the coordinates of our array)
	char color;					//Player's color
} Move;
/**********************************************************/


/*
Caution!

Server can ask for a move even if we have none available.
A Null move (the only legal move when we cannot move) has tile[0] = NULL_MOVE

*/


#endif
