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
#include <stdbool.h>

#include <string>
#include <map>
#include <iostream>
#include <climits>
#include <vector>
#include <set>

using namespace std;

#define HOST "localhost"
#define UDP_A "21994"
#define CHARSIZE 256
#define NUM_VERTICE 10
#define MAP_FILE_NAME "map.txt"
#define MAXLINESIZE 1024


char MapID, Node;
int mapcount = 0;
double propagation_speed[CHARSIZE];
double transmission_speed[CHARSIZE];
//mapsInfo: 0: nums of Vertexs 1: nums of edges
int mapsEdges[CHARSIZE];
int mapsName[CHARSIZE];
// int mapsVertices[CHARSIZE];
// set<int> numsOfEdges;
// int** constructedMaps[NUM_VERTICE];
int** constructedMaps;
char* result;



void process_file_edge(char map_id, char* buf, map<char, vector<vector<int> > > &readMaps);

// void process_file_edge(char map_id, char* buf);
int minDistance(int dist[], bool sptSet[]);
void dijkstra(int** graph, int src, char* result, char map_id, map<int, int> &indexToNode);
int printSolution(int dist[], int n);
void initmap(char map_id);
void make_data_tosend_and_print(char map_id, int dist[], int n, char* result, map<int, int> &indexToNode);


// ------------------------------------dijkstra algorithm------------------------------------------------
// A utility function to find the vertex with minimum distance value, from 
// the set of vertices not yet included in shortest path tree 
int minDistance(int dist[], bool sptSet[]) 
{ 
    // Initialize min value 
    int min = INT_MAX, min_index; 
  
    for (int v = 0; v < NUM_VERTICE; v++) 
        if (sptSet[v] == false && dist[v] <= min) 
            min = dist[v], min_index = v; 
  
    return min_index; 
} 

// Function that implements Dijkstra's single source shortest path algorithm 
// for a graph represented using adjacency matrix representation 
// This function is excerpted from https://www.geeksforgeeks.com
// void dijkstra(int** graph, int src, char* result) 
void dijkstra(int** graph, int src, char* result, char map_id, map<int, int> &indexToNode) 
{ 
    int dist[NUM_VERTICE]; // The output array.  dist[i] will hold the shortest 
    // distance from src to i 
  
    bool sptSet[NUM_VERTICE]; // sptSet[i] will be true if vertex i is included in shortest 
    // path tree or shortest distance from src to i is finalized 
  
    // Initialize all distances as INFINITE and stpSet[] as false 
    for (int i = 0; i < NUM_VERTICE; i++) 
        dist[i] = INT_MAX, sptSet[i] = false; 
  
    // Distance of source vertex from itself is always 0 
    dist[src] = 0; 
  
    // Find shortest path for all vertices 
    for (int count = 0; count < NUM_VERTICE - 1; count++) { 
        // Pick the minimum distance vertex from the set of vertices not 
        // yet processed. u is always equal to src in the first iteration. 
        int u = minDistance(dist, sptSet); 
  
        // Mark the picked vertex as processed 
        sptSet[u] = true; 
        
        // Update dist value of the adjacent vertices of the picked vertex. 
        for (int v = 0; v < NUM_VERTICE; v++) 
  
            // Update dist[v] only if is not in sptSet, there is an edge from 
            // u to v, and total weight of path from src to  v through u is 
            // smaller than current value of dist[v] 
            if (!sptSet[v] && graph[u][v] && dist[u] != INT_MAX 
                && dist[u] + graph[u][v] < dist[v]) 
                dist[v] = dist[u] + graph[u][v]; 
    } 
    //printSolution(dist, NUM_VERTICE);
    //make data to send back to aws
    make_data_tosend_and_print(map_id, dist, NUM_VERTICE, result, indexToNode); 
    // printf("%s", result);
}

// int make_data_tosend_and_print() {
// 	printf("Vertex   Distance from Source\n"); 
// 	for (int i = 0; i < V; i++) {
// 		printf("%d tt %d\n", i, dist[i]); 
// 	}
// 	sendto(sockfd, (char *)&NP, sizeof NP, 0, (struct sockaddr *) &their_addr, addr_len);
// 	return 0;
// }
void make_data_tosend_and_print(char map_id, int dist[], int n, char* result, map<int, int> &indexToNode) 
{ 
    char line_str[MAXLINESIZE];
    sprintf(line_str,"");
 //    sprintf(line_str, "%f %f\n", propagation_speed[map_id], transmission_speed[map_id]);
 //    cout << propagation_speed[map_id] << transmission_speed[map_id] << endl;
	strcpy(result, line_str);
    
    printf("The Server A has identified the following shortest paths:\n-----------------------------\nDestination  Min Length\n-----------------------------\n");

    for (int i = 0; i < NUM_VERTICE; i++) {
        if (dist[i] != INT_MAX && dist[i] != 0) {
            sprintf(line_str, "%d\t%d\n", indexToNode[i], dist[i]);
            printf("%s", line_str);
            strcat(result, line_str);
        }
    }
    printf("-----------------------------\n");
}

// A utility function to print the constructed distance array 
// int printSolution(int dist[], int n) 
// { 
//     printf("Vertex   Distance from Source\n"); 
//     for (int i = 0; i < NUM_VERTICE; i++) {
//     	if (dist[i] != INT_MAX && dist[i] != 0) {
//     		printf("%d tt %d\n", i, dist[i]);
//     	}
//     }
// } 

//------------------------------------------------------------------------------------------------------
void construct_maps(map<char, vector<vector<int> > > &readMaps) {
// void construct_maps() {
    char filename[MAXLINESIZE];
    strcpy(filename, MAP_FILE_NAME);
    FILE* fp;
    char buf[MAXLINESIZE];
    if ((fp = fopen(filename, "r")) == NULL) {
        perror("fopen map.txt failed\n");
        exit(1);
    }
    
    char map_id;
    while(fgets(buf, sizeof(buf), fp) != NULL) {
        //start three lines
        if (strchr(buf, ' ') == NULL) {
            // mapcount++;
            map_id = buf[0];
            //cout << "a is " << map_id << endl;
            int mapNumOfChar = map_id;
            //cout << "a is " << mapNumOfChar << endl;
            mapsName[mapNumOfChar] = 1;
            fgets(buf, sizeof(buf), fp);
            //cout << "buf is " << buf << endl;
            propagation_speed[mapNumOfChar] = atof(buf);
            // cout << "propagation_speed is " << propagation_speed[mapNumOfChar] << endl;
            fgets(buf, sizeof(buf), fp);
            transmission_speed[mapNumOfChar] = atof(buf);
            // cout << "propagation_speed is " << transmission_speed[mapNumOfChar](d) << endl;
            // printf("%.2f\n", transmission_speed[mapNumOfChar]);
            // initmap(map_id);
            // printf("p_speed:%f\nt_speed:%f\n", propagation_speed[mapNumOfChar], transmission_speed[mapNumOfChar]);
            continue;
        }
        // process_file_edge(map_id, buf);
        process_file_edge(map_id, buf, readMaps);
    }
    fclose(fp);
}

void process_file_edge(char map_id, char* buf, map<char, vector<vector<int> > > &readMaps) {
// void process_file_edge(char map_id, char* buf) {
    char* delim = " ";
    //string delim = " ";
    int count;
    int from;
    int to;
    int weight;
    vector<int> temp;


    int numOfIndex = 0;
    for (char* token = strtok(buf, delim), count = 0; token != NULL; token = strtok(NULL, delim), count++) {
        switch(count) {
            case 0:
                from = atoi(token);
                temp.push_back(from);
                // cout << from << " ";
                break;
            case 1:
                to = atoi(token);
                temp.push_back(to);
                // cout << to << " ";
                break;
            case 2:
                weight = atoi(token);
                temp.push_back(weight);
                // cout << weight << endl;
                break;
            default:
                return;
        }
    }
    // for (int i = 0; i < 3; i++) {
    //     cout << temp.at(i) << " ";
    // }
    // cout << endl;
    map<char, vector<vector<int> > > :: iterator it;
    it = readMaps.find(map_id);
    if (it != readMaps.end()) {
        readMaps[map_id].push_back(temp);
    } else {
        vector<vector<int> > newVec;
        newVec.push_back(temp);
        readMaps[map_id] = newVec;
    }



	// constructedMaps[map_id][from][to] = weight;
	// constructedMaps[map_id][to][from] = weight;

	mapsEdges[map_id]++;
}

void findIndex(map<char, vector<vector<int> > > &readMaps, map<int, int> &indexToNode, map<int, int> &nodeToIndex, char givenMapID) {
    set<int> tempIndex;
    for (int i = 0; i < readMaps[givenMapID].size(); i++) {
        int index1 = readMaps[givenMapID].at(i).at(0);
        int index2 = readMaps[givenMapID].at(i).at(1);
        tempIndex.insert(index1);
        tempIndex.insert(index2);
    }
    int index = 0;
    for(set<int>::iterator it=tempIndex.begin() ;it!=tempIndex.end();it++) {
        int node = *it;
        nodeToIndex[node] = index;
        indexToNode[index] = node;
        index++;
    }
}

void initmap() {
    constructedMaps = (int**)malloc(NUM_VERTICE * sizeof(int*));
    for (int i = 0; i < NUM_VERTICE; i++) {
        constructedMaps[i] = (int*)malloc(NUM_VERTICE * sizeof(int));
        for (int j = 0; j < NUM_VERTICE; j++) {
            constructedMaps[i][j] = 0;
        }
    }
    
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

	if ((rv = getaddrinfo(HOST, UDP_A, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("serverA: socket\n");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("serverA: bind\n");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "serverA: failed to bind socket\n");
		exit(1);
	}
	freeaddrinfo(servinfo);	// all done with this structure
	printf("The Server A is up and running using UDP on port <%s>.\n", UDP_A);

	

	//dijkstra(constructedMaps['A'], 0);

	map<char, vector<vector<int> > > readMaps;
    //construct the map and print some information
    construct_maps(readMaps);
    //find maps's num of vertices and edges
    cout << "------------------------------------------" << endl;
    cout << "Map ID  Num Vertices  Num Edges" << endl;
    cout << "------------------------------------------" << endl;

    map<char, vector<vector<int> > > :: iterator iter;
    for (iter = readMaps.begin(); iter != readMaps.end(); iter++) {
    	set<int> nodes;
    	char cur = iter->first;
    	for (int i = 0; i < readMaps[cur].size(); i++) {
    		int index1 = readMaps[cur].at(i).at(0);
    		int index2 = readMaps[cur].at(i).at(1);
    		nodes.insert(index1);
    		nodes.insert(index2);
		}
		cout << cur << "           " << nodes.size() << "            " << readMaps[cur].size() << endl;
    }
	cout << "------------------------------------------" << endl;

	while (1) {
		addr_len =  sizeof their_addr;

        // construct_maps();
        // cout << readMaps['A'].at(0).at(2) << endl;     
		char givenMapID;
		int givenNode;
		char res[MAXLINESIZE];

		recvfrom(sockfd, (char *)&givenMapID, sizeof givenMapID, 0, (struct sockaddr *)&their_addr, &addr_len);
		recvfrom(sockfd, (char *)&givenNode, sizeof givenNode, 0, (struct sockaddr *)&their_addr, &addr_len);

		// givenMapID = 'H';
		// givenNode = '12';
		// cout << "The Server A recieved input mapID: " << givenMapID << ", Node: " << givenNode << endl;
		cout << "The Server A has received input for finding shortest paths: starting vertex <" << givenNode << "> of map <" << givenMapID << ">." << endl;
		//printf("The Server A recieved input mapID: <%s>, Node: <%s>\n", givenMapID, givenNode);
		//int match = search(givenMapID, sockfd, their_addr, addr_len);

        //cast for demanded maps' index
        map<int, int> indexToNode;
        map<int, int> nodeToIndex;
        findIndex(readMaps, indexToNode, nodeToIndex, givenMapID);
        // cout << indexToNode.size() << endl;
        // cout << nodeToIndex.size() << endl;
        // cout << readMaps[givenMapID].size() << endl;


        initmap();
        //cast map
        for (int i = 0; i < readMaps[givenMapID].size(); i++) {
            int NodeOfFrom = readMaps[givenMapID].at(i).at(0);
            int NodeOfTo = readMaps[givenMapID].at(i).at(1);
            // cout << NodeOfFrom << " " << NodeOfTo << endl;

            int from = nodeToIndex[NodeOfFrom];
            int to = nodeToIndex[NodeOfTo];
            // cout << from << " " << to << endl;

            int weight = readMaps[givenMapID].at(i).at(2);
            constructedMaps[from][to] = weight;
            constructedMaps[to][from] = weight;
        }
        // for (int i = 0; i < NUM_VERTICE; i++) {
        //     for (int j = 0; j < NUM_VERTICE; j++) {
        //         cout << constructedMaps[i][j] << " ";
        //     }
        //     cout << endl;
        // }



		int src = nodeToIndex[givenNode];

		result = res;


		double propagation = propagation_speed[givenMapID];
		double transmission = transmission_speed[givenMapID];
		dijkstra(constructedMaps, src, result, givenMapID, indexToNode);

        // cout << result << endl;
        int numbytes;
        if ((numbytes = sendto(sockfd, (char *)&propagation, sizeof propagation, 0, (struct sockaddr *) &their_addr, addr_len) == -1)) {
            perror("client: sendto failed\n");
            return -1;
        }
    	// sendto(sockfd, (char *)&propagation, sizeof propagation, 0, (struct sockaddr *) &their_addr, addr_len);
    	sendto(sockfd, (char *)&transmission, sizeof transmission, 0, (struct sockaddr *) &their_addr, addr_len);
    	sendto(sockfd, res, MAXLINESIZE - 1, 0, (struct sockaddr *) &their_addr, addr_len);

		memset(res,'\0',sizeof(res));
		cout << "The Server A has sent shortest paths to AWS." << endl;

	}
	return 0;
}