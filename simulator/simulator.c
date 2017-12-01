/*---------------------------------------------------------------------------------------
--      SOURCE FILE:            simulator.c
--
--      PROGRAM:                sim.exe
--
--      DATE:                   November 18, 2017
--
--      DESIGNERS:              Brandon Gillespie
--
--      PROGRAMMERS:            Brandon Gillespie
--
--      NOTES:
--         Network Simulator for COMP7005 Project. 
---------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_TCP_PORT 7005     	// Default port
#define BUFLEN	        1024		// Buffer length
#define TRUE	        1
#define PAYLOAD         1024

struct packet {
	int  PacketType;
	int  SeqNum;
	char data[PAYLOAD];
	int  WindowSize;
	int  AckNum;
};

/*-----------------------------------------------------------------------
--	FUNCTION:	getClientMsg
--
--	DATE:       October 1, 2017
--
--	DESIGNER:   Brandon Gillespie
--
--  PROGRAMMER:	Brandon Gillespie
--
--	INTERFACE:	void getClientMsg(char *bfr, int s)
--
--	RETURNS:    void
--
--	NOTES:
--	Initializes variables and receives input from Client.
--  Stores the results in char *bfr
-----------------------------------------------------------------------*/
void getClientMsg(char *bfr, int s) {
	int m = 0;
	int btr = BUFLEN;
	char *bfrp = bfr;
	while ((m = recv(s, bfrp, btr, 0)) < BUFLEN) {
		bfrp += m;
		btr -= m;
	}
}

void buildPacketLogMsg(struct packet pck) {
	//sprintf("PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d", pck.PacketType, pck.SeqNum, pck.AckNum, pck.WindowSize);
	printf("PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d\n", pck.PacketType, pck.SeqNum, pck.AckNum, pck.WindowSize);
	//fprintf(f, "PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d\n", pck.PacketType, pck.SeqNum, pck.AckNum, pck.WindowSize);
}

void setPacketDataValue(char pa[], char va[], unsigned int pi, int s) {
	for (unsigned int i = 0; i < s; i++) {
		pa[pi++] = va[i];
	}
}

/*
void buildPacket(struct packet* pck, char packetdata[]) {
	char b[2], b1[5], b2[5], b3[4], b4[BUFLEN];
	pck->PacketType = atoi(strncpy(b, &packetdata[0], 1));
	pck->SeqNum = atoi(strncpy(b1, &packetdata[1], 4));
	pck->AckNum = atoi(strncpy(b2, &packetdata[5], 4));
	pck->WindowSize = atoi(strncpy(b3, &packetdata[9], 3));
	sprintf((*pck).data, "%s", &packetdata[12]);
	//printf("%d\n", pck->SeqNum);
	//printf("%d\n", pck->AckNum);
	//printf("%d\n", pck->WindowSize);
}

void buildPacketData(struct packet pck, char* pckdata) {
	//printf("1\n");
	char seq[5], ack[5], ws[4], pl[BUFLEN], pt[2];
	char t;
	for (unsigned int i = 0; i <= 12; i++) {
		switch (i) {
			case 0:
				sprintf(pt, "%d", pck.PacketType);
				pckdata[i] = pt[0];
				//printf("%d\n", pck.PacketType);
			break;
			case 1:
				sprintf(seq, "%d", pck.SeqNum);
				setPacketDataValue(pckdata, seq, i, 4);
				i += 3;
				//printf("%d\n", pck.SeqNum);
			break;
			case 5:
				sprintf(ack, "%d", pck.AckNum);
				setPacketDataValue(pckdata, ack, i, 4);
				i += 3;
				//printf("%d\n", pck.AckNum);
			break;
			case 9:
				sprintf(ws, "%d", pck.WindowSize);
				setPacketDataValue(pckdata, ws, i, 3);
				i += 2;
				//printf("%d\n", pck.WindowSize);
			break;
			case 12:
				setPacketDataValue(pckdata, pck.data, i, 2);
				//printf("%s\n", pck.data);
			break;
			default:
				//printf("D\n");
			break;
		}

	}
} */

void buildPacket(struct packet* pck, char packetdata[]) {
	char b[2], b1[7], b2[7], b3[4], b4[BUFLEN];
	pck->PacketType = atoi(strncpy(b, &packetdata[0], 1));
	pck->SeqNum = atoi(strncpy(b1, &packetdata[1], 6));
	pck->AckNum = atoi(strncpy(b2, &packetdata[7], 6));
	pck->WindowSize = atoi(strncpy(b3, &packetdata[13], 3));
	sprintf((*pck).data, "%s", &packetdata[16]);
}

void buildPacketData(struct packet pck, char* pckdata) {
	char pt[2], seq[7], ack[7], ws[4], pl[BUFLEN];
	char t;
	for (unsigned int i = 0; i <= 12; i++) {
		switch (i) {
			case 0:
				sprintf(pt, "%d", pck.PacketType);
				pckdata[i] = pt[0];
			break;
			case 1:
				sprintf(seq, "%d", pck.SeqNum);
				setPacketDataValue(pckdata, seq, i, 6);
				i += 5;
			break;
			case 5:
				sprintf(ack, "%d", pck.AckNum);
				setPacketDataValue(pckdata, ack, i, 6);
				i += 5;
			break;
			case 9:
				sprintf(ws, "%d", pck.WindowSize);
				setPacketDataValue(pckdata, ws, i, 3);
				i += 2;
			break;
			case 12:
				setPacketDataValue(pckdata, pck.data, i, 2);
			break;
			default:
			break;
		}

	}
}

void processRevPacket(struct packet* ipck, struct packet* opck) {
	//printf("%d\n", sizeof(ipck->data));
	opck->AckNum = ipck->SeqNum + sizeof(ipck->data);
	opck->SeqNum = 1;
	opck->PacketType = 1;
	opck->WindowSize = 5;
	opck->data[0] = 'A';
	opck->data[1] = '\0';
}

int main(int argc, char **argv) {
	int	                sd, new_sd, client_len, port;
	struct	sockaddr_in server, client;
	char	            rbuf[BUFLEN], sbuf[BUFLEN];
	struct packet       rpck, spck, pck1, pck2;

	// Assign port 7005
	port = SERVER_TCP_PORT;

	// Create a stream socket
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Can't create a socket");
		exit(1);
	}

	// Bind an address to the socket
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1) {
		perror("Can't bind name to socket");
		exit(1);
	}

	// Listen for connections

	// Queue up to 5 connect requests
	listen(sd, 5);

	while (TRUE) {
		client_len = sizeof(client);
		if ((new_sd = accept(sd, (struct sockaddr *)&client, &client_len)) == -1) {
			fprintf(stderr, "Can't accept client\n");
			exit(1);
		}

		printf(" Remote Address:  %s\n", inet_ntoa(client.sin_addr));


		for (unsigned int i = 0; i < 15; i++) {
			getClientMsg(rbuf, new_sd);
			buildPacket(&pck1, rbuf);
	
			getClientMsg(rbuf, new_sd);
			buildPacket(&pck2, rbuf);
	
			processRevPacket(&pck2, &spck);
	
			buildPacketData(spck, sbuf);
			buildPacketLogMsg(spck);
			send(new_sd, sbuf, BUFLEN, 0);
		}
		
		/*
		getClientMsg(rbuf, new_sd);
		buildPacket(&pck1, rbuf);

		getClientMsg(rbuf, new_sd);
		buildPacket(&pck2, rbuf);

		processRevPacket(&pck2, &spck);

		buildPacketData(spck, sbuf);
		buildPacketLogMsg(spck);
		send(new_sd, sbuf, BUFLEN, 0); */


        fflush(stdout);
        
		printf("%s Disconnected\n", inet_ntoa(client.sin_addr));
		close(new_sd);
	}
	close(sd);
	return(0);
}