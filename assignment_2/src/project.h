//
// Created by shashank on 14/10/21.
//

#ifndef ASSIGNMENT_H
#define ASSIGNMENT_H

#include <stdio.h>
#include <stdlib.h>
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

/*
Pre Condition : The table score_sheet should be formed.
Post-Condition : Sorts the table based on the given conditions in the question.
*/
void sort();

/*
Pre Condition : All schedules must be completed , and their outcomes properly updates
Post-Condition : Creates a score_sheet from the help of outcomes, which then can be printed showing the final summary.
*/
void tableCreation(SS (*)[]);

/*
Pre Condition : All the schedules must be read and stored in a queue
Post-Condition : Gives signal to processes(manager/stadium) after reading the schedule from a queue. And runs until queue becomes empty.
*/
void scheduling(int *, fixture *);

/*
Pre Condition : The table score_sheet should be formed.
Post-Condition : Prints the table.
*/
void printingTable();

/*
Pre Condition : match is against whom must be known via shared memory.
Post-Condition : Gives the score using rand() and also updates it in the shared memory.
*/
void simulate_match(SS (*)[]);

#endif //ASSIGNMENT_H
