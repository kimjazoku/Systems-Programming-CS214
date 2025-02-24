#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

int task1();
int task2();
int task3();

int main(int argc, char *argv[])
{
    int t1, t2, t3;
    t1 = task1();
    if(t1 == 0)
    {
        printf("Task 1 completed successfully\n");
    }
    else
    {
        printf("Task 1 failed\n");
    }
    t2 = task2();
    if(t2 == 0)
    {
        printf("Task 2 completed successfully\n");
    }
    else
    {
        printf("Task 2 failed\n");
    }
    t3 = task3();   
    if(t3 == 0)
    {
        printf("Task 3 completed successfully\n");
    }
    else
    {
        printf("Task 3 failed\n");
    }
    return 0;
}

int task1()
{
    for(int i = 0; i < 120; i++)
    {
        char *ptr = malloc(1);
        if(ptr == NULL)
        {
            return -1;
        }
        free(ptr);
        // printf("task 1: success. iteration %d\n", i);
    }
    return 0;
}

int task2() //Use malloc() to get 120 1-byte objects, storing the pointers in an array, then use free() to deallocate the chunks.
{
    char *ptr[120];
    for (int i = 0; i < 120; i++)
    {
        ptr[i] = malloc(1);
        if (ptr[i] == NULL)
        {
            return -1;
        }
        // printf("task 2: allocation success. iteration %d\n", i);
    }
    for (int i = 0; i < 120; i++)
    {
        free(ptr[i]);
        // printf("task 2: deallocation success. iteration %d\n", i);
    }
    return 0;
}

int task3() //Create an array of 120 pointers. Repeatedly make a random choice between (a) allocating a 1-byte object and adding the pointer to the array and (b) deallocating a previously allocated object (if any). Once 120 allocations have been performed, deallocate all objects.
{
    char *ptr[120];
    int count = 0;
    int allocCount = 0;
    while(allocCount < 120)
    {
        int choice = rand() % 2;
        if (choice == 0)
        {
            ptr[count] = malloc(1);
            count++;
            allocCount++;
        }
        else
        {   //free if we have something allocated
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
    return 0;
}