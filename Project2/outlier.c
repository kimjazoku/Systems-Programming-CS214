#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define BUFLEN 48
#define DEBUG 0

char InvalidStart[] = { '(', '{', '[', '\"', '\'', };
char InvalidEnd[] = { ')', '}', ']', '\"', '\'', '?', ',', '.', '!' }; 


typedef struct Node {

    char *word;
    struct Node *next;
    int freq;
    char *fileName;

} Node;


// Loops through entire word linked list to find if the word already exists. If yes, return it and increase frequency. Otherwise, return NULL (so we can create the node)
Node *FindWord(char *word, Node *ptr) {

    while(ptr != NULL) {
        if(strcmp(word, ptr->word) == 0) {
            ptr->freq++;
            return ptr;
        }
        ptr = ptr->next;
    }
    return NULL;

}


Node *CreateNode(char *word, Node *front) {

    Node *newNode = malloc(sizeof(Node));
    newNode->freq = 1;
    newNode->word = word;

    if(front == NULL) {
        return newNode;
    }
    else {
        newNode->next = front;
        return newNode;
    }

}

void WordReader(char *filename) {

    char buf[BUFLEN];
    int spaces = 0;

    // open file
    int file = open(filename, O_RDONLY);
    if(file == -1) {
        perror("File not found");
    }

    // read file
    int bytes = read(file, buf, BUFLEN);
    if(bytes < 0) {
        perror("Error reading file");
        close(file);
    }

    else if(bytes == 0) {
        puts("end of file");
        close(file);
    }

    else {
        buf[bytes] = '\0'; // add in the terminator
        printf("%s\n", buf); // print out the buffer
    }
    
    while(bytes > 0) {
        for(int i = 0; i < bytes; i++) {
            if(isspace(buf[i])) {
                spaces++;
            }
        }
        bytes = read(file, buf, BUFLEN);
        printf("%s\n", buf);
    }


}

int wordCounter(char *filename)
{
    // not sure if this is the best way to do this but it works
    // we don't need a separate function for this, but it makes the main function cleaner
    int words = 0;
    int inWord = 0;
    int bytes;
    char buf[BUFLEN];
    int file = open(filename, O_RDONLY);
    
    if(file == -1) {
        perror("File not found");
    }
    while((bytes = read(file, buf, BUFLEN)) > 0) {
        for(int i = 0; i < bytes; i++) {
            if(isspace(buf[i])) {
                inWord = 0;
            }
            else if(inWord == 0) {
                inWord = 1;
                words++;
            }
        }
    }
    printf("Word count: %d\n", words);
    if(bytes < 0) {
        fprintf(stderr, "Error reading file %s\n", filename);
    }
    return words; // we can use this to keep track of how many words are in the file
}

int main(int argc, char *argv[]) {

    Node *front = NULL;
    int wordCount;
    for(int i = 1; i < argc; i++)
    {
        wordCount = wordCounter(argv[i]); // used to get the word count
        printf("Word count: %d\n", wordCount);   
    }
    




    return 0;
}

