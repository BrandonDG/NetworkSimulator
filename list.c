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
		//if (acknum == 19457) { p->data.Ackd = 0; }
	}
}

node *find(node *lst, const char *name) {
	node *p;
	for (p = lst; p != 0; p = p->next) {
		/*
		if (strcmp(p->data.name, name) == 0) {
			printf("%s %d\n", p->data.name, p->data.score);
			return p;
		} */
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
	//strcpy(newNode->data.name, name);
	//newNode->data.score = score;
	//newNode->next = *plst;
	*plst = newNode;
	return 1;
}

/*
int insert_in_order(node **plst, const char *name, int score) {
	node *newNode = malloc(sizeof(node));
	node **tracer;
	if (newNode == 0) {
		return 0;
	}
	newNode->data.score = score;
	strcpy(newNode->data.name, name);
	for (tracer = plst; *tracer != 0; tracer = &(*tracer)->next) {
		if ((*tracer)->data.score >= score) {
			break;
		}
	}
	newNode->next = *tracer;
	*tracer = newNode;
	return 1;
} */

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

/*
int main(void) {
	int n;

	struct record r1;
	struct record r2;
	struct record r3;
	struct record r4;
	struct node *nd1 = malloc(sizeof(node));
	struct node *nd2 = malloc(sizeof(node));
	struct node *nd3 = malloc(sizeof(node));
	struct node *nd4 = malloc(sizeof(node));

	//struct node *head = malloc(sizeof(node));

	char n1[32] = "Brandon Gillespie";
	char n2[32] = "Ryan Williams";
	char n3[32] = "Sean Mackay";
	char n4[32] = "Brandon Gillespie";

	strcpy(r1.name, n1);
	r1.score = 75;
	strcpy(r2.name, n2);
	r2.score = 85;
	strcpy(r3.name, n3);
	r3.score = 95;
	strcpy(r4.name, n4);
	r4.score = 50;

	nd1->data = r1;
	nd2->data = r2;
	nd3->data = r3;
	nd4->data = r4;

	//head = nd1;

	//nd1->next = malloc(sizeof(node));
	//nd2->next = malloc(sizeof(node));
	//nd3->next = NULL;

	nd1->next = nd2;
	nd2->next = nd3;
	nd3->next = nd4;

	insert_at_end(&nd1, "Craig Foris", 100);

	print(nd1);

	find(nd1, n2);

	//n = delete_all(&nd1, "Sean Mackay");
	n = delete_second(&nd1, n1);
	printf("%lu\n", n);
	printf("\n");
	print(nd1);

	destroy(nd1);

	return 0;
} */
