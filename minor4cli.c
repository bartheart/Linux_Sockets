//Author: Bemnet Merkebu
//CSCE 3600
//Minor4 Client
//Using_Linux_Sockets 

#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <math.h>

#define BUFFER_SIZE 1025
#define NUM_MESSAGE 10  

//main function
int main(int argc, char *argv[]){
    int sockfd = 0;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in servaddr;

    //check command line args
    if (argc != 3){
        printf("Usage: %s <server hostname> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *serverHostname = argv[1];
    int serverPort = atoi(argv[2]);

    //check the host exist
    struct hostnet *server = gethostbyname(serverHostname);
    if( server == NULL){
        fprintf(stderr, "Error: no such host as%s\n", serverHostname);
        exit(EXIT_FAILURE);
    }

    //create socket file descriptor
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    //set to 0
    memset(&servaddr, 0 , sizeof(servaddr));
    memset(buffer, 0, sizeof(buffer));

    //fill server info
    servaddr.sin_family = AF_INET; //IPv4
    servaddr.sin_port = htons(serverPort);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //intitalize statstics variables
    int numSnt = 0, numRcvd = 0, numLst = 0;
    int min_rtt = INT_MAX;
    int max_rtt = INT_MIN;
    double avg_rtt = 0.0;

    //Set up the file descriptor set for select
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);

    for (int i = 0; i < NUM_MESSAGE; i++){
        numSnt++;
        // Send PONG message back to client
        char ping_msg[BUFFER_SIZE];
        sprintf(ping_msg, "PING %d", i);
        sendto(sockfd, ping_msg, strlen(ping_msg), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
    

        // Record the send time
        struct timeval start, end;
        gettimeofday(&start, NULL);

        // Wait for response using selectint n = 0;
        struct timeval timeout = {1, 0}; // Wait for 1 second
        int ready = select(sockfd+1, &fds, NULL, NULL, &timeout);
        if (ready == -1) {
            perror("select failed");
            exit(EXIT_FAILURE);
        }
        else if (ready == 0) {
            printf("%d: Sent... Time Out\n", i+1);
        }
        else {
            int n = 0;
            if((n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL)) < 0){
                perror("recvfrom failed");
                exit(EXIT_FAILURE);
            } else {
                buffer[n] = '\0';

                // Record the receive time
                gettimeofday(&end, NULL);

                // Calculate the RTT and update the statistics variables
                double rtt = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
                if (rtt < min_rtt) {
                    min_rtt = rtt;
                }
                if (rtt > max_rtt) {
                    max_rtt = rtt;
                }
                avg_rtt += rtt;
                numRcvd++;

                printf("%d: Sent... RTT=%.3f ms\n", i, rtt);
            }
        }

    //sleep
    sleep(1);
    }

    // Calculate the average RTT
    if (numRcvd > 0) {
        avg_rtt /= numRcvd;
    }
    return 0;
}