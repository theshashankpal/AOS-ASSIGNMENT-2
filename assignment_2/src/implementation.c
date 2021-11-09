#include "project.h"

void scheduling(int *manager_array, fixture *team)
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

void tableCreation(SS (*result)[size - 1])
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

void sort()
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

void printingTable()
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