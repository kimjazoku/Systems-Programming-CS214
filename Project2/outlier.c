#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>


#define BUFLEN 48
#define DEBUG 1

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
        //printf("%s ", newNode->word);
    }

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
            
        //treat "Hello" == "hello"
            buf[i] = tolower(buf[i]);


        //end of a word
            if(isspace(buf[i])) {
                
                inWord = 0;
                currentWord[j] = '\0';

            //loop through the word from the end. If there is an invalid character at the end, it will keep removing subsequent invalid characters until it finds a valid one
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
                }

            //loop through linked list to find if the word was already found in the file
                Node *currentNode = FindWord(currentWord, front);
                if(currentNode == NULL) {

                //if it doesn't exist, add it to the front
                    front = CreateNode(strdup(currentWord), front);
                }
                else {
                    currentNode->freq++;
                }
                
                j = 0;
              
            }

        //start of a word
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

        //in word
            if(inWord == 1) {
               currentWord[j] = buf[i];
               j++; 
            }
        }
    }

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
        }

    //loop through linked list to find if the word was already found in the file
        Node *currentNode = FindWord(strdup(currentWord), front);
        if(currentNode == NULL) {

        //if it doesn't exist, add it to the front
            front = CreateNode(strdup(currentWord), front);
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

void PrintTable(FileNode *arr, int size) {

    printf("\tWord \t | \t");
    
    Node *allWords = NULL;
    int totalWords = 0;

    for(int i = 0; i < size; i++) {
        printf("%s \t | \t", (*(arr+i)).fileName);

        Node *ptr = arr[i].front;
        while(ptr != NULL) {
            
        //See if current word is in AllWords. If not, add it.
            Node *currentNode = FindWord(ptr->word, allWords);
            if(currentNode == NULL) {
                allWords = CreateNode(ptr->word, allWords);
            }
        //if it is, increase TOTAL frequency  
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
        
        //finds amount of times word appears in file arr[i](should be under the correct letter)
            Node *wordForFile = FindWord(ptr->word, arr[i].front);

            if(wordForFile == NULL) {

                printf("%.2f \t | \t", 0.00);
                
            }
            else {
                float freakyFile = (float) wordForFile->freq / (float) arr[i].totalWords;
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
            Node *wordForFile = FindWord(temp->word, arr[i].front);
            
            if(wordForFile != NULL) {
                // calculate overall freq for the word
                float overall = (float) temp->freq / (float) totalWords;

                // calculate the word's freq in the file
                float freakyFile = (float) wordForFile->freq / (float) arr[i].totalWords;

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
                maxFile[i]->fileName = arr[i].fileName; // store the file
                if(wordFreq > maxFile[i]->gri)
                {
                    maxFile[i]->gri = wordFreq; // store the word's relative increase
                    maxFile[i]->maxWord = temp->word; // store the word
                }
 
                if(DEBUG)
                {
                   //printf("%s: %s == %f\n", arr[i].fileName, wordForFile->word, wordFreq);
                }
                //    printf("%s: %s\n", arr[i].fileName, temp->word);
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


void DirectoryTraversal(struct stat *fileInfo, char *fileName, FileNode *file) {
    
    char *extension = ".txt";
    size_t extSize = strlen(extension);

    

    if(stat(fileName, fileInfo) == 0) {
        if(S_ISREG(fileInfo->st_mode)) {
            if(DEBUG) {
                printf("Regular File\n");
            }
            
            if(strncmp(fileName + strlen(fileName) - extSize, extension, extSize) == 0) {
                if(DEBUG) {
                    printf("Text File\n");
                }
                
                file->front = wordCounter(fileName, file);
            }

        }

        else if(S_ISDIR(fileInfo->st_mode)) {

            
            if(DEBUG) {
                printf("Directory\n");
            }
            
            struct dirent *dEnt;
            
            DIR *directory = opendir(fileName);
            
            if (directory == NULL) {
                perror("Could not open directory");
                return;
            }
            while((dEnt = readdir(directory)) != NULL) {
                
                if(strcmp(dEnt->d_name, ".") != 0 && strcmp(dEnt->d_name, "..") != 0) {
                    
                    char newPath[1000];
                    
                    if(DEBUG) {
                        printf("%s\n", dEnt->d_name);
                    }
                    
                    strcpy(newPath, fileName);
                    strcat(newPath, "/");
                    strcat(newPath, dEnt->d_name);

                    DirectoryTraversal(fileInfo, newPath, file);

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


int main(int argc, char *argv[]) {

    if(DEBUG) {
        argc = 2;

        argv[1] = "abc";
        // argv[1] = "abc/a.txt";
        // argv[2] = "abc/b.txt";
        // argv[3] = "abc/c.txt";
    }
    
    FileNode *files = malloc((argc - 1) * sizeof(FileNode));
    
    
    for(int i = 1; i < argc; i++)
    {
        
        files[i - 1].front = NULL;
        files[i - 1].fileName = argv[i];
        
        struct stat fileInfo;
        DirectoryTraversal(&fileInfo, files[i-1].fileName, &files[i - 1]);
        
        files[i - 1].front = wordCounter(argv[i], &files[i-1]); // used to get the word count



        if(DEBUG) {

            printf("File: %s\n", files[i-1].fileName);
            printf("total words in file: %d\n", files[i-1].totalWords);

            Node *ptr = files[i-1].front;
            while(ptr != NULL) {
                printf("%s ", ptr->word);
                printf("(%d) ", ptr->freq);
                ptr = ptr->next;
            }
            printf("\n\n");
        } 
    }
    
    PrintTable(files, argc - 1);

    for(int i = 0; i < argc - 1; i++) {

        //free(&files[i]);
    }
    //free(files);

    return 0;
}
