#include "project.h"
#include <time.h>

int shm_fd;
void *ptr;

int main(int argc, char *argv[])
{
    int me = atoi(argv[1]);
    int size = atoi(argv[2]);

    int allocation_size = size * (sizeof(int)) + (size * size - 1) * (sizeof(struct _SS));

    shm_fd = shm_open(SHARED_MEMORY_NAME, O_RDWR, 0660);
    if (shm_fd == -1)
    {
        perror("Child Shared Memory");
        return 1;
    }

    // Mapping shared segment to process's virtual space.
    ptr = mmap(NULL, allocation_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("Main Mapping ");
        exit(2);
    }

    SS(*result)
    [size - 1];

    int *against = (int *)ptr;
    result = ptr + (2 * size * sizeof(int));
    int *busy_array = ptr + (size * sizeof(int));

    int count = 0;

    while (1)
    {
        raise(SIGSTOP);
        srand(time(0));
        int opponent = against[me];
        printf("Starting Match : Team %d vs Team %d\n",me+1,opponent+1);
        int mine_score = rand() % 5;
        int opponent_score = rand() % 5;
        result[me][count].against = opponent;
        result[me][count].mine = mine_score;
        result[me][count].against = opponent_score;
        sleep(3);
        printf("Match Ended : Team %d vs Team %d    Result : %d-%d\n",me+1,opponent+1,mine_score,opponent_score);
        busy_array[me]=1;
        busy_array[opponent]=1;
    }
}
