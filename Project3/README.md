Andrew Pepe adp204
Jake Kim    jgk98

Our Test Plan
-------------
To test our implementation of the shell, we came up with a handful of tests, ranking simple to complex.

**The simple tests** 
We first performed simple tests for specific shell functions (i.e. pwd, cd, ls, etc.) to ensure that our shell worked correctly and threw the appropriate errors. This initial testing phase exposed a lot of holes in error tossing,
so we spent some time using VS Code's built in debugger to go through our file line by line. 

After the basics were fixed and retested, we then began to ramp up the testing complexity. 
Here are some examples of commands we used to test our shell:
  echo hello > test.txt
  echo hello | cat
  cat < test2.txt (where test 2 contained inputs like "echo hello cd testF pwd")
  cd whatever
  pwd

**The more complex tests**
For this portion of testing our program, we began to stress test the shell by invoking numerous wildcard calls
Some examples of our testing include the following:
ls *.txt
echo test*.txt

Then we continued to test further on other functions
For example:
  echo "first"
  and echo "second"
  or echo "third"
  echo foo bar baz | tr a-z A-Z

By creating more complex test cases, we found more holes in our program (more imparticular running into a segfault issue by freeing our tokens too soon).

Overall, this project proved to be challenging and time consuming, with over 650 lines of code that needed constant editing/ revising.


