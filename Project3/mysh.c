#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
Things to impliment:

case where token is '*'
case where in word and at end of file
case where line is '\n'
*/


#define DEBUG 0
#define BUFSIZE 100
#define MAXWORD 45

char otherTokens[] = { '>', '<', '|', '*', '#'};

typedef struct Token
{
    char *str;
    struct Token *next;
} Token;

void *CreateToken(char *word, Token *front) {
    Token *newToken = malloc(sizeof(Token));
    

    if(DEBUG) {
        // printf("%s ", newNode->word);
    }
    
    while(front != NULL) {
        if (front->next == NULL) {
            front->next = newToken;
            newToken->str = word;
            newToken->next = NULL;
            break;
        }

        front = front->next;
    }
}

// use this for batch since we pass in a file
void readIn(int fd, Token *front)
{
    char buf[BUFSIZE];
    int bytes_read;


    int comment = 0;
    int inWord = 0;
    char currentToken[MAXWORD];
    int tokenPos = 0;

    while((bytes_read = read(fd, buf, BUFSIZE)) > 0) {
        for(int i = 0; i < bytes_read; i++) {

            if(buf[i] == '\n') {
                //Add something later to deal with this case
            }

            if(comment || buf[i] == '#') {
                if(buf[i] != '\n') {
                    comment = 1;
                    continue;
                }
                else {
                    comment = 0;
                }
            }
            //start of word
            if(!inWord && !isspace(buf[i])) {
                inWord = 1;                
            }

            if(inWord) {

            //end of word
                if(isspace(buf[i])) {
                    inWord = 0;
                //add current token to list of commands
                    CreateToken(currentToken, front);
                //reset current token length
                    currentToken[0] = '\0';
                    tokenPos = 0;

                    continue;
                }
                //middle of word
                currentToken[tokenPos++] = buf[i];
                currentToken[tokenPos] = '\0';
            }
        //NEED TO ADD CASE FOR END OF FILE WITH CURRENTTOKEN > 0


            
        }

    }
}

int main(int argc, char *argv[]) {    
    int fd;
    if (argc == 1) {
        printf("No arguments provided\n");
        printf("stdin is a terminal\n");
        printf("Welcome to my shell!\n");
        fd = 1;
        
        } 
    if (argc == 0) {
        printf("No arguments provided\n");
        printf("stdin is a terminal\n");
        printf("Welcome to my shell!\n");
    } 


    
    int fd = isatty(STDIN_FILENO);
    if (fd == 1) {
        int exitStatus = 1;
        // stdin is a terminal || interactive mode
        printf("Welcome to my shell!\n");
        printf("mysh>");


        // fopen(argv[1], "r");
        // ^ use this for batch since we pass in a file
        
        // read in the file
        Token *front = NULL;
        
        while(exitStatus != 0)
        {
            // read every str from stdin
            for(int i = 1; i < argc; i++)
            {
                // this implements the link list of tokens
                // it does all the work of creating and storing data
                CreateToken(argv[i], front);
            }

            
            //read from stdin
            printf("mysh> ");
            readIn(0, front);


            // search for exit in the linked list
            while(front != NULL)
            {
                if(front -> str == 'exit')
                {
                    // exit the loop if the line contains exit
                    exitStatus = 0;
                    break;
                }
                front = front->next;
            }
            
        }
        
        

        
    }

    return 0;
}
