#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**********************************************************/
char * port = DEFAULT_PORT;		// default port

/**********************************************************/
void listenToSocket( char * port, int * mySocket )
{
	// socket
	if( ( *mySocket = socket( PF_INET, SOCK_STREAM, 0 ) ) < 0)
	{
		printf( "ERROR: Opening socket Failed\n" );
		exit( 1 );
	}

	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons( atoi( port ) );
	serveraddr.sin_addr.s_addr = htonl( INADDR_ANY );

	int optval;
	optval = 1;
	setsockopt( *mySocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval );


	// bind
	if( bind( *mySocket, ( struct sockaddr* ) &serveraddr, sizeof( serveraddr ) ) < 0 )
	{
		printf( "ERROR: bind function Failed\n" );
		exit(1);
	}

	unsigned int length;

	length = sizeof( serveraddr );
	getsockname ( *mySocket, ( struct sockaddr* ) &serveraddr, &length );

	// listen
	if( listen( *mySocket, MAXPENDING ) < 0 )
	{
		printf( "ERROR: Listen failed!\n" );
		exit( 1 );
	}

	printf( "Listening to port: %d...\n", ntohs( serveraddr.sin_port ) );

}

/**********************************************************/
int acceptConnection( int mySocket )
{
	int connectedSocket;

	struct sockaddr_in connAddr;
	socklen_t clientaddrLen;

	clientaddrLen = sizeof( connAddr );

	// accept
	if( ( connectedSocket = accept( mySocket, ( struct sockaddr* ) &connAddr, &clientaddrLen ) ) < 0 )
	{
		printf( "ERROR: Accept failed!\n" );
		return -1;

	}

	return connectedSocket;
}


/**********************************************************/
void connectToTarget( char * port, char * ip, int * mySocket )
{

	if( ( *mySocket = socket( PF_INET, SOCK_STREAM, 0 ) ) < 0)
	{
		printf( "ERROR: Opening socket Failed\n" );
		exit( 1 );
	}


	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons( atoi( port ) );
	serveraddr.sin_addr.s_addr = inet_addr( ip );

	while( connect( *mySocket, ( struct sockaddr* ) &serveraddr, sizeof( serveraddr ) ) < 0 )
	{
		printf( "ERROR: Connect function Failed.. Retrying\n" );
		sleep( 1 );
	}

}

/**********************************************************/
int sendMsg( int msg, int mySocket )
{
	char msgCode;

	msgCode = ( char ) msg;

	if( send( mySocket, &msgCode, 1, 0 ) != 1 )
	{
		printf( "ERROR: Network problem\n" );
		return -1;
	}

	return 0;

}

/**********************************************************/
int recvMsg( int socket )
{
	char msg;

	if( recv( socket, &msg, 1, 0 ) != 1 )
	{
		printf( "ERROR: Network problem\n" );
		exit( 1 );
	}

	return ( int ) msg;

}


/**********************************************************/
int sendMove( Move * moveToSend, int mySocket )
{
	char buffer[ 2 ];

	buffer[ 0 ] = moveToSend->tile[ 0 ];
	buffer[ 1 ] = moveToSend->tile[ 1 ];

	if( send( mySocket, buffer, 2, 0 ) != 2 )
	{
		printf( "ERROR: Network problem\n" );
		return -1;
	}

	return 0;

}

/**********************************************************/
int getMove( Move * moveToGet, int mySocket )
{
	char buffer[ 2 ];

	if( recv( mySocket, buffer, 2, 0 ) != 2 )
	{
		printf( "ERROR: Network problem\n" );
		return -1;
	}


	moveToGet->tile[ 0 ] = buffer[ 0 ];
	moveToGet->tile[ 1 ] = buffer[ 1 ];


	return 0;

}

/**********************************************************/
void sendName( char textToSend[ MAX_NAME_LENGTH + 1 ], int mySocket )
{
	int size;
	char size_char;

	size = strlen( textToSend );
	size_char = ( char ) size;

	if( send( mySocket, &size_char, 1, 0 ) != 1 )
	{
		printf( "ERROR: Network problem\n" );
		exit( 1 );
	}

	if( send( mySocket, textToSend, size , 0 ) != size )
	{
		printf( "ERROR: Network problem\n" );
		exit( 1 );
	}

}


/**********************************************************/
int getName( char textToGet[ MAX_NAME_LENGTH + 1 ], int mySocket )
{

	int size;
	char size_char;
	char dummy[256];

	if( recv( mySocket, &size_char, 1, 0 ) != 1 )
	{
		printf( "ERROR: Network problem\n" );
		return -1;
	}

	size = ( int ) size_char;

	if( recv( mySocket, dummy, size, 0 ) != size )
	{
		printf( "ERROR: Network problem\n" );
		return -1;
	}


	if( size > MAX_NAME_LENGTH ) size = MAX_NAME_LENGTH;
	strncpy( textToGet, dummy, size );

	textToGet[ size ] = '\0';

	return 0;

}


/**********************************************************/
int sendPosition( Position * posToSend, int mySocket )
{
	char buffer[ ARRAY_BOARD_SIZE * ARRAY_BOARD_SIZE + 2 + 1 ];
	int i, j;

	//board
	for( i = 0; i < ARRAY_BOARD_SIZE; i++ )
		for( j = 0; j < ARRAY_BOARD_SIZE; j++ )
			buffer[ i * ARRAY_BOARD_SIZE + j ] =  posToSend->board[ i ][ j ];

	//score
	buffer[ ARRAY_BOARD_SIZE * ARRAY_BOARD_SIZE ] = posToSend->score[ WHITE ];
	buffer[ ARRAY_BOARD_SIZE * ARRAY_BOARD_SIZE + 1 ] = posToSend->score[ BLACK ];

	//turn
	buffer[ ARRAY_BOARD_SIZE * ARRAY_BOARD_SIZE + 2 ] = posToSend->turn;

	if( send( mySocket, buffer, ARRAY_BOARD_SIZE * ARRAY_BOARD_SIZE + 2 + 1, 0 ) != ARRAY_BOARD_SIZE * ARRAY_BOARD_SIZE + 2 + 1 )
	{
		printf( "ERROR: Network problem\n" );
		return -1;
	}

	return 0;

}

/**********************************************************/
void getPosition( Position * posToGet, int mySocket )
{
	char buffer[ ARRAY_BOARD_SIZE * ARRAY_BOARD_SIZE + 2 + 1 ];
	int i, j;

	if( recv( mySocket, buffer, ARRAY_BOARD_SIZE * ARRAY_BOARD_SIZE + 2 + 1, 0 ) != ARRAY_BOARD_SIZE * ARRAY_BOARD_SIZE + 2 + 1 )
	{
		printf( "ERROR: Network problem\n" );
		exit( 1 );
	}

	//board
	for( i = 0; i < ARRAY_BOARD_SIZE; i++ )
		for( j = 0; j < ARRAY_BOARD_SIZE; j++ )
			posToGet->board[ i ][ j ] = buffer[ i * ARRAY_BOARD_SIZE + j ];

	//score
	posToGet->score[ WHITE ] = buffer[ ARRAY_BOARD_SIZE * ARRAY_BOARD_SIZE ];
	posToGet->score[ BLACK ] = buffer[ ARRAY_BOARD_SIZE * ARRAY_BOARD_SIZE + 1 ];

	//turn
	posToGet->turn = buffer[ ARRAY_BOARD_SIZE * ARRAY_BOARD_SIZE + 2 ];
}






