#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h> // get time of day 
//#include <time.h>
#include "mymalloc.h"


#define SUCCESS 0 
#define FAILURE -1
#define RUNS 50

//gcc memgrind.c mymalloc.c -o memtest

int task1();
int task2();
int task3();
//int gettimeofday();

double avg;
double total;
double iter;



int main(int argc, char *argv[])
{
    struct timeval start, end;
    // long seconds, micros;

    gettimeofday(&start, NULL); // start timer. null value for tz bc we're not using it
    
    for(int i = 0; i < RUNS; i++) // execute workload 50 times
    {
        if(task1() == SUCCESS)
        {
             printf("Task 1 completed successfully\n");
        }
        else
        {
            printf("Task 1 failed\n");
        }
        
        if(task2() == SUCCESS)
        {
            printf("Task 2 completed successfully\n");
        }
        else
        {
            printf("Task 2 failed\n");
        }
        
        if(task3() == SUCCESS)
        {
            printf("Task 3 completed successfully\n");
        }
        else
        {
            printf("Task 3 failed\n");
        }
    }

    gettimeofday(&end, NULL); // end timer
    long seconds = end.tv_sec - start.tv_sec;
    long micros = end.tv_usec - start.tv_usec;
    total = (double)seconds + (micros / 1000000.0);
    double avg_time = total / RUNS; // convert to milliseconds
    return printf("The average time for performing the workload was %f seconds.", avg_time);
}

int task1()
{
    for(int i = 0; i < 120; i++)
    {
        char *ptr = malloc(1);
        if(ptr == NULL)
        {
            return FAILURE;
        }
        free(ptr);
        // printf("task 1: success. iteration %d\n", i);
    }
    return SUCCESS;
}

int task2() //Use malloc() to get 120 1-byte objects, storing the pointers in an array, then use free() to deallocate the chunks.
{
    char *ptr[120];
    for (int i = 0; i < 120; i++)
    {
        ptr[i] = malloc(1);
        if (ptr[i] == NULL)
        {
            return FAILURE;
        }
        // printf("task 2: allocation success. iteration %d\n", i);
    }
    for (int i = 0; i < 120; i++)
    {
        free(ptr[i]);
        // printf("task 2: deallocation success. iteration %d\n", i);
    }
    return SUCCESS;
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
            if(ptr[count] == NULL)
            {
                return FAILURE;
            }
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
    return SUCCESS;
}
