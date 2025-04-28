#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "player.h"

// #define PORT 8080
#define BUFFSIZE 1024
// ip 128.6.13.147
int main(int argc, char *argv[])
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFSIZE] = {0};
    ssize_t bytes_read = 0;
    
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <port> <server_ip> <user_name> \n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    int port = atoi(argv[1]);
    char *ipaddr = argv[2];
    char *name = argv[3];



    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n SOCKET CREATION ERROR");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ipaddr, &serv_addr.sin_addr) <= 0)
    {
        printf("INVALID ADDRESS || ADDRESS NOT SUPPORTED\n");
        return -1;
    }

    // connect to server
    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("CONNECTION FAILURE\n");
        return -1;
    }
    char msg[BUFFSIZE];
    int len = snprintf (msg, BUFFSIZE, "P|%s||", name);
    send(sock, msg, len, 0);
    //now client is connected to server
    printf("CONNETION SUCCESS\n");


    while(1)
    {
        // reset/clear the buffer
        memset(buffer, 0, BUFFSIZE);
        fflush(stdout);
        // bytes_read = read(STDIN_FILENO, buffer, BUFFSIZE);
        // if(bytes_read <= 0)
        // {
        //     printf("DISCONNECTED\n");
        //     break;
        // }
        // // send a message
        // printf("-----------------------\n");
        // printf("***listening***\n");
        // send(sock, buffer, bytes_read, 0);

        // read message from server
        read(sock, buffer, BUFFSIZE);
        printf("Server: %s\n", buffer);
        
    }

    close(sock);
    return 0;
}
