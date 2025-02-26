#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"
#include <stddef.h>

#define MEMLENGTH 4096

//gcc memtest.c mymalloc.c -o memtest

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
    size_t allignedSize = (size + 7) & ~7;


    while(current != NULL) // if null, then we have reached the end of the heap
    {
        if(current->isFree == 1 && current->size >= size) // current chunk is free and has enough space
        {
            if(current->size >= size + sizeof(chunk)) // if there is enough space, split the chunk
            {
                chunk *newChunk = (chunk *)((char *)current + sizeof(chunk) + allignedSize);
                newChunk->size = current->size - allignedSize - sizeof(chunk);
                newChunk->isFree = 1;
                newChunk->next = NULL;
                newChunk->prev = current;
                
                current->size = allignedSize;
                current->isFree = 0;
                current->next = newChunk;
                
                //printf("isFree = %d\n", current->isFree);


                if(newChunk->next != NULL) // if next reaches the end of the heap, loop back into itself
                {
                    newChunk->next->prev = newChunk;
                }

            }
            return (char *)current + sizeof(chunk);
        }
        current = current->next;
    }

    // while(current != NULL) {
    //     if(current->size > size && current->isFree == 1) {
            
    //     }
    // }

    
    fprintf(stderr, "malloc: Unable to allocate %zu bytes (source.c1000)\n", size);
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

    chunk *target = (chunk *)((char *)ptr - sizeof(chunk));

    //printf("isFree = %d\n", target->isFree);
    if(target->isFree == 1)
    {
        fprintf(stderr, "free: Attempting to free unallocated memory (source.c1000)\n");
        return;
    }
    chunk *current = (chunk*) heap.bytes;

    while (current != NULL) {
        if(current == target) {

            current->isFree = 1;
            break;
        }
        current = current -> next;
    }
    if(current == NULL) {
        fprintf(stderr, "free: Inappropriate pointer (source.c:1000)\n");
        return;
    }
    
    //mark the chunk as free
    target->isFree = 1;
    
    //2nd iteration of coalescing
    if (target->next != NULL && target->next->isFree == 1)
    {
        target->size += target->next->size;
        target->next = target->next->next;
        if (target->next != NULL)
        {
            target->next->prev = target;
        }
    }

    if (target->prev != NULL && target->prev->isFree == 1)
    {
        target->prev->size += target->size;
        target->prev->next = target->next;
        if (target->next != NULL)
        {
            target->next->prev = target->prev;
        }
    }

    // // this checks the next adjacent chunk and coalesces it if it is free
    // if (target->next != NULL && target->next->isFree == 1)
    // {
    //     target->prev->size += target->size;
    //     target->prev->next = target->next;
    //     if (target->next != NULL)
    //     {
    //         target->next->prev = target;
    //     }
    // }

    // // this checks the previous adjacent chunk and coalesces it if it is free
    // if (target->prev != NULL && target->prev->isFree == 1)
    // {
    //     target->prev->size += target->size;
    //     target->prev->next = target->next;
    //     if (target->next != NULL)
    //     {
    //         target->next->prev = target->prev;
    //     }
    // }
}



static void leakDetector()
{
    chunk *current = (chunk *)heap.bytes;
    int leakedChunks = 0;
    size_t leakedBytes = 0;

//loop through metadata
    while(current != NULL) {

    //if there is a non-free chunk, incriment leaked chunks by 1 and bytes by the bytes in said chunk
        if(current->isFree == 0) {
            leakedChunks += 1;
            leakedBytes += current->size;
        }

        current = current->next;
    }

    if(leakedBytes > 0) {
        fprintf(stderr, "Memory leak detected: %d bytes in %d objects.\n", (unsigned int) leakedBytes, leakedChunks);
    }

}
