#include <stdlib.h>
#include <string.h>
#include <stdio.h> // Added for perror
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define BACKLOG 10
#define DEBUG 1
#define NAME_SIZE 100
#define MOVE_SIZE 10
#define BUFLEN 1024

enum Moves {ROCK, PAPER, SCISSORS};


typedef struct client {

    int fd;
//characters still in buffer
    char buf[BUFLEN];
    int bufSize;

    char name[NAME_SIZE];

} client;



int setupListener(int port) {
//set up the socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(server_fd < 0) {
        perror("Socket error");
        return -1;
    }

//set it up so that I can reuse the same port
    int opt = 1;
    if (setsockopt(
        server_fd,        // the socket you just created
        SOL_SOCKET,       // at the "socket" API level
        SO_REUSEADDR,     // the option you want to tweak
        &opt,             // pointer to the new value (nonzero = on)
        sizeof(opt)       // size of that value
    ) < 0) 
    {
        perror("setsockopt");
        return -1;
    }

    return server_fd;

}


struct sockaddr_in setupAddress(int port) {

    struct sockaddr_in serverAddress;

    //set everything to 0 (default) in serverAddress 
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    return serverAddress;
}

client *acceptClient(int server_fd, struct sockaddr_in *peer_addr, socklen_t *peer_len) {
        client *currentClient = malloc(sizeof(client));

        currentClient->fd = accept(server_fd, (struct sockaddr *)peer_addr, peer_len);

        if (currentClient->fd < 0)
        {
            perror("ACCEPT ERROR");
            exit(EXIT_FAILURE);
        }
        printf("New connection accepted\n");
        

        return currentClient;
}

int readMessage(client *c) {
    memset(c->buf, 0, BUFLEN);
    c->bufSize = 0;

    while(1) {
        ssize_t n = recv(
            c->fd,
            c->buf + c->bufSize,
            BUFLEN - c->bufSize,
            0
        );
        if(n < 0) {
            perror("error during recv");
            return -1;
        }
        if (n == 0) {
            fprintf(stderr, "Client disconnected during handshake\n");
            return -1;
        }

        c->bufSize += n;

        for(int i = 0; i < c->bufSize - 1; i++) {
            if(c->buf[i] == '|' && c->buf[i+1] == '|') {
                int msg_len = i + 2;
                c->buf[msg_len] = '\0';

                return 0;
            }
        }

    }
}

int handshake(client *c) { 

    int status = readMessage(c);

    if(status == -1) {
        return -1;
    }

// validate format: must start with "P|"
    if (c->buf[0] != 'P' || c->buf[1] != '|') {
        fprintf(stderr, "Expected P|Name||, got %.2s\n", c->buf);
        return -1;
    }

// find second '|' (end of name)
    char *second_bar = strchr(c->buf + 2, '|');
    if (!second_bar) {
        fprintf(stderr, "Format: P|<Name>||\n");
        return -1;
    }
    int name_len = second_bar - (c->buf + 2);
    if (name_len <= 0 || name_len >= NAME_SIZE) {
        fprintf(stderr, "Invalid name length %d\n", name_len);
        return -1;
    }

// copy name
    memcpy(c->name, c->buf + 2, name_len);
    c->name[name_len] = '\0';
    printf("Player wants name: %s\n", c->name);

// send back "W|1||"
    const char *resp = "W|1||";
    if (send(c->fd, resp, strlen(resp), 0) != (ssize_t)strlen(resp)) {
        perror("send");
        return -1;
    }

// clear buffer for next stage
    c->bufSize = 0;
    return 0;
}


void run_match(client *c1, client *c2) {

}

void cleanup_client(int client_fd) {
    close(client_fd);

}


int main(int argc, char *argv[]) {

//client that is currently waiting for a game
    client *pendingClient = NULL;

//check valid # of arguments
    int port; 
    int client_fd;

    if(DEBUG) {
        port = 8080;
    }
    else {
        if(argc != 2) {
            fprintf(stderr, "Usage: rpsd <port>\n");
            return 1;
        }
        port = atoi(argv[1]);

    }
    

//check valid port (1024-65535)
    if(port > 65535 || port < 1024) {
        printf("Usage: rpsd <port>\n");
        return 1;        
    }

//server's file descriptor
    int server_fd = setupListener(port);
    
    if(server_fd == -1) {
        fprintf(stderr, "Failed to create listener\n");
        return -1;
    }

//server address
    struct sockaddr_in serverAddress = setupAddress(port);

    int status = bind(server_fd, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    if(status < 0) {
        perror("Binding error");
        return 1;
    }

//enter "listener" state then an infinite loop

    if(listen(server_fd, BACKLOG) < 0) {
        perror("Listening error");
        return 1;
    }
    

    printf("Server Listening on Port %d\n", port);
    
//create a second address for anyone connecting
    struct sockaddr_in peer_addr;
    socklen_t peer_len = sizeof(peer_addr);

    while(1) {

        client *c = acceptClient(server_fd, &peer_addr, &peer_len);
        if (!c) continue;

        if(handshake(c) != 0) {
            cleanup_client(c->fd);
            free(c);
            continue;
        }

    //check if there is currently a client waiting for a game
        if(pendingClient == NULL) {
            pendingClient = c;
        }
        else {

        //match up two clients -- no longer pending
            client *c1 = pendingClient;
            client *c2 = c;
            pendingClient = NULL;


        //run match between two players
        //TODO: implement forking for concurrent matches
            run_match(c1, c2);


        //free all clients at the end
            cleanup_client(c1->fd);
            cleanup_client(c1->fd);
            free(c1);
            free(c2);


        }

    }



    return 0;


}
