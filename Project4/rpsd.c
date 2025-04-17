#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>

#include "connection.h"


// messages sent by client
void play() // Argument: player’s name. 
            // Sent by client after establishing a connection.
{

}

void move() //Argument: one of ROCK, PAPER, SCISSORS. Sent by 
            // client after receiving Begin from server.
{

}

void cont()
{

}

void quit()
{

}

// messages sent by server
void wait()
{

}

void begin()
{

}

void result()
{
    char result[] = {"W", "L", "D", "F"}; // Win, Lose, Draw, Forfeit
}

