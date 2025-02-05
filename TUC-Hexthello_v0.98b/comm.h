#ifndef _COMM_H
#define _COMM_H

#include "global.h"
#include "board.h"
#include "move.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h> 

/**********************************************************/
#define MAXPENDING 10
/**********************************************************/
#define NM_NEW_POSITION 101
#define NM_COLOR_W 102
#define NM_COLOR_B 103
#define NM_REQUEST_MOVE 104
#define NM_PREPARE_TO_RECEIVE_MOVE 105
#define NM_REQUEST_NAME 106
#define NM_QUIT 107
/**********************************************************/
extern char * port;
/**********************************************************/

void listenToSocket( char * port, int * mySocket );
//creates a socket and starts to listen (used by server)

int acceptConnection( int mySocket );
//accepts new connections (used by server)

void connectToTarget( char * port, char * ip, int * mySocket );
//connects to a server (used by client)

int sendMsg( int msg, int mySocket );
//sends a network message (one char)

int recvMsg( int mySocket );
//receives a network message

int sendMove( Move * moveToSend, int mySocket );
//sends a move via mySocket

int getMove( Move * moveToGet, int mySocket );
//receives a move from mySocket

void sendName( char textToSend[ MAX_NAME_LENGTH + 1 ], int mySocket );
//used to send agent's name to server

int getName( char textToGet[ MAX_NAME_LENGTH + 1 ], int mySocket );
//used to receive agent's name

int sendPosition( Position * posToSend, int mySocket );
//used to send position struct

void getPosition( Position * posToGet, int mySocket );
//used to receive position struct

#endif




