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
#include "queue.h"

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

#define TAB -10

// Score struct that is put in shared memory.
typedef struct _SS
{   
    int team;
    int mine;
    int against;
} SS;

// for final table
typedef struct _table
{
    int mine_index;
    int won;
    int lost;
    int tie;
    int goals_scored;
    int goals_conceded;
    int score;
} table;

// ds to store in a queue that is used for scheduling purposes.
typedef struct _fixture
{
    int first;
    int second;
} fixture;


extern int shm_fd;
extern void *ptr;
extern int size;
extern int *against;
extern int *busy_array;
extern queue *q;
extern table **score_sheet;

void sort();

void tableCreation(SS (*)[]);

void scheduling(int *, fixture *);

void printingTable();

#endif //ASSIGNMENT_H
