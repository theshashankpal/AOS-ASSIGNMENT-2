#include "project.h"
#include "queue.h"

int shm_fd;
void *ptr;
int size;
int *against;
int *busy_array;

// ds to store in a queue that is used for scheduling purposes.
typedef struct _fixture
{
    int first;
    int second;
} fixture;

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

void sort(table **);

void tableCreation(table **, SS (*)[]);

void scheduling(queue *, int *, fixture *);

void printingTable(table **);

#define TAB -10

int main(int argc, char *argv[])
{
    printf("\n");
    shm_unlink(SHARED_MEMORY_NAME); // just a precaution.

    FILE *fp;
    char *line = NULL;
    fp = fopen("src/fixtures.txt", "r"); // opening file in read mode.
    size_t len = 0;
    ssize_t read;
    if (fp == NULL)
    {
        perror("File : ");
        exit(EXIT_FAILURE);
    }

    queue *q = createQueue(sizeof(fixture));

    read = getline(&line, &len, fp);
    size = atoi(line);

    printf("No. of teams : %d\n", size);

    // Reading fixtures from file and storing them in the queue.
    while ((read = getline(&line, &len, fp)) != -1)
    {
        char *temp_line = line;
        int count = 0;
        char temp[256];
        while ((*temp_line) != ' ')
        {
            temp[count++] = *temp_line;
            temp_line++;
        }
        temp[count] = '\0';
        int first = atoi(temp);

        temp_line++; // crossed ' '

        char temp1[256];
        count = 0;
        while ((*temp_line) != '\n')
        {
            temp1[count++] = *temp_line;
            temp_line++;
        }

        temp1[count] = '\0';
        int second = atoi(temp1);

        fixture team;
        team.first = first - 1;
        team.second = second - 1;
        enqueue(q, &team);
    }

    printf("Fixtures are read from file, now gonna start scheduling them : \n");

    // Closing the file descriptor and freeing up the memory used to read lines from a file.
    fclose(fp);
    if (line)
        free(line);

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

    printf("\n\n");

    // scheduling matches
    fixture team;
    scheduling(q, manager_array, &team);

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

    table **score_sheet = (table **)calloc(size, sizeof(table *)); // making a local data structure to capture or make the final score table, from results that are updated by manager processes

    // allocating all ds, doing this way because we also have to sort them in future , so having a pointer to struct in an array works.
    for (size_t i = 0; i < size; i++)
    {
        score_sheet[i] = (table *)calloc(1, sizeof(table));
        score_sheet[i]->mine_index = i;
    }

    // Making final table.
    tableCreation(score_sheet, result);

    // sorting the table by the given constraint. Used selection sort.
    sort(score_sheet);

    // Printing the table.
    printingTable(score_sheet);

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

void scheduling(queue *q, int *manager_array, fixture *team)
{
    while (!isEmpty(q))
    {
        dequeue(q, team);

        if (busy_array[team->first] == 1 && busy_array[team->second] == 1)
        {
            // making those teams busy, so that they can't be scheduled again , until they've finished their work.
            busy_array[team->first] = 0;
            busy_array[team->second] = 0;

            // storing the opponent info , so that manager_process can know who their opponent is , when they're signaled to schedule the match.
            against[team->first] = team->second;

            siginfo_t sig;
            waitid(P_PID, manager_array[team->first], &sig, WSTOPPED); // waiting for manager_process to stop
            kill(manager_array[team->first], SIGCONT);                 // giving it the continue signal
        }
        else
        {
            enqueue(q, team);
        }
    }
}

void tableCreation(table **score_sheet, SS (*result)[size - 1])
{
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
}

void sort(table **score_sheet)
{
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
        score_sheet[i] = starter;
    }
}

void printingTable(table **score_sheet)
{
    char *topRow[] = {"Team", "W", "D", "L", "GS", "GC", "Points"};

    printf("\n\n");

    printf("%*s%*s%*s%*s%*s%*s%*s\n", TAB, topRow[0], TAB, topRow[1], TAB, topRow[2], TAB, topRow[3], TAB, topRow[4], TAB, topRow[5], TAB, topRow[6]);
    printf("-------------------------------------------------------------------\n");
    for (int i = 0; i < size; i++)
    {
        printf("%*d%*d%*d%*d%*d%*d%*d",
               TAB, score_sheet[i]->mine_index + 1,
               TAB, score_sheet[i]->won,
               TAB, score_sheet[i]->tie,
               TAB, score_sheet[i]->lost,
               TAB, score_sheet[i]->goals_scored,
               TAB, score_sheet[i]->goals_conceded,
               TAB, score_sheet[i]->score);
        printf("\n");
    }
}
