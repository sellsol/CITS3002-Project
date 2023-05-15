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
#include "pquestions.h"
#include "mode.h"

enum PROGRAM PROGRAM_MODE = NONE;

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

int main(int argc, char **argv) {
	PROGRAM_MODE = NONE;
	int c;
	while ((c = getopt(argc, argv, "cp:")) != -1) {
		switch (c) {
			case 'c':
				if (PROGRAM_MODE == NONE) {
					PROGRAM_MODE = C;
					break;
				} else {
					return 1;
				}
			case 'p':
				if (PROGRAM_MODE == NONE) {
					PROGRAM_MODE = PYTHON;
					break;
				} else {
					return 1;
				}
			case '?':
				//Handle appropriately
			default:
				abort();
		}
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

		if (out[0] == 'G') { //Generate questions
			char numQuestions = out[1];
			int64_t seed;
			memcpy(&seed, out + 2, sizeof(int64_t));
			
			//printf("Generate request %d, %016llX\n", numQuestions, &seed);
			int *ids = malloc(numQuestions * sizeof(int));
			int val = question_ids(ids, 'c', numQuestions, seed);
			
			for (char i = 0; i < numQuestions; i++) {
				printf("%i\n", ids[i]);
			}
			//printf("%i\n");		
		} else if (out[0] == 'C') { //Check questions

			//Disects request data
			char questionIndex = out[1];
			int64_t seed;
			memcpy(&seed, out + 2, sizeof(int64_t));
			char lastAttempt = out[10];
			char *answer = out + 11;

			printf("Request to check questions with data %d, %016llX, %d, %s\n", questionIndex, &seed, lastAttempt, answer);

			//Get the question id
			int *ids = malloc(questionIndex * sizeof(int));
			int val = question_ids(ids, 'c', questionIndex + 1, seed);

			printf("Marking question %i...\n", ids[questionIndex]);

			//Gets the question file name
			char* line = a_question(line, C_Q, ids[questionIndex]);
			char* fileName = strtok(line, ",");

			//Compile and test code
			char completed;
			char** ret = compileCode(&completed, fileName, answer, lastAttempt);

			//Send response back
			if (completed == 0) {
				char *msg = calloc(1 + strlen(ret[0]) + 3 + strlen(ret[1]), sizeof(char));
				msg[0] = completed;
				strcat(msg + 1, ret[0]);
				strcat(msg, ";");
				strcat(msg, ret[1]);
				int thing = strlen(msg);
				sendAll(sockfd, msg, &thing);
			} else {
				printf("CORRECT\n");
				int thing = 1;
				sendAll(sockfd, &completed, &thing);
			}


		} else {
			//?????
			printf("Unknown request\n");
		}

		free(out);
	}

	close(sockfd);

	freeaddrinfo(res);

	return 0;
}
