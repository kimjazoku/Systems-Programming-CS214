#include "mymalloc.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    
    return 0;
}

void task1()
{
    for(int i = 0; i < 120; i++)
    {
        char *ptr = malloc(1);
        free(ptr);
    }
}

void task2() //Use malloc() to get 120 1-byte objects, storing the pointers in an array, then use free() to deallocate the chunks.
{
    char *ptr[120];
    for (int i = 0; i < 120; i++)
    {
        ptr[i] = malloc(1);
    }
    for (int i = 0; i < 120; i++)
    {
        free(ptr[i]);
    }
}

void task3() //Create an array of 120 pointers. Repeatedly make a random choice between (a) allocating a 1-byte object and adding the pointer to the array and (b) deallocating a previously allocated object (if any). Once 120 allocations have been performed, deallocate all objects.
{
    char *ptr[120];
    int count = 0;
    for (int i = 0; i < 120; i++)
    {
        int choice = rand() % 2;
        if (choice == 0)
        {
            ptr[count] = malloc(1);
            count++;
        }
        else
        {
            if (count > 0)
            {
                free(ptr[count - 1]);
                count--;
            }
        }
    }
    for (int i = 0; i < count; i++)
    {
        free(ptr[i]);
    }
}