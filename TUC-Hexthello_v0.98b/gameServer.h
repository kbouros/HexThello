#ifndef _GAMESERVER_H
#define _GAMESERVER_H

#include "global.h"
#include "board.h"
#include "move.h"


/**********************************************************/
/* Struct to store name, color, and the socket of a player (if remotely connected) */
/* used only by servers*/
typedef struct
{
	char connected;							//used in guiserver. If 1 then the struct is used, if 0 then it's available.
	char name[ MAX_NAME_LENGTH + 1 ];
	char color;
	int playerSocket;
} PlayerStruct;

/**********************************************************/

extern int serverSocket;

extern Position gamePosition;
extern Move tempMove;


extern PlayerStruct playerOne;
extern PlayerStruct playerTwo;

extern PlayerStruct * playingPlayer;
extern PlayerStruct * waitingPlayer;

extern int numberOfGames;
extern int swapAfterEachGame;

#endif
