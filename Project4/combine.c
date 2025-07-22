#include <stdlib.h>
#include <string.h>
#include <stdio.h> // Added for perror
#include <unistd.h>
#include <arpa/inet.h>

#ifndef PLAYER_H
#define PLAYER_H

#define PORT 8080
#define BUFFSIZE 1024

//gcc player.c rpsd.c -o rpsd

#define BACKLOG 10

#define NAME_SIZE 100
#define MOVE_SIZE 10

typedef struct player{
    int socket;
    char name[NAME_SIZE];
    char move[MOVE_SIZE];
    int move_received;
} player;

struct player *create_player(int socket, char *name);
void destroy_player(struct player *player);

#endif
#define DEBUG 1

int queueCount = 0;

struct player *create_player(int socket, char *name) {
    struct player *p = malloc(sizeof(struct player));
    if (p == NULL) {
        perror("Failed to allocate memory for player");
        return NULL;
    }
    
    p->socket = socket;
    strncpy(p->name, name, NAME_SIZE - 1);
    p->name[NAME_SIZE - 1] = '\0'; // Ensure null termination
    p->move_received = 0;
    memset(p->move, 0, MOVE_SIZE); // Initialize move to zero
    return p;
}

void destroy_player(struct player *player) {
    if (player != NULL) {
        free(player);
    }
}



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


void enqueue(queue *ptr, queue *newPlayer) {
    while(ptr->next != NULL) {
        ptr = ptr->next;
    } 
        ptr->next = newPlayer;
        queueCount++;
}  



int main(int argc, char* argv[])
{
    if(DEBUG) {
        argc = 2;
        argv[1] = "8082";
    }

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFSIZE] = {0};
    ssize_t bytes_read = 0;
    queue *front = NULL;
    // char ipaddr[15] = {0};
    char *ipaddr = "128.6.13.177"; // <- note that i hard coded this in. were gonna need to figure out ip addresses later
    
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
    match *matchlist = NULL;
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
                else if (DEBUG){
                    printf("player successfully created\n");
                }
                // add player to queue
                if(front == NULL)
                {
                    // create the queue
                    front = malloc(sizeof(queue));
                    if(front == NULL)
                    {
                        perror("Failed to allocate memory for queue");
                        destroy_player(new_player);
                        close(new_socket);
                        continue;
                    }
                    front->p = new_player;
                    front->next = NULL;
                    queueCount = 1;
                    if(DEBUG)
                    {
                        // check to make sure that the player got added to queue
                        printf("added player: %s\n", front->p->name);
                        printf("you got to line 198\n");
                    }
                }
                else
                {
                    struct queue newPlayer;
                    newPlayer.p = new_player;
                    enqueue(front, &newPlayer);
                    if(DEBUG)
                    {
                        printf("Queue Count: %d\n", queueCount);
                    }
                }

                // match players
                if(queueCount >= 2)
                { 
                    if(DEBUG)
                    {
                        
                        printf("FUCK YOU\n");
                    }
                    struct player *p1 = front->p;
                    struct player *p2 = front->next->p; // most recently added player is p2      
                    if(DEBUG)
                    {
                        printf("p1 name: %s", p1->name);
                        printf("p2 name: %s", p2->name);
                        
                    }              
                    match *new_match = malloc(sizeof(match));
                    if(new_match == NULL)
                    {
                        perror("Failed to allocate memory for match");
                        destroy_player(p1);
                        close(new_socket);
                        continue;
                    }
                    if(DEBUG){
                        printf("match created");
                    }
                    new_match->p1 = p1;
                    new_match->p2 = p2;
                    
                    if(matchlist == NULL)
                    {
                        matchlist = new_match;
                        new_match->next = NULL;
                    }
                    matchlist->next = new_match;
                    new_match->next = NULL;
                    // match created. reset/ clear the queue

                    // free the queue
                    for(int i = 0; i < queueCount; i++)
                    {
                        struct player *temp = front->p;
                        front = front->next;
                        destroy_player(temp);
                    }
 

                    struct player *temp = front->p;
                    front = front->next;
                    queueCount--;
                    
                    // send them a BEGIN message
                    char begin_message[BUFFSIZE];
                    snprintf(begin_message, BUFFSIZE, "B|%s|", p1->name);
                    send(new_socket, begin_message, strlen(begin_message), 0);
                    snprintf(begin_message, BUFFSIZE, "B|%s|", p2->name);
                    
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
        


    }
    close(server_fd);
    return 0;
    // free the queue


        for(int i = 0; i < queueCount; i++)
        {
            struct player *temp = front->p;
            front = front->next;
            destroy_player(temp);
        }
        free(front);
        // free the match
        struct match *current_match = matchlist;
        while (current_match != NULL)
        {
            struct match *temp = current_match;
            current_match = current_match->next;
            free(temp);
        }
        // free the player
}
