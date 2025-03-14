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

//open and read file
    int file = open(filename, O_RDONLY);
    if(file == -1) {
        perror("File not found");
    }
    int bytes = read(file, buf, BUFLEN);

}

int main(int argc, char *argv[]) {

    Node *front = NULL;

    


    return 0;
}
