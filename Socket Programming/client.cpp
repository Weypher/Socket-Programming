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
#include <vector>

using namespace std;

#define HOST "localhost"
#define AWS_TCP "24994"
#define MAXLINESIZE 1024

char buf1[MAXLINESIZE];
char buf2[MAXLINESIZE];
// get socket address, IPv4 or IPv6 --- code from Beej's Documentation
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)-> sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)-> sin6_addr);
}




// void getResult(char* resFromA, char* resFromB) {

// 	//split data from A and B
//     char data[MAXLINESIZE];
//     strcpy(data, resFromB);
//     cout << "=====================================" << endl;
// 	int curSum = 0;
// 	int node = 0;
// 	int info = 0;
// 	bool isNode = true;
// 	map<int, vector<int> > resMap;
// 	for(int i = 0;i < MAXLINESIZE; i++){
// 		vector<int> temp;
// 		if (data[i] == '\0') {
// 			break;
// 		}
// 		if (data[i] >= 48 && data[i] <= 57) {
// 			curSum = curSum * 10 + (int)(data[i] - '0');
// 		}
// 		if (data[i] == '\t'&& isNode) {
// 			node = curSum;
// 			curSum = 0;
// 			isNode = false;
// 			cout << "node is " << node << " info is: ";
// 			// resMap.insert(pair<int, vector<int>>{node, NULL});
// 		} else if (data[i] == '\t' && !isNode) {
// 			info = curSum;
// 			curSum = 0;
// 			temp.push_back(info);
// 			cout << info << " ";
// 		} else if (data[i] == '\n') {
// 			info = curSum;
// 			curSum = 0;
// 			temp.push_back(info);
// 			isNode = true;
// 			cout << info << endl;
// 			resMap.insert(pair<int, vector<int> >{node, temp});
// 		}
// 		// cout << resMap[node].at(0) << " " << resMap[node].at(1) << " " << resMap[node].at(2) << endl;

// 		// else if (data[i] == '\n') {
// 		// 	info = curSum;
// 		// 	cout << "* Path length for destination <" << node << ">:" << "<" << dis << ">" << endl;
// 		// 	mapsInfo.insert(pair<int,int>(node,dis));
// 		// 	mapKey.push_back(node);
// 		// 	curSum = 0;
// 		// 	node = 0;
// 		// 	dis = 0;
// 		// }
// 	}
// }






int main(int argc, char* argv[]) {
	// Check is MapID, size, power is provided by the user.
	if (argc <= 1) {
		printf("You didn't feed me (MapID, node, fsize)... I will die now :( \n");
		exit(1);
	} else if (argc > 1 && argc < 3) {
		printf("You miss something in (MapID, node, fsize)... I will die now :( \n");
		exit(1);
	}
	char* MapID = argv[1];
	int node = atoi(argv[2]);
	long long fsize = atoi(argv[3]);

	// Set up TCP connection --- code from Beej's Documentation
	int sockfd = 0;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(HOST, AWS_TCP, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can --- code from Beej's Documentation
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket\n");
			continue;
		}
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect\n");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	freeaddrinfo(servinfo);	// all done with this structure
	printf("The client is up and running. \n");

	send(sockfd, MapID, strlen(MapID), 0);	// sends <input> to AWS
	send(sockfd, (char *)&node,  sizeof node, 0);
	send(sockfd, (char *)&fsize, sizeof fsize, 0);
	cout << "The client has sent query to AWS using TCP over port <" << AWS_TCP << ">: start vertex <" << node << ">; map <" << MapID << ">; file size <" << fsize << ">." << endl;
	// cout << "The client has sent map ID <" << MapID << ">, start vertex <" << node << "> and file size <" << fsize 
	// 	<< ">"  << endl;


	char* resFromA = buf1;
	recv(sockfd, buf1, 1023, 0);
    cout << "=====================================" << endl;
    cout << "The client has received results from AWS:" << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "Destination  Min length  Tt     Tp       Delay" << endl;
    cout << "--------------------------------------------------" << endl;
	cout << resFromA << endl;
	// char* resFromB = buf2;
	// recv(sockfd, buf2, 1023, 0);
	// cout << resFromB << endl;
	// getResult(resFromA, resFromB);







	// int result;
	// recv(sockfd, (char *)&result, sizeof result, 0);	// recieve result of propagatin delay for specific link
	// if (result == 0) {
	// 	printf("Found no matches for Map <%d>\n\n", MapID);
	// } else {
	// 	double delay;
	// 	recv(sockfd, (char *)&delay, sizeof delay, 0);
	// 	printf("The delay for link <%d> is <%f>ms\n\n", MapID, delay);

	// }
	close(sockfd);
	return 0;
}