/*-----------------------------------------------------------------------
--	SOURCE FILE: a1_svr.c
--
--	PROGRAM:     a1svr.exe
--
--	DATE:		 September 26, 2017
--
--	DESIGNERS:	 Brandon Gillespie
--
--	PROGRAMMERS: Brandon Gillespie
--
--	NOTES:
--	A1 Server Application for COMP7005 by Brandon Gillespie
-----------------------------------------------------------------------*/
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>

#define SERVER_TCP_PORT 7005	// Default port
#define BUFLEN			1024  	// Buffer length
#define TRUE            1
#define PAYLOAD         1024
#define PACKETSIZE      524

struct packet {
	int  PacketType;
	int  SeqNum;
	char data[PAYLOAD];
	int  WindowSize;
	int  AckNum;
};

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
}

void buildPacketData(struct packet pck, char pckdata[]) {
	char seq[5], ack[5], ws[4], pl[BUFLEN], pt[2];
	char t;
	for (unsigned int i = 0; i <= 12; i++) {
		switch (i) {
			case 0:
				sprintf(pt, "%d", pck.PacketType);
				pckdata[i] = pt[0];
			break;
			case 1:
				sprintf(seq, "%d", pck.SeqNum);
				setPacketDataValue(pckdata, seq, i, 4);
				i += 3;
			break;
			case 5:
				sprintf(ack, "%d", pck.AckNum);
				setPacketDataValue(pckdata, ack, i, 4);
				i += 3;
			break;
			case 9:
				sprintf(ws, "%d", pck.WindowSize);
				setPacketDataValue(pckdata, ws, i, 3);
				i += 2;
			break;
			case 12:
				setPacketDataValue(pckdata, pck.data, i, 1);
			break;
			default:
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

void buildPacketData(struct packet pck, char pckdata[]) {
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
				setPacketDataValue(pckdata, pck.data, i, 1);
			break;
			default:
			break;
		}

	}
}

void buildPacketLogMsg(struct packet pck, FILE *f) {
	//sprintf("PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d", pck.PacketType, pck.SeqNum, pck.AckNum, pck.WindowSize);
	printf("PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d\n", pck.PacketType, pck.SeqNum, pck.AckNum, pck.WindowSize);
	fprintf(f, "PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d\n", pck.PacketType, pck.SeqNum, pck.AckNum, pck.WindowSize);
}

/*-----------------------------------------------------------------------
--	FUNCTION:	getServerMsg
--
--	DATE:       October 1, 2017
--
--	DESIGNER:   Brandon Gillespie
--
--  PROGRAMMER:	Brandon Gillespie
--
--	INTERFACE:	void getServerMsg(char *bfr, int s)
--
--	RETURNS:    void
--
--	NOTES:
--	Initializes variables and receives input from Server.
--  Stores the results in char *bfr
-----------------------------------------------------------------------*/
void getServerMsg(char *bfr, int s) {
	int m = 0;
	int btr = BUFLEN;
	char *bfrp = bfr;
	while ((m = recv(s, bfrp, btr, 0)) < BUFLEN) {
		bfrp += m;
		btr -= m;
	}
}

void processTrnsPacket(struct packet* ipck, struct packet* opck) {
	//printf("%d\n", sizeof(ipck->data));
	opck->AckNum = ipck->AckNum;
	opck->SeqNum = ipck->SeqNum + sizeof(ipck->data);
	opck->PacketType = 2;
	opck->WindowSize = ipck->WindowSize;
	opck->data[0] = 'S';
	opck->data[1] = '\0';
}

int main (int argc, char **argv) {
	int                sd, port, ws, ac, sc;
	struct hostent	   *hp;
	struct sockaddr_in server;
	char               *host, **pptr;
	char               str[16];
	char               sbuf[BUFLEN], tbuf[BUFLEN], rbuf[BUFLEN];
	FILE               *f;

	host =	argv[1];	// Host name
	port =	SERVER_TCP_PORT;
	
	// Create the socket
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Cannot create socket");
		exit(1);
	}

	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if ((hp = gethostbyname(host)) == NULL) {
		fprintf(stderr, "Unknown server address\n");
		exit(1);
	}
	
	bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);

	// Connecting to the server
	if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1) {
		fprintf(stderr, "Can't connect to server\n");
		perror("connect");
		exit(1);
	}
	printf("Connected:    Server Name: %s\n", hp->h_name);
	pptr = hp->h_addr_list;
	printf("\t\tIP Address: %s\n", inet_ntop(hp->h_addrtype, *pptr, str, sizeof(str)));

	if ((f = fopen("packetlog", "w")) == 0) {
		fprintf(stderr, "fopen\n");
		exit(1);
	}

	//buildPacketData(tbuf, '0', "0001", "0002", "003", "0");
	struct packet spck, rpck, lspck, lrpck;
	spck.PacketType = 2;
	spck.SeqNum = 1;
	spck.AckNum = 2;
	spck.WindowSize = 5;
	(spck.data)[0] = 'C';
	(spck.data)[1] = '\0';
	buildPacketData(spck, tbuf);
	buildPacketLogMsg(spck, f);
	send(sd, tbuf, BUFLEN, 0);
	lspck = spck;

	sc = 1;
	
	while (sc < 15) {

		for (unsigned int i = 0; i < lspck.WindowSize; i++) {
			processTrnsPacket(&lspck, &spck);
			buildPacketData(spck, tbuf);
			buildPacketLogMsg(spck, f);
			send(sd, tbuf, BUFLEN, 0);
			lspck = spck;
			++sc;
		}
		printf("After Window loop : W = %d \n", lspck.WindowSize);

		for (unsigned int i = 0; i < lspck.WindowSize; i++) {
			getServerMsg(rbuf, sd);
			buildPacket(&rpck, rbuf);
			printf("Received) PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d\n", rpck.PacketType, rpck.SeqNum, rpck.AckNum, rpck.WindowSize);
		}
		lspck.WindowSize++;

		/*
		processTrnsPacket(&lspck, &spck);
		buildPacketData(spck, tbuf);
		buildPacketLogMsg(spck, f);
		send(sd, tbuf, BUFLEN, 0);
		lspck = spck;
	
		getServerMsg(rbuf, sd);
		buildPacket(&rpck, rbuf);
		printf("Received) PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d\n", rpck.PacketType, rpck.SeqNum, rpck.AckNum, rpck.WindowSize);
		*/
	}
	//getServerMsg(rbuf, sd);
	//buildPacket(&rpck, rbuf);
	//printf("Received) PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d\n", rpck.PacketType, rpck.SeqNum, rpck.AckNum, rpck.WindowSize);

	/*
	buildPacketData(spck, tbuf);
	buildPacketLogMsg(spck, f);
	send(sd, tbuf, BUFLEN, 0);
	lspck = spck;
	//printf("%s\n", tbuf);
	//printf("%s\n", spck.data);

	// Send the second here, process packet(last send, plus next one to be sent),
	// build the data to be sent, then send
	processTrnsPacket(&lspck, &spck);
	buildPacketData(spck, tbuf);
	buildPacketLogMsg(spck, f);
	send(sd, tbuf, BUFLEN, 0);
	lspck = spck;

	// Wait for ACK packet to be received
	getServerMsg(rbuf, sd);
	buildPacket(&rpck, rbuf);
	printf("Received) PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d\n", rpck.PacketType, rpck.SeqNum, rpck.AckNum, rpck.WindowSize);
	*/

	close(sd);
	return(0);
}

