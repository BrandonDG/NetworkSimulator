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

/*-----------------------------------------------------------------------
--	FUNCTION:	fixFilename 
--
--	DATE:       October 1, 2017
--
--	DESIGNER:   Brandon Gillespie
--
--  PROGRAMMER:	Brandon Gillespie
--
--	INTERFACE:	void getFilename(char *fn)
--
--	RETURNS:    void
--
--	NOTES:
--	Replaces newline character with null character at end of filename
-----------------------------------------------------------------------*/
void fixFilename(char *fn) {
	char *ch;
	if ((ch = strchr(fn, '\n')) != NULL) { 
		*ch = '\0';
	}
}

int main(int argc, char **argv) {
	int	                ca;
	int	                sd, new_sd, client_len, port;
	struct	sockaddr_in server, client;
	char	            *bp, buf[BUFLEN], fbuf[BUFLEN], rbuf[BUFLEN], cbuf[BUFLEN];
	FILE                *f;
	char                filename[32];

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
		*buf = '-';
		while (*buf != 'Q') {

			// Waiting for user input.
			getClientMsg(buf, new_sd);

			// Use user input for switch.
			// A means client is downloading file
			// B means client is uploading file
			// Q means client is quitting
			switch (*buf) {
				case 'A':
					// Waiting for filename
					getClientMsg(buf, new_sd);
					sprintf(filename, buf);
					printf("%s", filename);
					fixFilename(filename);
					bp = buf;

					// Try to open file
					if ((f = fopen(filename, "r")) == 0) {
						fprintf(stderr, "fopen\n");
						*bp = 'N';
						// Send file open result
						send(new_sd, buf, BUFLEN, 0);
						continue;
					}
					*bp = 'E';
					// Send file open result
					send(new_sd, buf, BUFLEN, 0);
					bp = fbuf;
					while ((ca = fgetc(f)) != EOF) {
						(*bp) = ca;
						bp++;
					}
					(*bp) = '\0';
					// Send file
					send(new_sd, fbuf, BUFLEN, 0);
					fclose(f);
				break;
				case 'B':
					// Waiting for filename
					getClientMsg(buf, new_sd);
					// Waiting for file open result
					getClientMsg(cbuf, new_sd);
	
					bp = cbuf;
					if (*cbuf == 'N') {
						printf("Client file does not exist\n");
						continue;
					}
					sprintf(filename, buf);
					printf("%s", filename);
					fixFilename(filename);
					// Waiting for file
					getClientMsg(rbuf, new_sd);
					// Try to open file
					if ((f = fopen(filename, "w")) == 0) {
						fprintf(stderr, "fopen\n");
						exit(1);
					}
					fputs(rbuf, f);
					printf("File downloaded.");
					// Close file
					fclose(f);
					fflush(stdout);
				break;
				case 'Q':
				break;
			} 
		}
		printf("%s Disconnected\n", inet_ntoa(client.sin_addr));
		close(new_sd);
	}
	close(sd);
	return(0);
}


