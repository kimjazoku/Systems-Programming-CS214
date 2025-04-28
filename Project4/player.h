#ifndef PLAYER_H
#define PLAYER_H

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