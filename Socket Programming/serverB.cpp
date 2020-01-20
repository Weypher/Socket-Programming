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
#include <math.h>

#include <iostream>
#include <map>
#include <vector>

using namespace std;

#define HOST "localhost"
#define UDP_B "22994"
#define MAXLINESIZE 1024


char* resFromA;
char resBuf[MAXLINESIZE];
char finalResBuf[MAXLINESIZE];
char* resFromB;
char* finalResultForClient;
double propagation_speed;
double transmission_speed;




void spiltAndCal(char* resFromA, double propagation_speed, double transmission_speed, map<int, int> &mapsInfo, vector<int> &mapKey, long long fsize, char* resFromB
													, int sockfd, struct sockaddr_storage their_addr, socklen_t addr_len, char* finalResultForClient) {
	//read the results from A
    // char* delim = " ";
    //string delim = " ";
    char data[MAXLINESIZE];
    strcpy(data, resFromA);
    cout << "=====================================" << endl;
	int curSum = 0;
	int node = 0;
	int dis = 0;
	for(int i = 0;i < MAXLINESIZE; i++){
		if (data[i] == '\0') {
			break;
		}
		if (data[i] >= 48 && data[i] <= 57) {
			curSum = curSum * 10 + (int)(data[i] - '0');
		}
		if (data[i] == '\t') {
			node = curSum;
			curSum = 0;
		} else if (data[i] == '\n') {
			dis = curSum;
			cout << "* Path length for destination <" << node << ">:" << "<" << dis << ">" << endl;
			mapsInfo.insert(pair<int,int>(node,dis));
			mapKey.push_back(node);
			curSum = 0;
			node = 0;
			dis = 0;
		}
	}

	
	char line_str[MAXLINESIZE];
	char resultsLine[MAXLINESIZE];
	char resForClientLine[MAXLINESIZE];



	resFromB = resBuf;
	finalResultForClient = finalResBuf;

    cout << "=====================================" << endl;
    cout << "has Server B has finished the calculation of the delays:" << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "Destination  Delay" << endl;
    cout << "--------------------------------------------------" << endl;

    for (int i = 0; i < mapKey.size(); i++) {
    	double delay = 0;
    	double Tt = 0;
    	double Tp = 0;
    	Tt = fsize / transmission_speed * 8;
    	Tp = mapsInfo[mapKey.at(i)] / propagation_speed;
    	delay = Tt + Tp;
    	sprintf(line_str, "%d\t%.2f\n", mapKey.at(i), delay);
    	sprintf(resultsLine, "%d\t%.2f\t%.2f\t%.2f\n", mapKey.at(i), Tt, Tp, delay);
    	sprintf(resForClientLine, "%d\t\t%d\t%.2f\t%.2f\t%.2f\n", mapKey.at(i), mapsInfo[mapKey.at(i)], Tt, Tp, delay);

        printf("%s", line_str);
        // printf("%s", resultsLine);
        strcat(resFromB, resultsLine);
        strcat(finalResultForClient, resForClientLine);


	}
    cout << "--------------------------------------------------" << endl;
    mapsInfo.clear();
	mapKey.clear();
	// cout << resBuf << endl;
	// sendto(sockfd, (char *)&transmission_speed, sizeof transmission_speed, 0, (struct sockaddr *) &their_addr, addr_len);
	// cout << "send";
}

int main() {
	// Set up UDP --- code from Beej's Documentation
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(HOST, UDP_B, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("serverB: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("serverB: bind");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "serverB: failed to bind socket\n");
		exit(1);
	}
	freeaddrinfo(servinfo);	// all done with this structure
	printf("The Server B is up and running using UDP on port <%s>.\n", UDP_B);

	while (1) {
		addr_len =  sizeof their_addr;
		long long fsize;
		recvfrom(sockfd, (char *)&fsize, sizeof fsize, 0, (struct sockaddr *)&their_addr, &addr_len);
		// recvfrom(sockfd, (char *)&fsize, sizeof fsize, 0, NULL, NULL);
		// cout << fsize << endl;
		recvfrom(sockfd, (char *)&propagation_speed, sizeof propagation_speed, 0, NULL, NULL);
		recvfrom(sockfd, (char *)&transmission_speed, sizeof transmission_speed, 0, NULL, NULL);
		cout << "The Server B has received data for calculation:" << endl;
		// cout << "* Propagation speed: <" << propagation_speed << "> km/s;" << endl;
		// cout << "* transmission speed: <" << transmission_speed << "> Bytes/s;" << endl;
		printf("* Propagation speed: <%.2f> km/s;\n", propagation_speed);
		printf("* transmission speed: <%.2f> Bytes/s;\n", transmission_speed);
		int numreceived;
		char buf[MAXLINESIZE];
		if ((numreceived = recvfrom(sockfd, buf, MAXLINESIZE - 1, 0, NULL, NULL)) == -1) {
            perror("recvfrom");
            exit(1);
    	}

    	

    	resFromA = buf;
		// cout << "* Path length for destination <vertex index>:" << endl << resFromA;
		map<int, int> mapsInfo;
		vector<int> mapKey;
		spiltAndCal(buf, propagation_speed, transmission_speed, mapsInfo, mapKey, fsize, resFromB, 
			sockfd, their_addr, addr_len, finalResultForClient);
		// for (int i = 0; i < mapKey.size(); i++) {
		// 	cout << mapKey.at(i) << endl;
		// }
		// cout << resFromB << endl;
		// cout << "before send is ok" << endl;
		// int test = 10;
		// sendto(sockfd, resBuf, MAXLINESIZE - 1, 0, (struct sockaddr *) &their_addr, addr_len);
		// sendto(sockfd, (char *)&test, sizeof test, 0, (struct sockaddr *) &their_addr, addr_len);
		int numbytes;
		if ((numbytes = sendto(sockfd, resBuf, strlen(resBuf), 0, (struct sockaddr *)&their_addr, addr_len) == -1)) {
            perror("client: sendto failed\n");
            return -1;
    	}
    	// cout << finalResBuf << endl;
    	if ((numbytes = sendto(sockfd, finalResBuf, strlen(finalResBuf), 0, (struct sockaddr *)&their_addr, addr_len) == -1)) {
            perror("client: sendto failed\n");
            return -1;
    	}

		// cout << "after send is ok" << endl;

		//clear buffer
		memset(resBuf, 0, sizeof(resBuf) / sizeof(char));
		memset(finalResBuf, 0, sizeof(resBuf) / sizeof(char));
		cout << "The Server B has finished sending the output to AWS" << endl;


		//close(sockfd);
	}
}