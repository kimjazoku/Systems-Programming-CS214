Jake Kim
Andrew Pepe

During the implementation of this program, we utilized a DEBUG macro as learned in lecture. We did not need to inject print statements thanks to our macro. This also allowed us to print specific data to terminal during runtime. We utilized the same table structure provided to us in p2.pdf, and maintained a 2 decimal place floating point format per word frequency. We decided to store each word into a linked list, and each file inside of a FileNode struct array. By doing so, it allowed us to easily traverse the words and frequencies with respect to its original file. During testing, we wrote numerous text files containing words with special edge cases, words with no letters (containing digits only), as well as words that repeated themselves numerous times. We stress tested our directory traversing alogoritm by passing through directories with different lengths as well as different numbers of sub-directories. 

Overall, the project implementation required numerous code revisions, and extensive debugging. Project difficulty was a 6/10. The instructions gave the illusion that the program implementation was simpler than it actually was. 
