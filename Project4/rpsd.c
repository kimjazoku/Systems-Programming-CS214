#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "player.h"
#define PORT 8080
#define BUFFSIZE 1024


#define BACKLOG 10

typedef struct queue
{
    struct player *p;
    struct queue *next;
    int count;
} queue;

typedef struct match
{
    int match_id; // may delete this
    struct player *p1;
    struct player *p2;
    struct match *next;
} match;


int main(int argc, char* argv[])
{

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFSIZE] = {0};
    ssize_t bytes_read = 0;
    queue *queue = NULL;
    // char ipaddr[15] = {0};
    char *ipaddr = "128.6.13.147"; // <- note that i hard coded this in. were gonna need to figure out ip addresses later
    
    if (argc != 2)
    {
        fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // Port gets passed in from command line
    int port = atoi(argv[1]); 
    // create the server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("SOCKET CREATION ERROR");
        exit(EXIT_FAILURE);
    }

    // bind time baby
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // this listens on all available interfaces
    address.sin_port = htons(port);

    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("BINDING ERROR");
        exit(EXIT_FAILURE);
    }
    // shhh. are you listening?
    if (listen(server_fd, BACKLOG) < 0)
    {
        perror("LISTENING ERROR");
        exit(EXIT_FAILURE);
    }

    printf("Server Listening on Port %d\n", port);

    // accept a connection
    while(1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
        {
            perror("ACCEPT ERROR");
            exit(EXIT_FAILURE);
        }
        printf("New connection accepted\n");
        // read initial play message
        memset(buffer, 0, BUFFSIZE); // clears/ resets the buffer
        bytes_read = read(new_socket, buffer, BUFFSIZE);
        if (bytes_read <= 0)
        {
            perror("read failed\n");
            close(new_socket);
            continue;
        }
        match *matchlist = NULL;
        //  now parse the message
        if (strncmp(buffer, "P|", 2) == 0)
        {
            char player_name[NAME_SIZE];
            char *start = buffer + 2; // skip the "P|"
            char *end = strstr(start, "||"); // find the end of the name

            if (end != NULL)
            {
                size_t name_length = end - start;
                if (name_length >= NAME_SIZE)
                {
                    name_length = NAME_SIZE - 1; // ensure null termination
                }
                strncpy(player_name, start, name_length);
                player_name[name_length] = '\0'; // null terminate the string
                printf("Player name: %s\n", player_name);

                // send them a wait message
                const char *wait_message = "W|1||";
                send(new_socket, wait_message, strlen(wait_message), 0);

                // create a player struct
                struct player *new_player = create_player(new_socket, player_name);
                if (new_player == NULL)
                {
                    perror("Failed to create player");
                    close(new_socket);
                    continue;
                }
                
                // add player to queue
                if(queue == NULL)
                {
                    // create the queue
                    queue = malloc(sizeof(queue));
                    if(queue == NULL)
                    {
                        perror("Failed to allocate memory for queue");
                        destroy_player(new_player);
                        close(new_socket);
                        continue;
                    }
                    queue->p = new_player;
                    queue->next = NULL;
                    queue->count = 1;
                }
                else
                {
                    queue = queue->next;
                    queue->p = new_player;
                    queue->count = queue->count + 1;
                }

                // match players
                if(queue->count >= 2)
                {
                    struct player *oppheadahh = queue->p; // bug: this will grab the new player. this should be the op instead. 
                    struct queue *ptr = queue;
                    match *new_match = malloc(sizeof(match));\
                    if(new_match == NULL)
                    {
                        perror("Failed to allocate memory for match");
                        destroy_player(oppheadahh);
                        close(new_socket);
                        continue;
                    }
                    new_match->p1 = oppheadahh;
                    new_match->p2 = new_player;
                    
                    if(matchlist == NULL)
                    {
                        matchlist = new_match;
                        new_match->next = NULL;
                    }
                    matchlist->next = new_match;
                    new_match->next = NULL;
                    // match created. reset/ clear the queue
                    queue = NULL;
                    // free the queue
                    for(int i = 0; i < queue->count; i++)
                    {
                        struct player *temp = queue->p;
                        queue->p = queue->next;
                        destroy_player(temp);
                    }
                    free(queue);

                    struct player *temp = queue->p;
                    queue->p = queue->next;
                    queue->count = queue->count - 1;
                    
                    // send them a BEGIN message
                    char begin_message[BUFFSIZE];
                    snprintf(begin_message, BUFFSIZE, "B|%s|", oppheadahh->name);
                    send(new_socket, begin_message, strlen(begin_message), 0);
            }

            }
            else
            {
                fprintf(stderr, "Invalid player name format\n");
                close(new_socket);
            }
        }
        else 
        {
            printf("Unexpected initial message: %s\n", buffer);
            close(new_socket);
        }
        // free the queue
        for(int i = 0; i < queue->count; i++)
        {
            struct player *temp = queue->p;
            queue->p = queue->next;
            destroy_player(temp);
        }
        free(queue);
        // free the match
        struct match *current_match = matchlist;
        while (current_match != NULL)
        {
            struct match *temp = current_match;
            current_match = current_match->next;
            free(temp);
        }
        // free the player

        close(server_fd);
        return 0;
    }
}






// void ignore_this_shit()
// {

//     int sock;
//     int ipaddr;

//     char buffer[BUFFSIZE];
//     ssize_t bytes_read;
//     if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
//     {
//         printf("\n SOCKET CREATION ERROR");
//         return -1;
//     }

//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_port = htons(PORT);

//     if (inet_pton(AF_INET, ipaddr, &serv_addr.sin_addr) <= 0)
//     {
//         printf("INVALID ADDRESS || ADDRESS NOT SUPPORTED\n");
//         return -1;
//     }

//     // connect to server
//     if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
//     {
//         printf("CONNECTION FAILURE\n");
//         return -1;
//     }

//     //now client is connected to server
//     printf("CONNETION SUCCESS\n");
//     printf("Enter a message or input nothing to disconnect");

//     while(1)
//     {
//         // reset/clear the buffer
//         memset(buffer, 0, BUFFSIZE);
//         printf(">: ");
//         fflush(stdout);
//         bytes_read = read(STDIN_FILENO, buffer, BUFFSIZE);
//         if(bytes_read <= 0)
//         {
//             printf("DISCONNECTED\n");
//             break;
//         }
//         // send a message
//         printf("-----------------------\n");
//         printf("***listening***\n");
//         send(sock, buffer, bytes_read, 0);

//         // read message from server
//         read(sock, buffer, BUFFSIZE);
//         printf("Server: %s\n", buffer);
//     }

//     close(sock);
//     return 0;
// }
