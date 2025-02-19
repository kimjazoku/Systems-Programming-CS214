#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"
#define MEMLENGTH 4096

static int initializedCheck = 0;

static union {
    char bytes[MEMLENGTH];
    double not_used;
} heap;

typedef struct chunk
{
    size_t size; // size of the chunk (header + payload)
    int isFree; // 1 if free, 0 if not
    struct chunk *next; // pointer to the next chunk
    struct chunk *prev; // pointer to the previous chunk
} chunk;

void static initializer()
{
    chunk *head = (chunk *)heap.bytes; // remember that we need to start the heap with a big ass chunk
    head->size = MEMLENGTH; 
    head->isFree = 1;
    initializedCheck = 1;
}

void * mymalloc(size_t size, char *file, int line)
{
    if(initializedCheck == 0) // if the heap has not been initialized
    {
        initializer();
    }

    chunk *current = (chunk *)heap.bytes; // start at the beginning of the heap
    while(current != NULL) // if null, then we have reached the end of the heap
    {
        if(current->isFree == 1 && current->size >= size) // current chunk is free and has enough space
        {
            if(current->size > size + sizeof(chunk)) // if there is enough space, split the chunk
            {
                chunk *newChunk = (chunk *)((char *)current + sizeof(chunk) + size);
                newChunk->size = current->size - size - sizeof(chunk);
                newChunk->isFree = 1;
                newChunk->next = current->next;
                newChunk->prev = current;
                current->size = size;
                current->isFree = 0;
                current->next = newChunk;
                if(newChunk->next != NULL) // if next reaches the end of the heap, loop back into itself
                {
                    newChunk->next->prev = newChunk;
                }
            }
            return (char *)current + sizeof(chunk);
        }
        current = current->next;
    }
    // this algo prototype is to coallesce the heap if there are multiple adjacent free chunks
        chunk *newCurrent = (chunk *)heap.bytes;
        while (newCurrent != NULL)
        {
            if (newCurrent->isFree == 1 && newCurrent->next != NULL && newCurrent->next->isFree == 1)
            {
                newCurrent->size += newCurrent->next->size + sizeof(chunk);
                newCurrent->next = newCurrent->next->next;
                if (newCurrent->next != NULL)
                {
                    newCurrent->next->prev = newCurrent;
                }
            }
            newCurrent = newCurrent->next;
        }
    printf("malloc: Unable to allocate %zu bytes (source.c1000)\n", size);
    return NULL;
}

void myfree(void *ptr, char *file, int line)
{
    if(initializedCheck == 0)
    {
        initializer();
    }

    if(ptr == NULL)
    {
        return;
    }

    chunk *current = (chunk *)((char *)ptr - sizeof(chunk));
    if(current->isFree == 1)
    {
        printf("free: Attempting to free unallocated memory (source.c1000)\n");
        return;
    }
    current->isFree = 1;

    free(ptr);
}



void leakDetector(char *ptr)
{
    chunk *current = (chunk *)heap.bytes;
    while(current != NULL)
    {
        if((char *)current + sizeof(chunk) == ptr)
        {
            current->isFree = 1;
            return;
        }
        current = current->next;
    }
    printf("Memory leak detected\n");
}