#include "project.h"
#include "queue.h"

int shm_fd;
void *ptr;

typedef struct _fixture
{
    int first;
    int second;
} fixture;

int main(int argc, char *argv[])
{
    shm_unlink(SHARED_MEMORY_NAME);

    FILE *fp;
    char *line = NULL;
    fp = fopen("fixtures.txt", "r");
    size_t len = 0;
    ssize_t read;
    if (fp == NULL)
    {
        perror("File : ");
        exit(EXIT_FAILURE);
    }

    queue *q = createQueue(sizeof(fixture));

    read = getline(&line, &len, fp);
    int size = atoi(line);

    printf("No. of teams : %d\n", size);

    // fixture *schedule = (fixture *)calloc(size, sizeof(fixture));

    int count = 0;

    while ((read = getline(&line, &len, fp)) != -1)
    {
        int first = atoi(line);
        int second = atoi(line + 2);
        fixture temp;
        temp.first = first - 1;
        temp.second = second - 1;
        enqueue(q, &temp);
    }

    printf("Size of queue is %ld\n", getSize(q));

    printf("Fixtures read from file : \n");

    fclose(fp);
    if (line)
        free(line);

    pid_t manager_array[size];
    int allocation_size = size * (sizeof(int)) + size * (sizeof(int)) + (size * size - 1) * (sizeof(struct _SS));

    // Creating shared memory segment
    shm_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR | O_EXCL, 0660);
    if (shm_fd == -1)
    {
        perror("Main Shared Memory ");
        exit(4);
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
        exit(2);
    }

    // SS result[size-1];
    SS(*result)
    [size - 1];

    int *against = (int *)ptr;
    result = ptr + (2 * size * sizeof(int));
    int *busy_array = ptr + (size * sizeof(int));

    for (size_t i = 0; i < size; i++)
    {
        busy_array[i] = 1;
    }

    char temp1[256];
    sprintf(temp1, "%d", size);

    for (size_t i = 0; i < size; i++)
    {
        pid_t child = fork();
        if (child == 0)
        {
            char temp[256];
            sprintf(temp, "%ld", i);

            char *args[] = {"./manager", temp, temp1, NULL};
            execv("./manager", args);
        }
        else
        {
            manager_array[i] = child;
        }
    }

    fixture temp;

    while (!isEmpty(q))
    {

        dequeue(q, &temp);
        // printf("%d vs %d \n", temp.first, temp.second);
        if (busy_array[temp.first] == 1 && busy_array[temp.second] == 1)
        {
            busy_array[temp.first] = 0;
            busy_array[temp.second] = 0;
            against[temp.first] = temp.second;
            siginfo_t sig;
            waitid(P_PID, manager_array[temp.first], &sig, WSTOPPED);
            kill(manager_array[temp.first], SIGCONT);
        }
        else
        {
            enqueue(q, &temp);
        }
    }

    // int *against = (int *)ptr;
    // result = ptr + (size * sizeof(int));

    // printf("Against array : \n");
    // for (size_t i = 0; i < size; i++)
    // {
    //     printf("%d ", against[i]);
    // }

    // printf("\n");

    // for (size_t i = 0; i < size; i++)
    // {
    //     for (size_t j = 0; j < size - 1; j++)
    //     {
    //         result[i][j].mine = 100;
    //         result[i][j].against = 100;
    //     }
    // }

    // printf("Results that are stored in the shared memory \n");

    // for (size_t i = 0; i < size; i++)
    // {
    //     for (size_t j = 0; j < size - 1; j++)
    //     {
    //         printf("%d vs %d", result[i][j].mine, result[i][j].against);
    //         printf("\n");
    //     }
    // }

    while (!(busy_array[temp.first] == 1 && busy_array[temp.second] == 1));

    for (size_t i = 0; i < size; i++)
    {
        kill(manager_array[i],SIGTERM);
    }

    clearQueue(q);
    printf("Cleared queue\n");
    destroyQueue(q);
    printf("Destroyed queue\n");

    // Unmapping the shared object from process's virtual space.
    munmap(ptr, sizeof(allocation_size));

    // Closing the file descriptor of shared memory segment.
    close(shm_fd);

    // Unlinking shared memory segment , so it can be destroyed.
    shm_unlink(SHARED_MEMORY_NAME);

    // Destroying semaphore.
    // sem_destroy(sem);
    // exit(EXIT_SUCCESS);

    // // Initialization of a semaphore
    // sem = &(ptr->semaphore);
    // sem_init(sem, 1, 1);
}