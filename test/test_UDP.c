/*------------------------------------------------------------------
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

A test program for EGI UDP moudle, to transmit/exchange data between
a server and several clients.

Usage:	test_UDP [-hsca:p:]
Example:
	./test_UDP -s -p 8765			( As server )
	./test_UDP -c -a 192.168.9.9 -p 8765	( As client )

	( Use test_garph.c to show wifi interface speed )

Midas Zhou
midaszhou@yahoo.com
https://github.com/widora/wegi
------------------------------------------------------------------*/
#include <sys/types.h> /* this header file is not required on Linux */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> /* inet_ntoa() */
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <egi_inet.h>

enum transmit_mode {
	mode_2Way	=0,  /* Server and Client PingPong mode */
	mode_S2C	=1,  /* Server to client only */
	mode_C2S	=2,  /* Client to server only */
};
static enum transmit_mode trans_mode;

void show_help(const char* cmd)
{
	printf("Usage: %s [-hsca:p:m:] \n", cmd);
       	printf("        -h   help \n");
   	printf("        -s   work as UDP Server\n");
   	printf("        -c   work as UDP Client (default)\n");
	printf("	-a:  IP address\n");
	printf("	-p:  Port number\n");
	printf("	-m:  Transmit mode, 0-Twoway(default), 1-S2C, 2-C2S \n");
}

int Client_Callback( const struct sockaddr_in *rcvAddr, const char *rcvData, int rcvSize, char *sndBuff, int *sndSize );
int Server_Callback( const struct sockaddr_in *rcvAddr, const char *rcvData, int rcvSize, struct sockaddr_in *sndAddr, char *sndBuff, int *sndSize);

/*----------------------------
	    Main()
----------------------------*/
int main(int argc, char **argv)
{
	int opt;
	bool SetUDPServer=false;
	char *strAddr=NULL;		/* Default NULL, to be decided by system */
	unsigned short int port=0; 	/* Default 0, to be selected by system */

	/* Parse input option */
	while( (opt=getopt(argc,argv,"hsca:p:m:"))!=-1 ) {
		switch(opt) {
			case 'h':
				show_help(argv[0]);
				exit(0);
				break;
			case 's':
				printf("Set as UDP Server.\n");
				SetUDPServer=true;
				break;
			case 'c':
				printf("Set as UDP Client.\n");
				SetUDPServer=false;
				break;
			case 'a':
				strAddr=strdup(optarg);
				printf("Set addres='%s'\n",strAddr);
				break;
			case 'p':
				port=atoi(optarg);
				printf("Set port=%d\n",port);
				break;
			case 'm':
				trans_mode=atoi(optarg);
				break;
			default:
				SetUDPServer=false;
				break;
		}
	}

  /* A. --- Work as a UDP Server */
  if( SetUDPServer ) {

        /* Create UDP server */
        EGI_UDP_SERV *userv=inet_create_udpServer(strAddr, port, 4); //8765, 4);
        if(userv==NULL)
                exit(1);

        /* Set callback */
        userv->callback=Server_Callback; //inet_udpServer_TESTcallback;

        /* Process UDP server routine */
        inet_udpServer_routine(userv);

        /* Free and destroy */
        inet_destroy_udpServer(&userv);

  /* B. --- Work as a UDP Client */
  } else {
	if( strAddr==NULL || port==0 ) {
		printf("Please provide Server IP address and port number!\n");
		show_help(argv[0]);
		exit(2);
	}

        /* Create UDP client */
        EGI_UDP_CLIT *uclit=inet_create_udpClient(strAddr, port, 4); //"192.168.8.1", 8765, 4);
        if(uclit==NULL)
                exit(1);

        /* Set callback */
        uclit->callback=Client_Callback; //inet_udpClient_TESTcallback;

        /* Process UDP server routine */
        inet_udpClient_routine(uclit);

        /* Free and destroy */
        inet_destroy_udpClient(&uclit);

  }

	free(strAddr);
        return 0;
}



/*-----------------------------------------------------------------
A TEST callback routine for UPD server.

@rcvAddr:       Client address, from which rcvData was received.
@rcvData:       Data received from rcvAddr
@rcvSize:       Data size of rcvData.

@sndAddr:       Client address, to which sndBuff will be sent.
@sndBuff:       Data to send to sndAddr
@sndSize:       Data size of sndBuff

Return:
        0       OK
        <0      Fails
------------------------------------------------------------------*/
int Server_Callback( const struct sockaddr_in *rcvAddr, const char *rcvData, int rcvSize,
                           struct sockaddr_in *sndAddr,       char *sndBuff, int *sndSize)
{
	if(rcvSize>0)
		printf("Received %d bytes data.\n", rcvSize);

	/* If Client_to_Server only, reply a small packet to keep alive. */
	if(trans_mode==mode_C2S) {
		*sndSize=4;
		*sndAddr=*rcvAddr;
		return 0;
	}

	/* TEST FOR SPEED: whatever, just request to send back ... */
	*sndSize=EGI_MAX_UDP_PDATA_SIZE;
	/* Use default rcvAddr data in the UDP Server's buffer */
	*sndAddr=*rcvAddr;
	/* If rcvSize<0, then rcvAddr would be meaningless, whaterver, use default value in the UDP Server's. */

	return 0;
}

/*-------------------------------------------------------------------
A TEST callback routine for UPD client.
@rcvAddr:       Server/Sender address, from which rcvData was received.
@rcvData:       Data received from rcvAddr
@rcvSize:       Data size of rcvData.

@sndBuff:       Data to send to UDP server
@sndSize:       Data size of sndBuff

Return:
        0       OK
        <0      Fails
--------------------------------------------------------------------*/
int Client_Callback( const struct sockaddr_in *rcvAddr, const char *rcvData, int rcvSize,
                                                              char *sndBuff, int *sndSize )
{
	if(rcvSize>0)
		printf("Received %d bytes data.\n", rcvSize);

	/* If Server_to_Client only. Reply a small packet to keep alive. */
	if(trans_mode==mode_S2C) {
		*sndSize=4;
		return 0;
	}

	/* TEST FOR SPEED: whatever, just request to send back ... */
	*sndSize=EGI_MAX_UDP_PDATA_SIZE;
	/* Use default data in the UDP Server's buffer */

	return 0;
}
