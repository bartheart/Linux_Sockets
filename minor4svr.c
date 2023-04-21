//Author: Bemnet Merkebu
//CSCE 3600
//Minor4 Server
//Using_Linux_Sockets 

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#define BUFFER_SIZE 1025


//main function
int main(int argc, char *argv[]){
    int sockfd = 0;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in servaddr, cliaddr;

    //creating socket file desciptors
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        //usage statment 
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    //intitalize structs to 0
    memset(&servaddr, 0 , sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr));
    memset(buffer, 0, sizeof(buffer));

    //server port
    int port = atoi(argv[1]);

    //filling server information
    servaddr.sin_family = AF_INET; //IPv4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    //bind the socket with server address
    if (bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        //usage statement
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    //server ready statment 
    printf("[server]: ready to accept data...\n");

    srand(time(NULL));
    int packetLoss = 0;

    while(1){
        unsigned int len = sizeof(cliaddr);

        //recieve message from client
        int n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr*)&cliaddr, &len);

        if (n < 0){
            perror("Recieve from failed");
            exit(EXIT_FAILURE);
        }

        //simulate packet loss
        if (rand()%100 < 30) {
            printf("[server]: dropped packet\n");
            packetLoss++;
            continue;
        }

        printf("[client]: PING\n");

        //send PONG msg to client
        n = sendto(sockfd, "PONG", n, 0, (struct sockaddr *)&cliaddr, len);
        if (n<0){
            perror("Sendto failed");
            exit(EXIT_FAILURE);
        }


        
    }

    close(sockfd);

    return 0;
}