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

static void leakDetector();

static void initializer()
{
    chunk *head = (chunk *)heap.bytes; // remember that we need to start the heap with a big ass chunk
    head->size = MEMLENGTH; 
    head->isFree = 1;
    head->next = NULL;
    head->prev = NULL;
    initializedCheck = 1;

    atexit(leakDetector);
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
    // printf("malloc: Unable to allocate %zu bytes (source.c1000)\n", size);
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
        exit(2);
    }
    //mark the chunk as free
    current->isFree = 1;

    // this checks the next adjacent chunk and coalesces it if it is free
    if (current->next != NULL && current->next->isFree == 1)
    {
        current->prev->size += current->size;
        current->prev->next = current->next;
        if (current->next != NULL)
        {
            current->next->prev = current;
        }
    }

    // this checks the previous adjacent chunk and coalesces it if it is free
    if (current->prev != NULL && current->prev->isFree == 1)
    {
        current->prev->size += current->size;
        current->prev->next = current->next;
        if (current->next != NULL)
        {
            current->next->prev = current->prev;
        }
    }



    // right after we free a chunk we need to coalesce it with adjecent free chunks asap

    //myfree(ptr);
}



static void leakDetector()
{
    chunk *current = (chunk *)heap.bytes;
    int leakedChunks = 0;
    size_t leakedBytes = 0;

    while(current != NULL) {

        if(current->isFree = 0) {
            leakedChunks += 1;
            leakedBytes += current->size;

        }

        current = current->next;
    }

    if(leakedBytes > 0) {
        fprintf(stderr, "Memory leak detected: %d bytes in %d objects.\n", (int) leakedBytes, leakedChunks);
    }

}
