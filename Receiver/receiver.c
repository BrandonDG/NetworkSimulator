/*---------------------------------------------------------------------------------------
--      SOURCE FILE:            receiver.c
--
--      PROGRAM:                receiver.exe
--
--      DATE:                   November 18, 2017
--
--      DESIGNERS:              Brandon Gillespie & Justen DePourcq
--
--      PROGRAMMERS:            Brandon Gillespie
--
--      NOTES:
--      Final Project Receiver Application for COMP7005 by Brandon Gillespie
--      and Justen DePourcq
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
#include "../list.c"

#define SERVER_TCP_PORT 7005     	// Default port
#define BUFLEN	        1024		// Buffer length
#define TRUE	        1
#define PAYLOAD         1024

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
	recv(s, bfrp, btr, MSG_WAITALL);
}

/*-----------------------------------------------------------------------
--	FUNCTION:	buildPacketLogMsgS
--
--	DATE:       December 4, 2017
--
--	DESIGNER:   Brandon Gillespie & Justen DePourcq
--
--  PROGRAMMER:	Brandon Gillespie
--
--	INTERFACE:	void buildPacketLogMsgS(struct packet pck, FILE *f)
--
--	RETURNS:    void
--
--	NOTES:
--	Logs and prints a message for a sent packet.
-----------------------------------------------------------------------*/
void buildPacketLogMsgS(struct packet pck, FILE *f) {
	printf("Sent: PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d\n", pck.PacketType, pck.SeqNum, pck.AckNum, pck.WindowSize);
	fprintf(f, "Sent: PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d\n", pck.PacketType, pck.SeqNum, pck.AckNum, pck.WindowSize);
}

/*-----------------------------------------------------------------------
--	FUNCTION:	buildPacketLogMsgR
--
--	DATE:       December 4, 2017
--
--	DESIGNER:   Brandon Gillespie & Justen DePourcq
--
--  PROGRAMMER:	Brandon Gillespie
--
--	INTERFACE:	void buildPacketLogMsgR(struct packet pck, FILE *f)
--
--	RETURNS:    void
--
--	NOTES:
--	Logs and prints a message for a received packet.
-----------------------------------------------------------------------*/
void buildPacketLogMsgR(struct packet pck, FILE *f) {
	printf("Received: PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d\n", pck.PacketType, pck.SeqNum, pck.AckNum, pck.WindowSize);
	fprintf(f, "Received: PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d\n", pck.PacketType, pck.SeqNum, pck.AckNum, pck.WindowSize);
}

/*-----------------------------------------------------------------------
--	FUNCTION:	setPacketDataValue
--
--	DATE:       November 18, 2017
--
--	DESIGNER:   Brandon Gillespie & Justen DePourcq
--
--  PROGRAMMER:	Brandon Gillespie
--
--	INTERFACE:	void setPacketDataValue(char pa[], char va[], unsigned
--              int pi, int s)
--
--	RETURNS:    void
--
--	NOTES:
--	Loop through char array va and assign values to char array pa
--  for s characters, starting at index pi for pa.
-----------------------------------------------------------------------*/
void setPacketDataValue(char pa[], char va[], unsigned int pi, int s) {
	for (unsigned int i = 0; i < (unsigned int)s; i++) {
		pa[pi++] = va[i];
	}
}

/*-----------------------------------------------------------------------
--	FUNCTION:	buildPacket
--
--	DATE:       November 18, 2017
--
--	DESIGNER:   Brandon Gillespie & Justen DePourcq
--
--  PROGRAMMER:	Brandon Gillespie
--
--	INTERFACE:	void buildPacket(struct packet* pck, char packetdata[])
--
--	RETURNS:    void
--
--	NOTES:
--	Build a packet pck based on packetdata[].
-----------------------------------------------------------------------*/
void buildPacket(struct packet* pck, char packetdata[]) {
	char b[2], b1[7], b2[7], b3[4], b4[BUFLEN];
	pck->PacketType = atoi(strncpy(b, &packetdata[0], 1));
	pck->SeqNum = atoi(strncpy(b1, &packetdata[1], 6));
	pck->AckNum = atoi(strncpy(b2, &packetdata[7], 6));
	pck->WindowSize = atoi(strncpy(b3, &packetdata[13], 3));
	sprintf((*pck).data, "%s", &packetdata[16]);
}

/*-----------------------------------------------------------------------
--	FUNCTION:	buildPacketData
--
--	DATE:       November 18, 2017
--
--	DESIGNER:   Brandon Gillespie & Justen DePourcq
--
--  PROGRAMMER:	Brandon Gillespie
--
--	INTERFACE:	void buildPacketData
--
--	RETURNS:    void
--
--	NOTES:
--	Populate pckdata based on the values in packet pck.
-----------------------------------------------------------------------*/
void buildPacketData(struct packet pck, char* pckdata) {
	char pt[2], seq[7], ack[7], ws[4], pl[BUFLEN];
	char t;
	for (unsigned int i = 0; i <= 16; i++) {
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
			case 7:
				sprintf(ack, "%d", pck.AckNum);
				setPacketDataValue(pckdata, ack, i, 6);
				i += 5;
			break;
			case 13:
				sprintf(ws, "%d", pck.WindowSize);
				setPacketDataValue(pckdata, ws, i, 3);
				i += 2;
			break;
			case 16:
				setPacketDataValue(pckdata, pck.data, i, 2);
			break;
			default:
			break;
		}

	}
}

/*-----------------------------------------------------------------------
--	FUNCTION:	processRevPacket
--
--	DATE:       November 18, 2017
--
--	DESIGNER:   Brandon Gillespie & Justen DePourcq
--
--  PROGRAMMER:	Brandon Gillespie
--
--	INTERFACE:	void processRevPacket(struct packet* ipck, struct 
--              packet* opck)
--
--	RETURNS:    void
--
--	NOTES:
--	Take an input packet and set output packet based on values
-----------------------------------------------------------------------*/
void processRevPacket(struct packet* ipck, struct packet* opck) {
	opck->AckNum = ipck->SeqNum + sizeof(ipck->data);
	opck->SeqNum = 1;
	opck->PacketType = 1;
	opck->WindowSize = 5;
	opck->data[0] = 'A';
	opck->data[1] = '\0';
}

/*-----------------------------------------------------------------------
--	FUNCTION:	main
--
--	DATE:       November 18, 2017
--
--	DESIGNER:   Brandon Gillespie & Justen DePourcq
--
--  PROGRAMMER:	Brandon Gillespie
--
--	INTERFACE:	int main(int argc, char **argv)
--
--	RETURNS:    int
--
--	NOTES:
--	Main.
-----------------------------------------------------------------------*/
int main(int argc, char **argv) {
	int	                sd, new_sd, client_len, port, ws, rc, t;
	struct	sockaddr_in server, client;
	char	            rbuf[BUFLEN], sbuf[BUFLEN];
	struct packet       rpck, spck, pck1, pck2;
	FILE                *f;

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
		
		if ((f = fopen("packetlog", "w")) == 0) {
		    fprintf(stderr, "fopen\n");
		    exit(1);
	    }
		
		struct node *head = malloc(sizeof(node));

		getClientMsg(rbuf, new_sd);
		buildPacket(&pck1, rbuf);
		rc = 1;
		ws = 0;
		t = 1;
		buildPacketLogMsgR(pck1, f);
		fillValues(head, pck1);

		while (t) {
			struct node *head = malloc(sizeof(node));
			
			getClientMsg(rbuf, new_sd);
			buildPacket(&pck1, rbuf);
			
			if (pck1.WindowSize == ws) {
				printf("Sending Ack Again\n");
				fprintf(f, "Sending Ack Again\n");
				
				processRevPacket(&pck1, &spck);
				buildPacketData(spck, sbuf);
				buildPacketLogMsgS(spck, f);
				send(new_sd, sbuf, BUFLEN, 0);
				continue;
			}
			
			++rc;
			
			if (rc != 2) {
				fillValues(head, pck1);
			} else {
				insert_at_end(&head, pck1);
			}
			
			buildPacketLogMsgR(pck1, f);
			
			for (unsigned int i = 1; i < (unsigned int)pck1.WindowSize; i++) {
				getClientMsg(rbuf, new_sd);
				buildPacket(&pck1, rbuf);
				++rc;
				insert_at_end(&head, pck1);
				buildPacketLogMsgR(pck1, f);
				if (pck1.PacketType == 3) {
					t = 0;
				}
				
				ws = pck1.WindowSize;
			}
			for (node *p = head; p != 0; p = p->next) {
				processRevPacket(&p->data, &spck);
				buildPacketData(spck, sbuf);
				buildPacketLogMsgS(spck, f);
				send(new_sd, sbuf, BUFLEN, 0);
			}
		}

        fflush(stdout);

		printf("%s Disconnected\n", inet_ntoa(client.sin_addr));
		close(new_sd);
	}
	close(sd);
	return(0);
}
