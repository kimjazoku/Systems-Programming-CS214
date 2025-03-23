#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>


#define BUFLEN 48
#define DEBUG 0

char InvalidStart[] = { '(', '{', '[', '\"', '\'', };
char InvalidEnd[] = { ')', '}', ']', '\"', '\'', '?', ',', '.', '!' }; 


typedef struct Node {
    
    char *word;
    struct Node *next;
    int freq;

} Node;

typedef struct FileNode {
    
    Node *front;
    char *fileName;
    char *maxWord;
    int totalWords;
    float gri;
    
    
} FileNode;

FileNode *allFiles[100];
int fileCount = 0;

// Loops through entire word linked list to find if the word already exists. If yes, return it and increase frequency. Otherwise, return NULL (so we can create the node)
Node *FindWord(char *word, Node *ptr) {

    while(ptr != NULL) {
        if(strcmp(word, ptr->word) == 0) {
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

    if(DEBUG) {
        // printf("%s ", newNode->word);
    }
    newNode->next = front;
    return newNode;
}

Node *wordCounter(char *filename, FileNode *fileName)
{
    // not sure if this is the best way to do this but it works
    // we don't need a separate function for this, but it makes the main function cleaner
    int words = 0;
    int inWord = 0;
    int bytes;
    char buf[BUFLEN];
    int file = open(filename, O_RDONLY);
    

    Node *front = NULL;

    int j = 0;
    char currentWord[BUFLEN];

    if(file == -1) {
        perror("File not found");
    }
    while((bytes = read(file, buf, BUFLEN)) > 0) {
        for(int i = 0; i < bytes; i++) {
            
        // treat "Hello" == "hello"
            buf[i] = tolower(buf[i]);


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

    // end of file
    if(inWord == 1 && j > 0) {
        
        inWord = 0;
        currentWord[j] = '\0';

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
        Node *currentNode = FindWord(strdup(currentWord), front);
        if(currentNode == NULL) {

            int hasLetter = 0;
            for(int j = 0; j < strlen(currentWord); j++) {
                if(isalpha(currentWord[j])) {
                    hasLetter = 1;
                }
            }
            if(hasLetter) {
            // if it doesn't exist, add it to the front
                front = CreateNode(strdup(currentWord), front);
            }
        }
        else {
            currentNode->freq++;
        }
        
        j = 0;    }
    
    if(bytes < 0) {
        fprintf(stderr, "Error reading file %s\n", filename);
    }

    fileName->totalWords = words;

    return front; // we can use this to keep track of how many words are in the file
}

void PrintTable(FileNode **arr, int size) {

    printf("\tWord \t | \t");
    
    Node *allWords = NULL;
    int totalWords = 0;

    for(int i = 0; i < size; i++) {
        printf("%s \t | \t", (*(arr+i))->fileName);

        Node *ptr = arr[i]->front;
        while(ptr != NULL) {
            
        // See if current word is in AllWords. If not, add it.
            Node *currentNode = FindWord(ptr->word, allWords);
            if(currentNode == NULL) {
                allWords = CreateNode(ptr->word, allWords);
            }
        // if it is, increase TOTAL frequency  
            else {
                currentNode->freq += ptr->freq;
            }
            totalWords += ptr->freq;

            ptr = ptr->next;
        }

    }

    printf("Overall\n");

    printf("________________________________________________________________________________\n\n");

    Node *ptr = allWords; 

    while(ptr != NULL) {
        printf("\t%s \t | \t", ptr->word);

        for(int i = 0; i < size; i++) {
        
        // finds amount of times word appears in file arr[i](should be under the correct letter)
            Node *wordForFile = FindWord(ptr->word, arr[i]->front);

            if(wordForFile == NULL) {

                printf("%.2f \t | \t", 0.00);
                
            }
            else {
                float freakyFile = (float) wordForFile->freq / (float) arr[i]->totalWords;
                printf("%.2f \t | \t", freakyFile);
            }


        }

        printf("%.2f\n", (float) ptr->freq / (float) totalWords);
        ptr = ptr->next;
    }

    printf("\n");

    // what we need to do is find the word with the greatest relative increase in frequency with respect to overall frequency
    // then we need to print the word with the greatest relative increase in frequency with respect to overall frequency and the file it is in
    Node *temp = allWords;
    Node *max = NULL;
    float maxFreq = 0.0;
    FileNode *maxFile[size];
    // set 
    for (int i = 0; i < size; i++)
    {
        maxFile[i] = calloc(1, sizeof(FileNode));
    }

    while(temp != NULL) {

        for(int i = 0; i < size; i++) {
            Node *wordForFile = FindWord(temp->word, arr[i]->front);
            
            if(wordForFile != NULL) {
                // calculate overall freq for the word
                float overall = (float) temp->freq / (float) totalWords;

                // calculate the word's freq in the file
                float freakyFile = (float) wordForFile->freq / (float) arr[i]->totalWords;

                // calculate the word's freq relative to overall freq
                float wordFreq = freakyFile / overall;
                
                // track the word with the greatest relative increase
                if(wordFreq > maxFreq) {
                    maxFreq = wordFreq;
                    max = temp; // store the word (Node)

                    
                    // maxFile[i]->fileName = arr[i].fileName; // store the file
                    // maxFile[i]->gri = wordFreq; // store the word's relative increase
                    // maxFile[i]->maxWord = temp->word; // store the word
                }
                maxFile[i]->fileName = arr[i]->fileName; // store the file
                if(wordFreq > maxFile[i]->gri)
                {
                    maxFile[i]->gri = wordFreq; // store the word's relative increase
                    maxFile[i]->maxWord = temp->word; // store the word
                }
 
                if(DEBUG)
                {
                   // printf("%s: %s == %f\n", arr[i].fileName, wordForFile->word, wordFreq);
                }
                // printf("%s: %s\n", arr[i].fileName, temp->word);
            }
        }
        temp = temp->next;
    }

    // now print the word with the greatest relative increase in frequency with respect to overall frequency and the file it is in
    if (max != NULL)
    {
        for (int i = 0; i < size; i++)
        {
            printf("%s: %s", maxFile[i]->fileName, maxFile[i]->maxWord);
            if(DEBUG) {
                printf(" freq: %.2f", maxFile[i]->gri);
            }
            printf("\n");
        }
    }
}


int isTxtFile(char *fileName) {
    char *extension = ".txt";
    size_t extSize = strlen(extension);
    size_t fileNameSize = strlen(fileName);

    if(fileNameSize < extSize) {
        return 0;
    }

    return strcmp(fileName + fileNameSize - extSize, extension) == 0;
}


void DirectoryTraversal(struct stat *fileInfo, char *fileName) {
    
    char *extension = ".txt";
    size_t extSize = strlen(extension);

    
    if (stat(fileName, fileInfo) == 0) {
            if (S_ISREG(fileInfo->st_mode)) {
                if (isTxtFile(fileName)) { 
                    allFiles[fileCount] = malloc(sizeof(FileNode));
                    allFiles[fileCount]->fileName  = strdup(fileName);
                    allFiles[fileCount]->front     = NULL;
                    allFiles[fileCount]->maxWord   = NULL;
                    allFiles[fileCount]->totalWords= 0;
                    allFiles[fileCount]->gri       = 0.0f;
                    allFiles[fileCount]->front = wordCounter(fileName, allFiles[fileCount]);
                    fileCount++;
                }
            }

            else if(S_ISDIR(fileInfo->st_mode)) {

                
                if(DEBUG) {
                    // printf("Directory\n");
                }
                
                struct dirent *dEnt;
                
                DIR *directory = opendir(fileName);
                
                if (directory == NULL) {
                    perror("Could not open directory");
                    return;
                }
                while((dEnt = readdir(directory)) != NULL) {
                    
                    if(dEnt->d_name[0] != '.') {
                        
                        char newPath[1000];
                        
                        if(DEBUG) {
                            printf("%s\n", dEnt->d_name);
                        }
                        
                        strcpy(newPath, fileName);
                        strcat(newPath, "/");
                        strcat(newPath, dEnt->d_name);

                        DirectoryTraversal(fileInfo, newPath);

                    }
                    char *newPath;

                }
                closedir(directory);
            }
            else {
                if(DEBUG) {
                    printf("The fuck wrong witchu");
                }
            }
        }
        else {
            perror("sum wrong wit the file gang");
        }
}

void freeAllFiles(FileNode **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i] != NULL) {
            // free the list 
            Node *curr = files[i]->front;
            while (curr != NULL) {
                Node *temp = curr;
                curr = curr->next;
                free(temp->word);  
                free(temp);        
            }
            
            free(files[i]->fileName);
            
            free(files[i]);
        }
    }
}


int main(int argc, char *argv[]) {

    if(DEBUG) {
        argc = 2;

        argv[1] = "abc";
        // argv[1] = "abc/a.txt";
        // argv[2] = "abc/b.txt";
        // argv[3] = "abc/c.txt";
    }
    
    for (int i = 1; i < argc; i++) {
        struct stat fileInfo;
        if (stat(argv[i], &fileInfo) == 0) {
            if (S_ISDIR(fileInfo.st_mode)) {
                DirectoryTraversal(&fileInfo, argv[i]);
            } else if (S_ISREG(fileInfo.st_mode)) {
                if (isTxtFile(argv[i])) {
                    allFiles[fileCount] = malloc(sizeof(FileNode));
                    // initialize
                    allFiles[fileCount]->fileName  = strdup(argv[i]);
                    allFiles[fileCount]->front     = NULL;
                    allFiles[fileCount]->maxWord   = NULL;
                    allFiles[fileCount]->totalWords= 0;
                    allFiles[fileCount]->gri       = 0.0f;

                    allFiles[fileCount]->front = wordCounter(argv[i], allFiles[fileCount]);
                    fileCount++;
                }
            }
        }
    }
    
    PrintTable(allFiles, fileCount);

    freeAllFiles(allFiles, fileCount);

    return 0;
}
