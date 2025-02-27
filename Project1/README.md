Andrew Pepe
adp204

Jake Kim
jgk98

Our program test plans are conceptually simple. Here is a breakdown of how we implemented each file program:

**mymalloc.c**
mymalloc.c was designed to meet all of the requirements needed provided by the p1.pdf in canvas. We define our MEMLENGTH (the size of the heap) to be int 4096. We also created a static int variable initializedCheck to 0, meant to be used at any point in the program to check to see if the heap has been initialized or not. 

    _datastructure for chunk_
    Our data structure for chunks was designed to replicate a chunk in a simple but accurate way. It is comprised of the following:
    size_t size <used to hold the size of the chunk (header + payload)>
    an int variable isFree which tells us if the chunk is free (1) or not (0)
    a pointer to the next chunk <used for linkedList properties>
    a pointer to the prev chunk <used for linkedList properties>
    
    _our initializer method_
    our initializer method creates a chunk pointer called head that starts at the beginning of our heap. It's size is == MEMLENGTH since we need to start the heap with one big chunk. We set the head isFree int checker to free (1), then NULLed our next and prev pointer variables. Before the method exits, it calls atexit(leakDetector()) as per project requirement
    _mymalloc implementation_
    our malloc implementation first checks to see if the heap has been initialized by looking at the intitializedCheck variable and calls our initializer method if not. Then it does the following:
    creates a chunk pointer current which starts at the beginning of the heap
    creates a size_t variable alignedSize which sets the alignment for the heap
    checks to see if the current chunk pointer is free an dhas enough space
    then splits the chunk using our chunk split algorithm
    then it returns the current chunk (casted as a char *) + the size of the chunk

    if the while loop exits, we print a message to stderr and return null.

    _our free method_
    our free method starts the same was as mymalloc(), then goes through a couple of checks
    1. if the ptr to the chunk that is passed in is NULL
    2. if the target chunk (the chunk that we are trying to free) is already free, and if it is print a message to stderr. 

    Then the program utilizes our search algorithm. The search algo creates a chunk pointer that starts at the beginning of the heap and searches the heap (start to end) and checks to see if the chunk it is currently pointing to is the same as our target chunk. If it is it frees it, then breaks the loop. If it doesn't find the target chunk we print a message to stderr. 

    Then the program begins checking the heap for any adjacent free chunks. If it finds any it coalesces them into one big free chunk. 
    
    _our leak detector method_
    our leak detector method works similarly to our free method. We initialize a chunk pointer and iterate through the entire heap to find if any remaining allocated chunks. If it does, it increments an int variable that stores information regarding the number of chunks still allocated in the heap. If the value of this variable > 0, it prints a message to stderr. 

**memtest.c**
    memtest.c is the given program used to test basic aspects of the code (correctly filling up and freeing the whole space). In this code, we changed HEADERSIZE to equal 32 bytes, as that is the size of our header.
    
**memgrind.c**
memgrind.c is our stress test program. It is probably my favorite part of the entire project. 
    We use <sys/time.h> in order to utilize the gettimeofday() method/ function, as per project requirements. The workload is divided up into 3 tasks, each that get their own respective function. They are numbered from 1-3, meant to mimic the project instructions. The reason why we made them their own functions is for code readability and reusability. In our main function, we grab the start time before we execute the workload (tasks 1-3) 50 times. Then we execute the tasks 50 times using a for loop. Once the loop exits we grab the end time and calculate the average in seconds. When memgrind.c exits, it returns this average to terminal.
    We then created two more tasks for the program to execute. One task tested if free() would work on a pointer that is not exactly on the start of a payload. The other tests if free() could work on an object that has already been freed.

Note for memgrind: the minimum chunk size is greater than 32 bits (40 bits exactly). Therefore, a maximum of 101 chunks can be created if the payload is one bit.

Overall this project proved itself to be both challenging and engaging. Figuring out a design idea that was both simple and minimal (two ingredients for a successful program) was what took the most time (besides debugging) during our project implementation. Specifically, we kept having issues when the last chunk would be at the exact end of the heap, as it would attempt to create a new chunk after, thinking that it had enough space to do so, and we fixed this by manually checking when this case ocurred and avoided making a new chunk.
