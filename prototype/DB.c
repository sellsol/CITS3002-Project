#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>

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
int recvAll(int s, char *out) {
	int len;
	int ret = recv(s, &len, sizeof len, 0); //Get length of message
	if (ret == -1) {
		//Go apeshit
	}
	//len = ntohl(len); Don't know what beej was thinking, this shit just breaks everything
	int cpy = len;
	
	int total = 0;
	char *b = out;

	while (len > 0) {
		int n = recv(s, b, len, 0);
		if (n == -1) {
			return -1;
		}
		b += n;
		len -= n;
	}

	return cpy;
}

int main(void) {
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

	//Here's where we start our loop
	int len = 0;
	while (len != -1) {
		char *msg = malloc(1024);
		int len = recvAll(sockfd, msg);
		printf("%s", msg);
		len = sendAll(sockfd, msg, &len);
	}

	close(sockfd);

	freeaddrinfo(res);

	return 0;
}
