#include "project.h"
#include "queue.h"

int shm_fd;
void *ptr;

typedef struct _fixture
{
    int first;
    int second;
} fixture;

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

#define TAB 10

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
        fixture team;
        team.first = first - 1;
        team.second = second - 1;
        enqueue(q, &team);
    }

    printf("Size of queue is %ld\n", getSize(q));

    printf("Fixtures read from file, now gonna start scheduling them : \n");

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

    char team1[256];
    sprintf(team1, "%d", size);

    for (size_t i = 0; i < size; i++)
    {
        pid_t child = fork();
        if (child == 0)
        {
            char team[256];
            sprintf(team, "%ld", i);

            char *args[] = {"./manager", team, team1, NULL};
            execv("./manager", args);
        }
        else
        {
            manager_array[i] = child;
        }
    }

    fixture team;

    printf("\n\n");

    while (!isEmpty(q))
    {

        dequeue(q, &team);
        // printf("%d vs %d \n", team.first, team.second);
        if (busy_array[team.first] == 1 && busy_array[team.second] == 1)
        {
            busy_array[team.first] = 0;
            busy_array[team.second] = 0;
            against[team.first] = team.second;
            siginfo_t sig;
            waitid(P_PID, manager_array[team.first], &sig, WSTOPPED);
            kill(manager_array[team.first], SIGCONT);
        }
        else
        {
            enqueue(q, &team);
        }
    }

    while (!(busy_array[team.first] == 1 && busy_array[team.second] == 1))
        ;

    for (size_t i = 0; i < size; i++)
    {
        kill(manager_array[i], SIGTERM);
    }

    table **score_sheet = (table **)calloc(size, sizeof(table *));

    for (size_t i = 0; i < size; i++)
    {
        score_sheet[i] = (table *)malloc(sizeof(table));
        score_sheet[i]->mine_index = i;
        score_sheet[i]->goals_conceded = 0;
        score_sheet[i]->goals_scored = 0;
        score_sheet[i]->lost = 0;
        score_sheet[i]->score = 0;
        score_sheet[i]->tie = 0;
        score_sheet[i]->won = 0;
    }


    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size - 1; j++)
        {
            int team = result[i][j].team;
            score_sheet[i]->goals_scored += result[i][j].mine;
            score_sheet[i]->goals_conceded += result[i][j].against;
            score_sheet[team]->goals_scored += result[i][j].against;
            score_sheet[team]->goals_conceded += result[i][j].mine;
            if (result[i][j].mine > result[i][j].against)
            {
                score_sheet[i]->won += 1;
                score_sheet[team]->lost += 1;
                score_sheet[i]->score += 3;
            }
            else if (result[i][j].mine < result[i][j].against)
            {
                score_sheet[team]->won += 1;
                score_sheet[i]->lost += 1;
                score_sheet[team]->score += 3;
            }
            else
            {
                score_sheet[i]->tie += 1;
                score_sheet[team]->tie += 1;
                score_sheet[i]->score += 1;
                score_sheet[team]->score += 1;
            }
        }
    }

    for (size_t i = 0; i < size; i++)
    {
        table *starter = score_sheet[i];

        for (size_t j = i; j < size; j++)
        {
            if (starter->score < score_sheet[j]->score)
            {
                table *temp = score_sheet[j];
                score_sheet[j] = starter;
                starter = temp;
            }
            else if (starter->score == score_sheet[j]->score)
            {
                if (starter->goals_scored < score_sheet[j]->goals_scored)
                {
                    table *temp = score_sheet[j];
                    score_sheet[j] = starter;
                    starter = temp;
                }

                else if (starter->goals_scored == score_sheet[j]->goals_scored)
                {
                    if (starter->mine_index > score_sheet[j]->mine_index)
                    {
                        table *temp = score_sheet[j];
                        score_sheet[j] = starter;
                        starter = temp;
                    }
                }
            }
        }

        score_sheet[i]=starter;
    }

    char *topRow[] = {"Team", "W", "D", "L", "GS", "GC", "Points"};

    printf("\n\n");

    printf("%*s%*s%*s%*s%*s%*s%*s\n", -TAB, topRow[0], -TAB, topRow[1], -TAB, topRow[2], -TAB, topRow[3], -TAB, topRow[4], -TAB, topRow[5], -TAB, topRow[6]);
    printf("----------------------------------------------------------------------\n");
    for (int i = 0; i < size; i++)
    {
        printf("%*d%*d%*d%*d%*d%*d%*d",
               -TAB, score_sheet[i]->mine_index + 1,
               -TAB, score_sheet[i]->won,
               -TAB, score_sheet[i]->tie,
               -TAB, score_sheet[i]->lost,
               -TAB, score_sheet[i]->goals_scored,
               -TAB, score_sheet[i]->goals_conceded,
               -TAB, score_sheet[i]->score);
        printf("\n");
    }


    for (size_t i = 0; i < size; i++)
    {
        free(score_sheet[i]);
    }

    free(score_sheet);
    
    printf("\n\n");

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
}