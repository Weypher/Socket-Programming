#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <string>
#include <map>
#include <iostream>
#include <climits>

using namespace std;

#define HOST "localhost"
#define UDP_A "21994"			// UDP for serverA
#define UDP_B "22994"			// UDP for serverB
#define UDP_AWS "23994"			// UDP for AWS
#define TCP_CLIENT "24994"		// TCP for client
#define BACKLOG 10				// how many pending connections queue will hold
#define MAXLINESIZE 1024

char buf[MAXLINESIZE];
char bufForB[MAXLINESIZE];
char* resFromA;
char* resFromB;
double propagation_speed;
double transmission_speed;
char* finalRes;
char finalBuf[MAXLINESIZE];


// get socket address, IPv4 or IPv6 --- code from Beej's Documentation
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)-> sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)-> sin6_addr);
}


int sendClientRequesToServerAThenGetReply(char MapID, int node, char* resFromA) {
	// Set up UDP --- code from Beej's Documentation
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char * serverPort;
	serverPort = UDP_A;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(HOST, serverPort, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("talker: socket\n");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "talker: failed to bind socket\n");
		exit(1);
	}
	sendto(sockfd, (char *)&MapID, sizeof MapID, 0, p->ai_addr, p->ai_addrlen);
	sendto(sockfd, (char *)&node, sizeof node, 0, p->ai_addr, p->ai_addrlen);
	cout << "The AWS has sent map ID and starting vertex to server A using UDP over port <" << UDP_A << ">" << endl;
	cout << "-------------------------------------------------------------" << endl;

	recvfrom(sockfd, (char *)&propagation_speed, sizeof propagation_speed, 0, NULL, NULL);
	recvfrom(sockfd, (char *)&transmission_speed, sizeof transmission_speed, 0, NULL, NULL);
	// cout << "prop: " << propagation_speed << "      trans: " << transmission_speed << endl;
	//2: received from udp1
    int numreceived;
    if ((numreceived = recvfrom(sockfd, resFromA, MAXLINESIZE - 1, 0, p->ai_addr, &p->ai_addrlen)) == -1) {
            perror("recvfrom");
            exit(1);
    }
    // if ((numreceived = recvfrom(sockfd, resFromA, MAXLINESIZE - 1, 0, NULL, NULL) == -1)) {
    //         perror("recvfrom");
    //         exit(1);
    // }
	return 0;
}



int sendResToServerBAndGetRely(double propagation_speed, double transmission_speed, char* resFromA, long long fsize, char* resFromB, char* finalRes) {
	// Set up UDP --- code from Beej's Documentation
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(HOST, UDP_B, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("talker: socket\n");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "talker: failed to bind socket\n");
		exit(1);
	}

	

	sendto(sockfd, (char *)&fsize, sizeof fsize, 0, p->ai_addr, p->ai_addrlen);
	sendto(sockfd, (char *)&propagation_speed, sizeof propagation_speed, 0, p->ai_addr, p->ai_addrlen);
	sendto(sockfd, (char *)&transmission_speed, sizeof transmission_speed, 0, p->ai_addr, p->ai_addrlen);
	sendto(sockfd, buf, MAXLINESIZE - 1, 0,  p->ai_addr, p->ai_addrlen);
	// cout << "game start" << endl;
	// cout << transmission_speed << endl;
	cout << "The AWS has sent path length, propagation speed and transmission speed to server B using UDP over port <" << UDP_B << ">." << endl;
	// cout << "game over" << endl;

	int numreceived;
    if ((numreceived = recvfrom(sockfd, bufForB, MAXLINESIZE - 1, 0, p->ai_addr, &p->ai_addrlen)) == -1) {
            perror("recvfrom");
            exit(1);
    }

    if ((numreceived = recvfrom(sockfd, finalBuf, MAXLINESIZE - 1, 0, p->ai_addr, &p->ai_addrlen)) == -1) {
            perror("recvfrom");
            exit(1);
    }
	// printf("The AWS recieved outputs from Backend-Server C using UDP over port <%s>\n", UDP_B);
	return 0;
}


int main() {
	printf("The AWS is up and running.\n");
	cout << "-------------------------------------------------------------" << endl;


	// Set up TCP connection for Client--- code from Beej's Documentation
	int sockfd, client_sock;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;	// use my IP

	if ((rv = getaddrinfo(HOST, TCP_CLIENT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and connect to the first we can --- code from Beej's Documentation
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket\n");
			//continue;
			exit(1);
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt\n");
			exit(1);
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind\n");
			//continue;
			exit(1);
		}
		break;
	}

	freeaddrinfo(servinfo);	// all done with this structure
	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}
	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen\n");
		exit(1);
	}

	while (1) {
		sin_size = sizeof their_addr;
		client_sock = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (client_sock == -1) {
			perror("accept\n");
			//continue;
			exit(1);
		}

		// recieve inputs from client
		char MapID;
		int node;
		long long fsize;
		recv(client_sock, (char *)&MapID, sizeof (MapID), 0);
		recv(client_sock, (char *)&node, sizeof node, 0);
		recv(client_sock, (char *)&fsize, sizeof fsize, 0);
		cout << "The AWS has received map ID <" << MapID << ">, start vertex <" << node << "> and file size <" << fsize 
		<< "> from the client using TCP over port number <" << TCP_CLIENT << ">" << endl;
		cout << "-------------------------------------------------------------" << endl;
		// send to serverA
		resFromA = buf;
		sendClientRequesToServerAThenGetReply((char)MapID, node, resFromA);
		cout << "The AWS has received shortest path from server A:" << endl;
		cout << "-------------------------------------------------------------" << endl;
		cout << "Destination            Min Length" << endl;
		cout << "-------------------------------------------------------------" << endl;
		cout << resFromA;
		cout << "-------------------------------------------------------------" << endl;

		// send to B and get res From B
		finalRes = finalBuf;
		resFromB = bufForB;
		sendResToServerBAndGetRely(propagation_speed, transmission_speed, resFromA, fsize, resFromB, finalRes);
		cout << "The AWS has received delays from server B:" << endl;
		cout << "-------------------------------------------------------------" << endl;
		cout << "Destination Tt Tp Delay" << endl;
		cout << "-------------------------------------------------------------" << endl;
		cout << resFromB;
		cout << "-------------------------------------------------------------" << endl;
		// cout << "The AWS has sent calculated delay to client using TCP over port <" << TCP_CLIENT << ">." << endl;

		memset(bufForB, 0, sizeof(bufForB) / sizeof(char));
		// send(client_sock, bufForB, MAXLINESIZE - 1, 0);
		send(client_sock, finalBuf, MAXLINESIZE - 1, 0);
		cout << "The AWS has sent calculated delay to client using TCP over port <" << TCP_CLIENT << ">." << endl;
		
		// cout << finalBuf << endl;
		memset(finalBuf, 0, sizeof(finalBuf) / sizeof(char));
		// cout << finalBuf << endl;
		close(client_sock);
		// cout << "The AWS has sent calculated delay to client using TCP over port <" << TCP_CLIENT << ">." << endl;

	}
	return 0;
}