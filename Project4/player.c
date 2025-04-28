#include <stdlib.h>
#include <string.h>
#include <stdio.h> // Added for perror
#include "player.h"

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