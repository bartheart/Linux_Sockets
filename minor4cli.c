//Author: Bemnet Merkebu
//CSCE 3600
//Minor4 Client
//Using_Linux_Sockets 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PING_MSG_SIZE 32

//main function
int main(int argc, char *argv[]) {
    //check the number of arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //initate hostname and port number variables
    char *hostname = argv[1];
    int port = atoi(argv[2]);

    struct hostent *hostinfo = gethostbyname(hostname);
    if (hostinfo == NULL) {
        fprintf(stderr, "Unknown host %s.\n", hostname);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;

    //filling server information
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *) hostinfo->h_addr);

    //create socket file descriptor
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket() error");
        exit(EXIT_FAILURE);
    }

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv)) < 0) {
        perror("setsockopt() error");
        exit(EXIT_FAILURE);
    }

    //intitate address structs
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(0);
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind the socket with client address
    if (bind(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
        perror("bind() error");
        exit(EXIT_FAILURE);
    }

    //initiate statisitcs variables
    int msg_count = 10;
    int rcvd_count = 0;
    int loss_count = 0;
    double min_rtt = 1000000.0;
    double max_rtt = 0.0;
    double total_rtt = 0.0;

    //loop over total message transmitted
    for (int i = 1; i <= msg_count; ++i) {
        char ping_msg[PING_MSG_SIZE];
        snprintf(ping_msg, PING_MSG_SIZE, "Ping message %d", i);

        struct timeval start_time, end_time;
        gettimeofday(&start_time, NULL);

        //send PING msg to server
        if (sendto(sockfd, ping_msg, strlen(ping_msg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("sendto() error");
            exit(EXIT_FAILURE);
        }

        printf("%d: Sent... ", i);

        char pong_msg[PING_MSG_SIZE];
        socklen_t server_len = sizeof(server_addr);
        ssize_t recv_len = recvfrom(sockfd, pong_msg, PING_MSG_SIZE, 0, (struct sockaddr *)&server_addr, &server_len);

        //check if package is lost
        if (recv_len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("Timed Out\n");
                loss_count++;
            } else {
                perror("recvfrom() error");
                exit(EXIT_FAILURE);
            }
        } 
        //recived packages
        else {
            gettimeofday(&end_time, NULL);

            double rtt = (end_time.tv_sec - start_time.tv_sec) * 1000.0 + (end_time.tv_usec - start_time.tv_usec) / 1000.0;
            if (rtt < min_rtt) {
                min_rtt = rtt;
            }
            if (rtt > max_rtt) {
                max_rtt = rtt;
            }
            total_rtt += rtt;
            rcvd_count++;

            //print response time
            printf("RTT=%.6f ms\n", rtt);
        }
        

    //sleep 
    sleep(1);
    }

    // Calculate the average RTT
    if (rcvd_count > 0) {
        total_rtt /= rcvd_count;
    }

    //calculate loss precentage
    double loss_percentage = (double) loss_count / msg_count * 100.0;

    //output analysis
    printf("%d pkts xmited, %d pkts rcvd, %.0f%% pkt loss\n", msg_count, rcvd_count, loss_percentage);
    printf("min: %f ms, max: %f ms, avg: %f ms", min_rtt, max_rtt, total_rtt);
    return 0;
}