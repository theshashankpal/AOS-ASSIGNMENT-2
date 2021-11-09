#include "project.h"

int shm_fd;
void *ptr;
int size;
int *against;
int *busy_array;
queue *q;
table **score_sheet;


int main(int argc, char *argv[])
{
    printf("\n");
    shm_unlink(SHARED_MEMORY_NAME); // just a precaution.

    if (argc != 2)
    {
        printf("Error, no. of arguments you entered are too few or too many, please run it again : %d\n", argc);
        return 0;
    }

    FILE *fp;
    char *line = NULL;
    fp = fopen(argv[1], "r"); // opening file in read mode.
    size_t len = 0;
    ssize_t read;
    if (fp == NULL)
    {
        perror("File : ");
        exit(EXIT_FAILURE);
    }

    q = createQueue(sizeof(fixture));

    read = fscanf(fp, "%d", &size);

    printf("No. of teams : %d\n", size);

    // Reading fixtures from file and storing them in the queue.
    while (1)
    {
        int first;
        int second;
        read = fscanf(fp, "%d%d", &first, &second);
        if (read == -1)
            break;
        fixture team;
        team.first = first - 1;
        team.second = second - 1;
        enqueue(q, &team);
    }

    printf("Fixtures have been read from file, now gonna start scheduling them : \n");

    // Closing the file descriptor and freeing up the memory used to read lines from a file.
    fclose(fp);

    pid_t manager_array[size];

    /*
        Calculating allocation size of shared memory :
        - We're putting two int arrays whose size will be equivalent to total number of teams(n).
        - And lastly a 2D struct _SS array whose outer size will be (n) and inner size will be (n-1).
    */
    int allocation_size = size * (sizeof(int)) + size * (sizeof(int)) + (size * size - 1) * (sizeof(struct _SS));

    // Creating shared memory segment
    shm_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR | O_EXCL, 0660);
    if (shm_fd == -1)
    {
        perror("Main Shared Memory ");
        exit(EXIT_FAILURE);
    }

    // Allocating space for our defined struct in shared memory.
    if (ftruncate(shm_fd, allocation_size) == -1)
    {
        perror("Ftruncate ");
        exit(EXIT_FAILURE);
    }

    // Mapping it to this process's virtual space.
    ptr = mmap(NULL, allocation_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("Main Mapping ");
        exit(EXIT_FAILURE);
    }

    // Declaration for our 2d array, so we can use it easily without bothering with pointers.
    SS(*result)
    [size - 1];

    against = (int *)ptr;                    // setting base address for first array i.e. against_array
    result = ptr + (2 * size * sizeof(int)); // setting base address for 2d array.
    busy_array = ptr + (size * sizeof(int)); // setting base address for second array. i.e. base_array

    for (size_t i = 0; i < size; i++)
    {
        busy_array[i] = 1; // making all 1's indicating every manager is available for schedule.
    }

    char totalTeams[11];
    sprintf(totalTeams, "%d", size); // to pass size = no. of total teams to each manager process.

    for (size_t i = 0; i < size; i++)
    {
        pid_t child = fork();
        if (child == 0)
        {
            char processIndex[11];
            sprintf(processIndex, "%ld", i); // to pass an index which will tell each manager process what their index is.
            char *args[] = {"./manager", processIndex, totalTeams, NULL};
            execv("./manager", args);
        }
        else
        {
            manager_array[i] = child; // storing all pids in an array for easy referencing.
        }
    }

    printf("\n");

    // scheduling matches
    fixture team;
    scheduling(manager_array, &team);

    // waiting for the last manager_process to finish its match
    while (!(busy_array[team.first] == 1 && busy_array[team.second] == 1))
        ;

    // waiting for the last manager_process to remove all file descriptors to shared memory.
    siginfo_t sig;
    waitid(P_PID, manager_array[team.first], &sig, WSTOPPED);

    // now as all manager_process's work have been completed, killing them.
    for (size_t i = 0; i < size; i++)
    {
        kill(manager_array[i], SIGTERM);
    }

    score_sheet = (table **)calloc(size, sizeof(table *)); // making a local data structure to capture or make the final score table, from results that are updated by manager processes

    // allocating all ds, doing this way because we also have to sort them in future , so having a pointer to struct in an array works.
    for (size_t i = 0; i < size; i++)
    {
        score_sheet[i] = (table *)calloc(1, sizeof(table));
        score_sheet[i]->mine_index = i;
    }

    // Making final table.
    tableCreation(result);

    // sorting the table by the given constraint. Used selection sort.
    sort();

    // Printing the table.
    printingTable();

    // Freeing up all the used local and shared memory.
    for (size_t i = 0; i < size; i++)
    {
        free(score_sheet[i]);
    }

    free(score_sheet);

    printf("\n\n");

    // Clearing queue any unused ds that is still in the queue.
    clearQueue(q);

    // Destroying the queue.
    destroyQueue(q);

    // Unmapping the shared object from process's virtual space.
    munmap(ptr, sizeof(allocation_size));

    // Closing the file descriptor of shared memory segment.
    close(shm_fd);

    // Unlinking shared memory segment , so it can be destroyed.
    shm_unlink(SHARED_MEMORY_NAME);
}


