//
// Created by shashank on 14/10/21.
//

#ifndef ASSIGNMENT_H
#define ASSIGNMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h> /* for semaphores */
#include <unistd.h>    /* for fork() and execv */
#include <sys/mman.h>  /*Prot_READ constants*/
#include <mqueue.h>    /* "O_CREAT" O_Constants*/
#include <wait.h>      /* SIGCONT , SIGSTOP*/

#define SHARED_MEMORY_NAME "/SYSSHAREDMEMORY"

#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

#define SHARED_MEMORY_NAME "/SYSSHAREDMEMORY"

extern int shm_fd;

typedef struct _SS
{   
    int team;
    int mine;
    int against;
} SS;


struct shared_memory_structure
{
    int *against;
    SS *sheet;
};

#endif //ASSIGNMENT_H
