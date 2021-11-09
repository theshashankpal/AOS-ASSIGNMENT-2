Name : Shashank Pal
Student ID : 2021H1030064G
-----------------------------------------------------------------------------------------------------------------------
List of Directories :

assignment_2 : It houses all the files related to this assignment.
assignment_2/src : It contains all the needed source and header files.
-----------------------------------------------------------------------------------------------------------------------
List of Files :

All source and header files are included in directory assignment_2/src :

scheduler.c : It creates n (total no. of teams) processes. Reads the schedules from the file and schedules
                them by giving signal to these processes.

manager.c : I'm taking manager, team and stadium as a single entity. As making them different didn't make sense and
            also question didn't ask for it specifically. This file simualte the match and updates the result.

queue.c : File containing the code for data structure queue.

implementation.c : Contains all the implementations of different functions used.

project.h : Simple header file indicating what #includes are used,
             what functions and custom data structures that I'm using.
             And , what are the pre and post conditions of a particular function.
-----------------------------------------------------------------------------------------------------------------------
HOW TO COMPILE :

First : cd/assignment_2
Then run : make clean (to remove any unnecessary object and executable files)
Lastly run : make
-----------------------------------------------------------------------------------------------------------------------
HOW TO RUN :

First make sure you're in /assignment_1 directory.
Then to run program, type following with required amount of arguments : ./output (path of a file)

For ex : ./output src/fixtures.txt
-----------------------------------------------------------------------------------------------------------------------
Description Of The Structure Of My Program :

- First by running ./output (path of a a file) we start scheduler.c.

- Then scheduler.c sets up the shared memory and that our program will use.
    Will read the schedules from the file given in the argument and store it in a queue.
    Will loop until queue becomes empty and gives signal to the process which has to simulate the match.
    After all the schedules have been completed, it will terminate all the running processes.
    Will create the table from the result that is stored in the shared memory.
    Then, it will sort the table and print it.
    (I've used named Shared Memory from POSIX interface.)

- Manager.c will link itself to the shared memory
    Will simulate the match using rand() and sleep.
    After simulation , updates the result into the shared memory.
    And exit.

- After everyone has exited and closed their file descriptors for shared memory ,
    scheduler.c will unlink the shared memory.
    And exit.
-----------------------------------------------------------------------------------------------------------------------
BUGS :
None (To my limited knowledge)

-----------------------------------------------------------------------------------------------------------------------