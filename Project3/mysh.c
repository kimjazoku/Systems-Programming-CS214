#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define DEBUG 1
#define BUFSIZE 100
#define MAXWORD 45
#define MAX_BUF 100

char otherTokens[] = { '>', '<', '|', '*', '#' };
char *builtIns[] = {"cd", "pwd", "which", "exit", "die"};


typedef struct Token {
    char *str;
    struct Token *next;
} Token;

//pass in front->next
int cd(Token *args) {

    if(args == NULL) {
        errno = EINVAL;
        perror("0 arguments given");
        return -1;
    }
    if(args->next) {
        errno = EINVAL;
        printf("Too many arguments given");
        return -1;
    }

    int status = chdir(args->str);
    if(status == -1) {
        perror("Cannot change current directory");
        return -1;
    }

    return 0;

}

int pwd(Token *args) {

    if(args != NULL) {
        printf("Invalid number of args");
        return -1;
    }
    char buf[BUFSIZE];
    getcwd(buf, BUFSIZE);
    printf("%s\n", buf);

    return 0;
}

int execute(Token *ptr) {

    char call[128] = ".";

    while(ptr != NULL) {
        strcat(call, ptr->str);
        strcat(call, " ");
        ptr = ptr->next;
    }

    int exitStatus = system(call);
    if(exitStatus != 0) {
        errno = exitStatus;
        perror("error running program");
        return -1;
    }

    return 0;
    
}

/* int which(Token *arg) {
    
    if(arg == NULL) {
        errno = EINVAL;
        perror("0 arguents given");
        return -1;
    }
    if (arg->next != NULL) {
        errno = EINVAL;
        perror("Too many arguments given");
        return -1;
    }

    for(int i = 0; i < sizeof(builtIns); i++) {
        if(strcmp(arg, builtIns[i]) == 0) {
            printf("Error: cannot use the name of a built-in argument (%s)\n", arg->str);
            return -1;
        }
    }
    
    
} */

Token* CreateToken(const char *word, Token *front) {
    Token *newToken = malloc(sizeof(Token));
    if (!newToken) {
        perror("malloc failed");
        return NULL;
    }

    newToken->str = strdup(word); // allocate new memory for word
    newToken->next = NULL;

    if (!front) return newToken;

    Token *curr = front;
    while (curr->next)
        curr = curr->next;
    curr->next = newToken;
    return front;
}

void printTokens(Token *front) {
    Token *curr = front;
    if(DEBUG)
    {
        while (curr) {
            printf("Token: %s\n", curr->str);
            curr = curr->next;
        }
    }
}

void freeTokens(Token *front) {
    while (front) {
        Token *temp = front;
        front = front->next;
        free(temp->str);
        free(temp);
    }
}

void readInput(int input_fd, int interactive, Token **front) {
    char buf[MAX_BUF];
    char word[45];
    int j = 0, inWord = 0;
    ssize_t bytes;
    char ch;

    if (interactive) {
        printf("Welcome to my shell!\n");
        printf("mysh> ");
        fflush(stdout);
    }

    while ((bytes = read(input_fd, &ch, 1)) > 0) {
        if (ch == '\n') {
            if (inWord) {
                word[j] = '\0';
                *front = CreateToken(word, *front);
                j = 0;
            }

            // check for exit/die
            Token *curr = *front;
            while (curr) {
                if (strcmp(curr->str, "exit") == 0 || strcmp(curr->str, "die") == 0) {
                    if (interactive) printf("Goodbye!\n");
                    return;
                }
                curr = curr->next;
            }

            if (interactive) {
                if(DEBUG)
                {
                    printTokens(*front);
                }
                // freeTokens(*front);
                *front = NULL;
                printf("mysh> ");
                fflush(stdout);
            } else {
                if(DEBUG)
                {
                    printTokens(*front);
                }
                // freeTokens(*front);
                *front = NULL;
            }
        } else if (isspace(ch)) {
            if (inWord) {
                word[j] = '\0';
                *front = CreateToken(word, *front);
                j = 0;
                inWord = 0;
            }
        } else {
            if (!inWord) inWord = 1;
            if (j < sizeof(word) - 1) {
                word[j++] = ch;
            }
        }
    }
    if (interactive) printf("\nGoodbye!\n");
    freeTokens(*front);
}

int main(int argc, char *argv[]) {
    Token *front = NULL;
    int input_fd = STDIN_FILENO;

    if (argc == 2) {
        input_fd = open(argv[1], O_RDONLY);
        if (input_fd == -1) {
            perror("Failed to open file");
            return 1;
        }
    }

    int interactive = isatty(input_fd);
    if (DEBUG) {
        printf("fd status: %d\n", interactive);
    }

    readInput(input_fd, interactive, &front);

    // Token *front = (Token*) malloc(sizeof(Token));
    // front->str = "";
    // front->next = NULL;

    if(strcmp(front->str, "cd") == 0) {
        cd(front->next);
    }


    if(strcmp(front->str, "pwd") == 0) {
        pwd(front->next);
    }

    if(front->str[0] == '/') {
        execute(front);
    }

   /* if(strcmp(command->str, "which") == 0) {
        which(command->next);
    } */

    if (input_fd != STDIN_FILENO)
        close(input_fd);

    return 0;
}
