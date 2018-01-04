#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAYLOAD 1024

typedef struct packet packet;
struct packet {
	int  PacketType;
	int  SeqNum;
	char data[PAYLOAD];
	int  WindowSize;
	int  AckNum;
	int  Ackd;
};

typedef struct node node;
struct node {
	packet data;
	node *next;
};

void fillValues(node *n, packet pck) {
	n->data.PacketType = pck.PacketType;
	n->data.AckNum = pck.AckNum;
	n->data.SeqNum = pck.SeqNum;
	n->data.WindowSize = pck.WindowSize;
	strcpy(n->data.data, pck.data);
}

void print(node *lst) {
	node *p;
	for (p = lst; p != 0; p = p->next) {
		printf("%d %d %d \n", p->data.SeqNum, p->data.AckNum, p->data.Ackd);
	}
}

void ack(node *lst, int acknum) {
	node *p;
	for (p = lst; p != 0; p = p->next) {
		if ((p->data.SeqNum + sizeof(p->data.data)) == acknum) {
			p->data.Ackd = 1;
		}
	}
}

node *find(node *lst, const char *name) {
	node *p;
	for (p = lst; p != 0; p = p->next) {
		// Return here
	}
	return 0;
}

void destroy(node *lst) {
	node *p, *q;
	for (p = lst; p != 0; p = q) {
		q = p->next;
		free(p);
	}
}

int insert(node **plst, const char *name, int score) {
	node *newNode = malloc(sizeof(node));
	if (newNode == 0) {
		return 0;
	}
	*plst = newNode;
	return 1;
}

int insert_at_end(node **plst, packet pck) {
	node *newNode = malloc(sizeof(node));
	node **tracer;
	if (newNode == 0) {
		return 0;
	}
	fillValues(newNode, pck);
	for (tracer = plst; (*tracer)->next != 0; tracer = &(*tracer)->next) {
	}
	(*tracer)->next = newNode;
	return 1;
}

int delete_all(node **plst, packet *pck) {
	node **tracer;
	int counter = 0;
	for (tracer = plst; (*tracer) != 0; ) {
		if (((*tracer)->data.SeqNum + sizeof((*tracer)->data.data)) == pck->AckNum) {
			node *p = *tracer;
			*tracer = p->next;
			free(p);
			counter++;
		} else {
			tracer = &(*tracer)->next;
		}
	}
	return counter;
}
