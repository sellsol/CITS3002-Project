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
#include "mark.h"

enum PROGRAM PROGRAM_MODE = NONE;

//Send all data 'buf' to socket on file descriptor 's'. Buf is of length 'len'
//Returns 0 if all data is sent successfully, else -1
int sendAll(int s, char *buf, int len) {
	int total = 0; //Tracker of the number of bytes sent
	int n = 0; //Temporary variable to store output of send

	n = send(s, &len, sizeof(int), 0); //Send the header
	//TODO: Ensure the header will be completely sent
	
	while (total < len) {
		n = send(s, buf + total, len - total, 0); //Send all data 
		if (n == -1) {
			return -1; //Quit the function on an error
		}

		total += n;
	}

	return 0;
}

//Receive an entire message from socket on file descriptor 's'
char* recvAll(int s) {
	int len; //Header describing the length of the message
	int ret = recv(s, &len, sizeof(len), 0); //Get length of message
	if (ret == -1) {
		return NULL;
	}
	char *msg = malloc(len); //Error handle here
	//len = ntohl(len); Don't know what beej was thinking, this shit just breaks everything
	
	char *index = msg; //Create an index to trawl across allocated memory

	//While we haven't read the entire message yet
	while (len > 0) {
		ret = recv(s, index, len, 0); //Read data
		if (ret == -1) {
			return NULL;
		}
		index += ret;
		len -= ret;
	}

	return msg; //TODO: Return length of the message with message
}

int main(int argc, char **argv) {

	//Use argv to determine QB mode
	int c;
	while ((c = getopt(argc, argv, "c:p")) != -1) {
		switch (c) {
			case 'c':
				if (PROGRAM_MODE == NONE) {
					PROGRAM_MODE = C;
					q_path = c_path;
					break;
				} else {
					printf("Usage: QB -[c|p]\n");
					exit(EXIT_FAILURE);
				}
			case 'p':
				if (PROGRAM_MODE == NONE) {
					PROGRAM_MODE = PYTHON;
					q_path = py_path;
					break;
				} else {
					printf("Usage: QB -[c|p]\n");
					exit(EXIT_FAILURE);
				}
			case '?':
				printf("Usage: QB -[c|p]\n");
				exit(EXIT_FAILURE);
			default:
				abort();
		}
	}

	if (PROGRAM_MODE == NONE) {
		printf("Usage: QB -[c|p]\n");
		exit(EXIT_FAILURE);
	}

	//Socket setup
	int status, sockfd;
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof hints); //Allocate memory for hints
									//NOTE: I'm not gonna error handle every goddamn malloc that cross my path- we'll be here all day
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	//Initialise socket data
	if ((status = getaddrinfo("127.0.0.1", "65432", &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(EXIT_FAILURE);
	}
	//Feels like we should free hints and res after this

	//Open socket
	if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
		perror("client: socket");
		exit(EXIT_FAILURE);
	}

	//Connect to server
	if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
		close(sockfd);
		perror("client: connect");
		exit(EXIT_FAILURE);
	}

	//SetSockOpt should be around here somewhere

	printf("SUCCESSFULLY CONNECTED TO SERVER\n");

	//Here's where we start our loop
	int len = 0;
	char *msg;
	while (len != -1) {
		msg = recvAll(sockfd);

		//Possibly make this a switch?
		if (msg[0] == 'G') { //Generate questions request
			char numQuestions = msg[1];
			uint64_t seed;
			memcpy(&seed, msg + 2, sizeof(uint64_t));
			char *output = get_questions(seed, numQuestions);
			sendAll(sockfd, output, strlen(output));

			free(output);
			
		} else if (msg[0] == 'C') { //Check questions request

			//Disects request data
			char questionIndex = msg[1];
			uint64_t seed;
			memcpy(&seed, msg + 2, sizeof(uint64_t));
			char lastAttempt = msg[10];
			char *answer = msg + 11;

			printf("Request to check questions with data %d, %016llX, %d, %s\n", questionIndex, &seed, lastAttempt, answer);

			struct FileData response = question_correct(seed, questionIndex, lastAttempt, answer);
			sendAll(sockfd, response.data, response.len);
			free(response.data);
			/* OLD CODE IN STORAGE
			//Get the question id
			int *ids = malloc(questionIndex * sizeof(int));
			int val = question_ids(ids, 'c', questionIndex + 1, seed);

			printf("Marking question %i...\n", ids[questionIndex]);

			//Gets the question file name
			char* line = a_question(C_Q, ids[questionIndex]);
			char* questionFile = strtok(line, ",");
			
			//Cleans up some loose memory from disecting request data
			free(ids);

			//Compile and test code
			char completed;
			struct FileData* ret = compileCode(&completed, questionFile, answer, lastAttempt);

			//Send response back
			if (completed == 0) {
				//Formulate incorrect response
				//Template: 0 + expectedOutput + ';' + output

				int responseLen = sizeof(char) + strlen(ret[0]) + 3 + strlen(ret[1]);
				char *response = calloc(responseLen, sizeof(char));
				snprintf(response, responseLen, "%c%s;%s", completed, ret[0], ret[1]);

				//Send response
				printf("WRONG: SENDING %s\n", response);
				sendAll(sockfd, response, responseLen);
				free(response);
			} else {
				printf("CORRECT\n"); //Actually, this will trigger even if we have some kind of failure on our side
				sendAll(sockfd, &completed, sizeof(char));
			}*/


		} else {
			printf("Unknown request: %s\n", msg);
			//We should send some kind of clarification code on this one
		}

		free(msg);
	}
}