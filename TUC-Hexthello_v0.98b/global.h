#ifndef _GLOBAL_H
#define _GLOBAL_H


//#define _DEBUG_		//comment this line when you are done

#ifdef _DEBUG_
    #include <assert.h>
#else
    #define assert(p) {}
#endif

#ifndef getOtherSide
	#define getOtherSide( a ) ( 1-(a) )
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/**********************************************************/

/* The size of our board */
#define HEX_BOARD_RADIUS 7
#define ARRAY_BOARD_SIZE (HEX_BOARD_RADIUS * 2 + 1)

/* used to describe a null move (the only legal "move" when no move is available) */
#define NULL_MOVE -50

/* Values for each possible tile state */
#define WHITE 0
#define BLACK 1
#define EMPTY 2
#define ILLEGAL 3
#define OUT_OF_BOUND 4

// max size of our name
#define MAX_NAME_LENGTH 8

//default port for client and server
#define DEFAULT_PORT "6002"


#endif
