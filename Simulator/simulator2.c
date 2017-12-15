/*---------------------------------------------------------------------------------------
--      SOURCE FILE:            simulator.c
--
--      PROGRAM:                sim.exe
--
--      DATE:                   November 29, 2017
--
--      DESIGNERS:              Brandon Gillespie & Justen DePourcq
--
--      PROGRAMMERS:            Justen DePourcq
--
--      NOTES:
--         Network Simulator for COMP7005 Project.
---------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#include "../list.c"

#define SERVER_TCP_PORT 7005     	// Default port
#define BUFLEN	        1024		// Buffer length
#define TRUE	        1
#define PAYLOAD         1024

/*-----------------------------------------------------------------------
--	FUNCTION:	getClientMsg
--
--	DATE:       November 29, 2017
--
--	DESIGNER:   Brandon Gillespie & Justen DePourcq
--
--  PROGRAMMER:	Justen DePourcq
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
	int btr = BUFLEN;
	char *bfrp = bfr;
	recv(s, bfrp, btr, MSG_WAITALL);
}

/*-----------------------------------------------------------------------
--	FUNCTION:	buildPacketLogMsg
--
--	DATE:       November 29, 2017
--
--	DESIGNER:   Brandon Gillespie & Justen DePourcq
--
--  PROGRAMMER:	Justen DePourcq
--
--	INTERFACE:	void buildPacketLogMsg(struct packet pck)
--
--	RETURNS:    void
--
--	NOTES:
--	Prints the packet information to the screen
-----------------------------------------------------------------------*/
void buildPacketLogMsg(struct packet pck) {
	
	printf("PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d\n", pck.PacketType, pck.SeqNum, pck.AckNum, pck.WindowSize);
	
}

/*-----------------------------------------------------------------------
--	FUNCTION:	setPacketDataValue
--
--	DATE:       November 29, 2017
--
--	DESIGNER:   Brandon Gillespie & Justen DePourcq
--
--  PROGRAMMER:	Justen DePourcq
--
--	INTERFACE:	void setPacketDataValue(char pa[], char va[], unsigned int pi, int s)
--
--	RETURNS:    void
--
--	NOTES:
--	Copies the value of one array to another array
-----------------------------------------------------------------------*/
void setPacketDataValue(char pa[], char va[], unsigned int pi, int s) {
	for (unsigned int i = 0; i < (unsigned int)s; i++) {
		pa[pi++] = va[i];
	}
}

/*-----------------------------------------------------------------------
--	FUNCTION:	buildPacket
--
--	DATE:       November 29, 2017
--
--	DESIGNER:   Brandon Gillespie & Justen DePourcq
--
--  PROGRAMMER:	Justen DePourcq
--
--	INTERFACE:	void buildPacket(struct packet* pck, char packetdata[])
--
--	RETURNS:    void
--
--	NOTES:
--	Fills the packet struct with the packet information
-----------------------------------------------------------------------*/
void buildPacket(struct packet* pck, char packetdata[]) {
	char b[2], b1[7], b2[7], b3[4];
	pck->PacketType = atoi(strncpy(b, &packetdata[0], 1));
	pck->SeqNum = atoi(strncpy(b1, &packetdata[1], 6));
	pck->AckNum = atoi(strncpy(b2, &packetdata[7], 6));
	pck->WindowSize = atoi(strncpy(b3, &packetdata[13], 3));
	sprintf((*pck).data, "%s", &packetdata[16]);
}

/*-----------------------------------------------------------------------
--	FUNCTION:	buildPacketData
--
--	DATE:       November 29, 2017
--
--	DESIGNER:   Brandon Gillespie & Justen DePourcq
--
--  PROGRAMMER:	Justen DePourcq
--
--	INTERFACE:	void buildPacketData(struct packet pck, char* pckdata)
--
--	RETURNS:    void
--
--	NOTES:
--	Builds packet for sending
-----------------------------------------------------------------------*/
void buildPacketData(struct packet pck, char* pckdata) {
	char pt[2], seq[7], ack[7], ws[4];

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
--	FUNCTION:	processRedirectPacket
--
--	DATE:       November 29, 2017
--
--	DESIGNER:   Brandon Gillespie & Justen DePourcq
--
--  PROGRAMMER:	Justen DePourcq
--
--	INTERFACE:	void processRedirectPacket(struct packet* ipck, struct packet* opck)
--
--	RETURNS:    void
--
--	NOTES:
--	Copy the information from one packet to another
-----------------------------------------------------------------------*/
void processRedirectPacket(struct packet* ipck, struct packet* opck) {
	opck->AckNum = ipck->AckNum;
	opck->SeqNum = ipck->SeqNum;
	opck->PacketType = ipck->PacketType;
	opck->WindowSize = ipck->WindowSize;
	memcpy(ipck->data, opck->data, sizeof(ipck->data));
}

/*-----------------------------------------------------------------------
--	FUNCTION:	main
--
--	DATE:       November 29, 2017
--
--	DESIGNER:   Brandon Gillespie & Justen DePourcq
--
--  PROGRAMMER:	Justen DePourcq
--
--	INTERFACE:	void main(int argc, char **argv)
--
--	RETURNS:    void
--
--	NOTES:
--	Listens for Transmitter connection, Creates connection with Receiver
--  Receives and sends data packets, Receives and sends Ack packets
--  Creates random dropping rate for packets.
-----------------------------------------------------------------------*/
int main(int argc, char **argv) {
	int	                sd, sd2, new_sd, client_len, port, ws, rc, port2, fl, t, a, num;
	struct	sockaddr_in server, client;
	char	            rbuf[BUFLEN], sbuf[BUFLEN];
	struct packet       spck, pck1;
	char * host, **pptr;
	struct hostent	   *hp;
	char               str[16];
	char			   ran[9];
	int rchk = 0;
	
	srand(time(NULL));
	
	if(argc > 2) {
		a = atoi(argv[2]);
	} else {
		a = 1;
	}
	

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
	
	//Create connection with receiver side
	
	host =	argv[1];	// Host name
	port2 =	SERVER_TCP_PORT;

	// Create the socket
	if ((sd2 = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Cannot create socket");
		exit(1);
	}

	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port2);
	if ((hp = gethostbyname(host)) == NULL) {
		fprintf(stderr, "Unknown server address\n");
		exit(1);
	}

	bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);

	// Connecting to the server
	if (connect(sd2, (struct sockaddr *)&server, sizeof(server)) == -1) {
		fprintf(stderr, "Can't connect to server\n");
		perror("connect");
		exit(1);
	}
	printf("Connected:    Server Name: %s\n", hp->h_name);
	pptr = hp->h_addr_list;
	printf("\t\tIP Address: %s\n", inet_ntop(hp->h_addrtype, *pptr, str, sizeof(str)));


	while (t) {
		client_len = sizeof(client);
		if ((new_sd = accept(sd, (struct sockaddr *)&client, &client_len)) == -1) {
			fprintf(stderr, "Can't accept client\n");
			exit(1);
		}

		printf(" Remote Address:  %s\n", inet_ntoa(client.sin_addr));

		getClientMsg(rbuf, new_sd);
		buildPacket(&pck1, rbuf);
		printf("Data: \n");
		rc = 1;
		ws = 0;
		fl = 1;
		t = 1;
		buildPacketLogMsg(pck1);

		while (t) {
			struct node *head = malloc(sizeof(node));
			
			if (rc == 1) {
				fillValues(head, pck1);
			}
			
			getClientMsg(rbuf, new_sd);
			buildPacket(&pck1, rbuf);
			
			if (pck1.WindowSize == ws) {
				buildPacketData(pck1, sbuf);
				printf("Retransmitted Packet: ");
				buildPacketLogMsg(pck1);
				send(sd2, sbuf, BUFLEN, 0);
				
				getClientMsg(rbuf, sd2);
				buildPacket(&pck1, rbuf);
				
				buildPacketData(pck1, sbuf);
				printf("Retransmit Ack: ");
				buildPacketLogMsg(pck1);
				send(new_sd, sbuf, BUFLEN, 0);
				continue;
			}
			
			if (rc != 1) {
				fillValues(head, pck1);
			} else {
				insert_at_end(&head, pck1);
			}
			
			ws = pck1.WindowSize;
			
			++rc;
			
			printf("%s\n", "Data: ");
			for (unsigned int i = 1; i < (unsigned int)pck1.WindowSize; i++) {
				getClientMsg(rbuf, new_sd);
				buildPacket(&pck1, rbuf);
				++rc;
				
				if (pck1.PacketType == 3) { t = 0; }
				
				insert_at_end(&head, pck1);
			}
			
			if(pck1.PacketType == 2 || pck1.PacketType == 3) {
				
				for (node *p = head; p != 0; p = p->next) {
					processRedirectPacket(&p->data, &spck);
					buildPacketData(spck, sbuf);
					buildPacketLogMsg(spck);
					send(sd2, sbuf, BUFLEN, 0);
				}
			}
			
			
			printf("%s\n", "ACK: ");
			
			struct node *head2 = malloc(sizeof(node));
			for (unsigned int i = 0; i < ws + fl; i++) {
				getClientMsg(rbuf, sd2);
				buildPacket(&pck1, rbuf);
				if (i == 0) {
					fillValues(head2, pck1);
				} else {
					insert_at_end(&head2, pck1);
				}
			}
			fl = 0;
			
			for (node *p = head2; p != 0; p = p->next) {
				rchk = 0;
				processRedirectPacket(&p->data, &spck);
				buildPacketData(spck, sbuf);
				if(t != 0 ) {
					for(int j = 0; j < a; j++) {
						num = rand() % 10;
						ran[j] = num;
					}
					for(int i = 0; i < a; i++) {
						if(ran[i] == 5) {
							rchk = 1;
							break;
						}
					}
				}
				if(rchk != 1) {
					buildPacketLogMsg(spck);
					send(new_sd, sbuf, BUFLEN, 0);
				}
			}
		}


        fflush(stdout);

		printf("%s Disconnected\n", inet_ntoa(client.sin_addr));
		close(new_sd);
		close(sd2);
	}
	close(sd);
	return(0);
}
