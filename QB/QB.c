#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <stdint.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "questions.h"
char prog_lang;

//sendAll Shamelessly stolen from beej's guide to networking
int sendAll(int s, char *buf, int *len) {
	int total = 0;
	int bytesLeft = *len;
	int n;
	
	while (total < *len) {
		n = send(s, buf + total, bytesLeft, 0);
		if (n == -1) {
			return -1;
		}

		total += n;
		bytesLeft -= n;
	}

	*len = total;
	return 0;
}

//out should be length 1024 for testing, i'll come up with something better later
char* recvAll(int s) {
	int len;
	int ret = recv(s, &len, sizeof len, 0); //Get length of message
	if (ret == -1) {
		//Go apeshit
	}
	char *out = malloc(len); //Error handle here
	//len = ntohl(len); Don't know what beej was thinking, this shit just breaks everything
	int cpy = len;
	
	int total = 0;
	char *b = out;

	while (len > 0) {
		int n = recv(s, b, len, 0); //Error handle here
		if (n == -1) {
			return NULL;
		}
		b += n;
		len -= n;
	}

	return out;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("error: wrong number of parameters\n");
		exit(0);
	}
	prog_lang = argv[1][0];
    if(prog_lang != 'c' && prog_lang != 'p'){
        printf("QB does not exist - choose from 'c' and 'p'\n");
        exit(0);
    }

	int status, sockfd;
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((status = getaddrinfo("127.0.0.1", "65432", &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
		perror("client: socket");
		exit(1);
	}

	if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
		close(sockfd);
		perror("client: connect");
		exit(1);
	}

	printf("SUCCESSFULLY CONNECTED TO SERVER\n");

	//Here's where we start our loop
	int len = 0;
	char *out;
	while (len != -1) {
		out = recvAll(sockfd);
		
		if (strlen(out) == 0) {
			printf("Connection to server lost.\n");
			printf("Exiting program, rerun to reconnect\n");
			close(sockfd);
			free(out);
			freeaddrinfo(res);
			return(0);
		} 
		else if (out[0] == 'G') { //Generate questions
			char numQuestions = out[1];
			int64_t seed;
			memcpy(&seed, out + 2, sizeof(int64_t));
			printf("num questions = %i, seed = %li\n", numQuestions, seed);//intesting
		
			char *buf = genQuestionsReply(numQuestions, seed);
			int length = strlen(buf);
			sendAll(sockfd, buf, &length);//intesting
			printf("Sending reply: %s\n", buf);//intesting
		} 
		else if (out[0] == 'C') { //Check questions
			char questionIndex = out[1];
			int64_t seed;
			memcpy(&seed, out + 2, sizeof(int64_t));
			char lastAttempt = out[10];
			char *answer = out + 11;
			printf("seed index = %i, seed = %li, is last attempt = %i, answer = %s\n", 
				questionIndex, seed, lastAttempt, answer);//intesting
			
			
			char buf[] = "heyo! Request C received!"; //intesting
			int length = strlen(buf);
			sendAll(sockfd, buf, &length);//intesting
			printf("Sending reply: %s\n", buf);//intesting
		} 
		else { //Should not happen
			printf("Unknown request: %s\n", out);//intesting
		}

	}
}
