#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFSIZE 1024

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFSIZE] = {0};
    ssize_t bytes_read = 0;
    // char ipaddr[15] = {0};
    char *ipaddr = "128.6.13.147"; // <- note that i hard coded this in. were gonna need to figure out ip addresses later


    // bytes_read = read(STDIN_FILENO, buffer, BUFFSIZE); // grab ip from stdin by inputting ipconfig into terminal
    // if (bytes_read <= 0)
    // {
    //     perror("ERROR READING IP FROM STDIN");
    //     exit(EXIT_FAILURE);
    // }

    // // now pass in the ip from buffer into ipaddr
    // for(int i = 0; i < bytes_read; i++)
    // {
    //     ipaddr[i] = buffer[i];
    // }

    // // reset/clear the buffer
    // memset(buffer, 0, BUFFSIZE);
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n SOCKET CREATION ERROR");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

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

    //now client is connected to server
    printf("CONNETION SUCCESS\n");
    printf("Enter a message or input nothing to disconnect");

    while(1)
    {
        // reset/clear the buffer
        memset(buffer, 0, BUFFSIZE);
        printf(">: ");
        fflush(stdout);
        bytes_read = read(STDIN_FILENO, buffer, BUFFSIZE);
        if(bytes_read <= 0)
        {
            printf("DISCONNECTED\n");
            break;
        }
        // send a message
        printf("-----------------------\n");
        printf("***listening***\n");
        send(sock, buffer, bytes_read, 0);

        // read message from server
        read(sock, buffer, BUFFSIZE);
        printf("Server: %s\n", buffer);
    }

    close(sock);
    return 0;
}
