/*-----------------------------------------------------------------------
--	SOURCE FILE: transmitter.c
--
--	PROGRAM:     transmitter.exe
--
--	DATE:		 November 18, 2017
--
--	DESIGNERS:	 Brandon Gillespie & Justen DePourcq
--
--	PROGRAMMERS: Brandon Gillespie
--
--	NOTES:
--	Final Project Transmitter Application for COMP7005 by Brandon Gillespie
--  and Justen DePourcq
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
#include <pthread.h>
#include <time.h>
#include <errno.h>

#include "../list.c"

#define SERVER_TCP_PORT 7005	// Default port
#define BUFLEN			1024  	// Buffer length
#define TRUE            1
#define PAYLOAD         1024

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fi = PTHREAD_COND_INITIALIZER;
int runner = 0;

struct thread_parameters {
	struct packet *lspck;
	struct packet *rpck;
	struct node *h;
	char *rbuf;
	int sd;
	FILE *lf;
};

/*-----------------------------------------------------------------------
--	FUNCTION:	setPacketDataValue
--
--	DATE:       November 18, 2017
--
--	DESIGNER:   Brandon Gillespie & Justen DePourcq
--
--  PROGRAMMER:	Brandon Gillespie
--
--	INTERFACE:	void setPacketDataValue(char pa[], char va[], unsigned int
--              pi, int s)
--
--	RETURNS:    void
--
--	NOTES:
--	Goes through char array va and stores the values to pa, for s amount
--  of characters, starting pa at index pi.
-----------------------------------------------------------------------*/
void setPacketDataValue(char pa[], char va[], unsigned int pi, int s) {
	for (unsigned int i = 0; i < s; i++) {
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
--	Builds the packet pck values based on packetdata.
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
--	INTERFACE:	void buildPacketData(struct packet pck, char pckdata[])
--
--	RETURNS:    void
--
--	NOTES:
--	Maps the data from pck to packetdata for transmission.
-----------------------------------------------------------------------*/
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

/*-----------------------------------------------------------------------
--	FUNCTION:	buildPacketLogMsgS
--
--	DATE:       November 22, 2017
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
--	Logs and prints a message for sent packet.
-----------------------------------------------------------------------*/
void buildPacketLogMsgS(struct packet pck, FILE *f) {
	printf("Sent: PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d\n", pck.PacketType, pck.SeqNum, pck.AckNum, pck.WindowSize);
	fprintf(f, "Sent: PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d\n", pck.PacketType, pck.SeqNum, pck.AckNum, pck.WindowSize);
}

/*-----------------------------------------------------------------------
--	FUNCTION:	buildPacketLogMsgR
--
--	DATE:       November 22, 2017
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
--	Logs and prints a message for received packet.
-----------------------------------------------------------------------*/
void buildPacketLogMsgR(struct packet pck, FILE *f) {
	printf("Received: PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d\n", pck.PacketType, pck.SeqNum, pck.AckNum, pck.WindowSize);
	fprintf(f, "Received: PacketType: %d, SeqNum: %d, AckNum: %d, WindowSize: %d\n", pck.PacketType, pck.SeqNum, pck.AckNum, pck.WindowSize);
}

/*-----------------------------------------------------------------------
--	FUNCTION:	getServerMsg
--
--	DATE:       November 18, 2017
--
--	DESIGNER:   Brandon Gillespie & Justen DePourcq
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
	recv(s, bfrp, btr, MSG_WAITALL);
}

/*-----------------------------------------------------------------------
--	FUNCTION:	getServerMsg
--
--	DATE:       November 18, 2017
--
--	DESIGNER:   Brandon Gillespie & Justen DePourcq
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
void processTrnsPacket(struct packet* ipck, struct packet* opck) {
	opck->AckNum = ipck->AckNum;
	opck->SeqNum = ipck->SeqNum + sizeof(ipck->data);
	opck->PacketType = 2;
	opck->WindowSize = ipck->WindowSize;
	opck->data[0] = 'S';
	opck->data[1] = '\0';
	opck->Ackd = 0;
}

/*-----------------------------------------------------------------------
--	FUNCTION:	getAcks
--
--	DATE:       December 2, 2017
--
--	DESIGNER:   Brandon Gillespie & Justen DePourcq
--
--  PROGRAMMER:	Brandon Gillespie
--
--	INTERFACE:	void *getAcks(void *context)
--
--	RETURNS:    void *
--
--	NOTES:
--	Loops through window size to get all acknowledgements. Used in
--  thread.
-----------------------------------------------------------------------*/
void *getAcks(void *context) {
	if(runner == 0) {
		struct thread_parameters *thp = context;
		int oldtype;
		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
		for (unsigned int i = 0; i < thp->lspck->WindowSize+1; i++) {
			getServerMsg(thp->rbuf, thp->sd);
			buildPacket(thp->rpck, thp->rbuf);
			ack(thp->h, thp->rpck->AckNum);
			buildPacketLogMsgR(*(thp->rpck), thp->lf);
		}
		runner =1 ;
		pthread_cond_signal(&fi);
		return NULL;
	} else {
		struct thread_parameters *thp = context;
		int oldtype;
		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
		for (unsigned int i = 0; i < thp->lspck->WindowSize; i++) {
			getServerMsg(thp->rbuf, thp->sd);
			buildPacket(thp->rpck, thp->rbuf);
			ack(thp->h, thp->rpck->AckNum);
			buildPacketLogMsgR(*(thp->rpck), thp->lf);
		}
		runner =1 ;
		pthread_cond_signal(&fi);
		return NULL;
	}
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
--	INTERFACE:	int main (int argc, char **argv)
--
--	RETURNS:    int
--
--	NOTES:
--	main.
-----------------------------------------------------------------------*/
int main (int argc, char **argv) {
	int                sd, port, ws, ac, sc, cut, a;
	struct hostent	   *hp;
	struct sockaddr_in server;
	char               *host, **pptr;
	char               str[16];
	char               sbuf[BUFLEN], tbuf[BUFLEN], rbuf[BUFLEN];
	FILE               *f;

	host =	argv[1];	// Host name
	port =	SERVER_TCP_PORT;
	
	if(argc > 2) {
		a = atoi(argv[2]);
	} else {
		a = 15;
	}

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

	struct packet spck, rpck, lspck, lrpck;
	
	spck.PacketType = 2;
	spck.SeqNum = 1;
	spck.AckNum = 2;
	spck.WindowSize = 5;
	spck.Ackd = 0;
	(spck.data)[0] = 'C';
	(spck.data)[1] = '\0';
	buildPacketData(spck, tbuf);
	buildPacketLogMsgS(spck, f);
	send(sd, tbuf, BUFLEN, 0);
	lspck = spck;
	
	sc = 1;
	
	while (sc < a) {
		struct node *head = malloc(sizeof(node));
		fprintf(stderr, "Window Size: %d \n", lspck.WindowSize);
		fprintf(f, "Window Size: %d \n", lspck.WindowSize);
		for (unsigned int i = 0; i < lspck.WindowSize; i++) {
			processTrnsPacket(&lspck, &spck);
			++sc;
			
			if(sc >= a && (i + 1) == lspck.WindowSize) {
				spck.PacketType = 3;
			}
			
			buildPacketData(spck, tbuf);
			buildPacketLogMsgS(spck, f);
			send(sd, tbuf, BUFLEN, 0);
			lspck = spck;
			
			
			if (i == 0) { 
				fillValues(head, spck);
			}

			if (i != 0 || sc == 2) {
				insert_at_end(&head, spck);
			}
		}

		struct timespec max_wait;
		memset(&max_wait, 0, sizeof(max_wait));
		max_wait.tv_sec = 2;
		
		pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
		pthread_cond_t fi = PTHREAD_COND_INITIALIZER;
		struct timespec abs_time;
		pthread_t tid;
		int err;
		pthread_mutex_lock(&m);
		clock_gettime(CLOCK_REALTIME, &abs_time);
		abs_time.tv_sec += max_wait.tv_sec;
		abs_time.tv_nsec += max_wait.tv_nsec;

		struct thread_parameters thp;
		thp.lspck = &lspck;
		thp.rpck = &rpck;
		thp.rbuf = rbuf;
		thp.sd = sd;
		thp.h = head;
		thp.lf = f;

		pthread_create(&tid, NULL, getAcks, &thp);
		err = pthread_cond_timedwait(&fi, &m, &abs_time);
		pthread_cancel(tid);

		if (err == ETIMEDOUT) {
			node *p;
			for (p = head; p != 0; p = p->next) {
				if (p->data.Ackd == 0) {
					fprintf(stderr, "Timeout Occured\n");
					fprintf(f, "Timeout Occured\n");
					
					buildPacketData(p->data, tbuf);
					send(sd, tbuf, BUFLEN, 0);
					
					getServerMsg(rbuf, sd);
					buildPacket(&rpck, rbuf);
					rpck.Ackd = 1;
					cut = 1;
					buildPacketLogMsgR(rpck, f);
				}
			}
		}
		
		printf("Done waiting\n");
		
		if (!err) {
			pthread_mutex_unlock(&m);
		}
		
		if (cut == 1) {
			lspck.WindowSize = lspck.WindowSize / 2;
			if (lspck.WindowSize == 0) { lspck.WindowSize = 1; };
		} else {
			lspck.WindowSize++;
		}
		cut = 0;
	}
	close(sd);

	return(0);
}
