#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>


#define DEBUG 0

char otherTokens[] = { '>', '<', '|', '*', '#'};

typedef struct Token
{
    char *str;
    struct Token *next;
} Token;

Token *CreateToken(char *word, Token *front) {
    Token *newToken = malloc(sizeof(Token));
    if (!newToken) {
        perror("Failed to allocate memory for new token");
        return NULL;
    }

    newToken->str = word;
    newToken->next = NULL;
    
    if (front == NULL) {
        front = newToken;
        return front;
    }

    Token *curr = front;
    while(curr->next != NULL) {
        // if (front->next == NULL) {
        //     front->next = newToken;
        //     newToken->next = NULL;
        //     break;
        // }

        curr = curr->next;
    }
    curr->next = newToken;
    return front;
}

void readFile(int argc, char *filename, Token *front)
{
    char buf[100];
    char currentWord[45];
    int bytes;
    int inWord;
    int j = 0;
    while((bytes = read(bytes, buf, sizeof(buf))) > 0) {
        for(int i = 0; i < bytes; i++) {
        // end of a word
            if(isspace(buf[i])) {
                
                inWord = 0;
                currentWord[j] = '\0';
                createToken(currentWord, front);

                // loop through the word from the end. If there is an invalid character at the end, it will keep removing subsequent invalid characters until it finds a valid one
                int validEnd = 1;
                for(int w = strlen(currentWord) - 1; w >= 0; w--) {
                    for(int c = 0; c < sizeof(otherTokens); c++) {
                        if(currentWord[w] == otherTokens[c]) {
                            currentWord[w] = '\0';
                            validEnd = 0;
                        }
                    }
                    if(validEnd) {
                        break;
                    }
                    validEnd = 1;
                }
                j = 0;
              
            }

        // start of a word
            else if(inWord == 0) {
                int validStart = 1;
                for(int c = 0; c < sizeof(otherTokens); c++) {
                    if(buf[i] == otherTokens[c]) {
                        validStart = 0;
                    } 
                }
                if(validStart) {
                    inWord = 1;
                    //words++;
                }
            }

        // in word
            if(inWord == 1) {
               currentWord[j] = buf[i];
               j++; 
            }
        }
    }
}

// use this for batch since we pass in a file
void readIn(int argc, char *filename, Token *front)
{
    char buf[100];
    char currentWord[45];
    int bytes;
    int inWord;
    int j = 0;
    while((bytes = read(bytes, buf, sizeof(buf))) > 0) {
        for(int i = 0; i < bytes; i++) {
        // end of a word
            if(isspace(buf[i])) {
                
                inWord = 0;
                currentWord[j] = '\0';
                createToken(currentWord, front);

                // loop through the word from the end. If there is an invalid character at the end, it will keep removing subsequent invalid characters until it finds a valid one
                int validEnd = 1;
                for(int w = strlen(currentWord) - 1; w >= 0; w--) {
                    for(int c = 0; c < sizeof(otherTokens); c++) {
                        if(currentWord[w] == otherTokens[c]) {
                            currentWord[w] = '\0';
                            validEnd = 0;
                        }
                    }
                    if(validEnd) {
                        break;
                    }
                    validEnd = 1;
                }
                j = 0;
              
            }

        // start of a word
            else if(inWord == 0) {
                int validStart = 1;
                for(int c = 0; c < sizeof(otherTokens); c++) {
                    if(buf[i] == otherTokens[c]) {
                        validStart = 0;
                    } 
                }
                if(validStart) {
                    inWord = 1;
                    //words++;
                }
            }

        // in word
            if(inWord == 1) {
               currentWord[j] = buf[i];
               j++; 
            }
        }
    }
}

int main(int argc, char *argv[]) {    
    Token *front = NULL;
    
    // if (argc == 1) {
    //     printf("No arguments provided\n");
    //     printf("stdin is a terminal\n");
    //     printf("Welcome to my shell!\n");
    //     } 
    // if (argc == 0) {
    //     printf("No arguments provided\n");
    //     printf("stdin is a terminal\n");
    //     printf("Welcome to my shell!\n");
    // } 

    int fd = isatty(STDIN_FILENO);
    if (fd == 1) {
        int exitStatus = 1;
        // stdin is a terminal || interactive mode
        printf("Welcome to my shell!\n");
        printf("mysh>");
        
        // read in the file
        
        while(exitStatus != 0)
        {
            // read every str from stdin
            for(int i = 1; i < argc; i++)
            {
                // this implements the link list of tokens
                // it does all the work of creating and storing data
                // linked list order is backwards 
                front = CreateToken(argv[i], front);
            }

            

            // search for exit in the linked list
            while(front != NULL)
            {
                if(front -> str == 'exit' || front -> str == 'die')
                {
                    // exit the loop if the line contains exit
                    exitStatus = 0;
                    break;
                }
                front = front->next;
            }

        }
        
        readIn(argc, argv, front);
    }
        
    // stdin is a file
    else {// fd == 0 
        readFile(argc, argv[1], front);
    }

    return 0;
}
