#include "project.h"
#include <time.h>

int shm_fd;
void *ptr;
int *against;
int *busy_array;
int count;
int me;
int opponent;
int size;

int main(int argc, char *argv[])
{
    me = atoi(argv[1]);   // getting its own index
    size = atoi(argv[2]); // getting total no. of teams

    int allocation_size = size * (sizeof(int)) + (size * size - 1) * (sizeof(struct _SS)); // same formula as before.
    count = 0;

    while (1) // infinte loop
    {
        raise(SIGSTOP); // stopping itself.

        // opeing the shared memory.
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

        against = (int *)ptr;                    // setting base address for first array
        result = ptr + (2 * size * sizeof(int)); // setting base address for 2d array.
        busy_array = ptr + (size * sizeof(int)); // setting base address for second array.

        srand(time(NULL) + getpid()); // setting up the seed for random function , and as pid is unique , we'll get random number everytime.

        opponent = against[me]; // storing index of the opponent.

        simulate_match(result); // simulating match.

        // indicating that this manager_process has finished its match.
        busy_array[me] = 1;
        busy_array[opponent] = 1;

        // Unmapping the shared object from process's virtual space.
        munmap(ptr, sizeof(allocation_size));

        // Closing the file descriptor of shared memory segment.
        close(shm_fd);
    }
}

void simulate_match(SS (*result)[size-1])
{
    printf("Starting Match : Team %d vs Team %d\n", me + 1, opponent + 1);

    sleep(3);

    int mine_score = rand() % 6;
    int opponent_score = rand() % 6;

    // storing details of a match in the 2d shared array
    result[me][count].team = opponent;
    result[me][count].mine = mine_score;
    result[me][count++].against = opponent_score;

    printf("Match Ended : Team %d vs Team %d    Result : %d-%d\n", me + 1, opponent + 1, mine_score, opponent_score);
}

