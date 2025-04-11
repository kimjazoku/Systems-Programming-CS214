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

void *CreateToken(char *word, Token *front) {
    Token *newToken = malloc(sizeof(Token));
    newToken->str = word;

    if(DEBUG) {
        // printf("%s ", newNode->word);
    }
    
    while(front != NULL) {
        if (front->next == NULL) {
            front->next = newToken;
            newToken->next = NULL;
            break;
        }

        front = front->next;
    }
}

// use this for batch since we pass in a file
void readIn(int argc, char *filename)
{
    char buf[100];
    int bytes;
    while((bytes = read(bytes, buf, sizeof(buf))) > 0) {
        for(int i = 0; i < bytes; i++) {
        // end of a word
            if(isspace(buf[i])) {
                
                inWord = 0;
                currentWord[j] = '\0';

            // loop through the word from the end. If there is an invalid character at the end, it will keep removing subsequent invalid characters until it finds a valid one
                int validEnd = 1;
                for(int w = strlen(currentWord) - 1; w >= 0; w--) {
                    for(int c = 0; c < sizeof(InvalidEnd); c++) {
                        if(currentWord[w] == InvalidEnd[c]) {
                            currentWord[w] = '\0';
                            validEnd = 0;
                        }
                    }
                    if(validEnd) {
                        break;
                    }
                    validEnd = 1;
                }

            // loop through linked list to find if the word was already found in the file
                Node *currentNode = FindWord(currentWord, front);
                if(currentNode == NULL) {

                    int hasLetter = 0;
                    for(int j = 0; j < strlen(currentWord); j++) {
                        if(isalpha(currentWord[j])) {
                            hasLetter = 1;
                        }
                    }
                //if it doesn't exist, add it to the front
                    if(hasLetter) {
                        front = CreateNode(strdup(currentWord), front);
                    }
                }
                else {
                    currentNode->freq++;
                }
                
                j = 0;
              
            }

        // start of a word
            else if(inWord == 0) {
                
                int validStart = 1;

                for(int c = 0; c < sizeof(InvalidStart); c++) {
                    if(buf[i] == InvalidStart[c]) {
                        validStart = 0;
                    } 
                }

                if(validStart) {
                    inWord = 1;
                    words++;
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
    
    if (argc == 1) {
        printf("No arguments provided\n");
        printf("stdin is a terminal\n");
        printf("Welcome to my shell!\n");
        } 
    if (argc == 0) {
        printf("No arguments provided\n");
        printf("stdin is a terminal\n");
        printf("Welcome to my shell!\n");
    } 


    else{
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
                // linked list order is backwards 
                CreateToken(argv[i], front);
            }

            

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
        
        readIn(argc, argv[1]);

        
    }

    }

    return 0;
}
