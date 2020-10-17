// proj2.c
// IOS - Projekt 2
// Date: 24. 4. 2020
// Author: David Hurta, FIT
// Compiled: gcc 7.5.0
// Main source file
//
// Details: This file also contains definitions for functions processing arguments, variables,... 

#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <unistd.h>
#include <math.h>
#include "imm.h"
#include "judge.h"

#define NUMBER_OF_ARGS 5

sem_t *sem_serial_number_of_action;         // sem for accessing important global shared variables and for actions
sem_t *sem_juge_in_building;                // judge is currently in the building
sem_t *sem_can_judge_start_confirmation;    // judge have to wait for all imms to check in
sem_t *sem_judge_conf_sec_lock;             // sem for synchonizing (acts as entrance doors to the confirmation room)
sem_t *sem_judge_confirmation;              // judge ended confirmation, imms can get certificate (acts as exit doors from the confirmation room)
sem_t *sem_imm_left_conf_room;              // sem for synchonizing (all imms have left confirmation room)

int *serial_number_of_action;               // number of actions (enter, checks, ...)
int *number_of_imm_confirmated;             // number of overall imms confirmated 
int *imm_entered_and_not_confirmated;       // number of imms, who had entered building, but werent confirmated
int *imm_checked_and_not_confirmated;       // number of imms, who had checked in in the building, but werent confirmated
int *imm_currently_in_building;             // number of imms currently in the building
int *first_imm_in_building;                 // true/false (1/0), first imm after entering building toogles the value
int *is_judge_waitinig;                     // true/false If its value is higher than zero, judge needs to wait
int *counter_of_imm_left_conf;              // counter of imms just confirmated, who haven't left "confirmation room" (helps synchonizing the whole program)

FILE *file;

// Function initializes semaphores, semaphores must not already exist
// If successful, function returns 0
// If unsuccessful, function returns 1
int init_semaphores()
{
    if ((sem_serial_number_of_action = sem_open("/xhurta04.sem_serial_number_of_action.semafor", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED)
    {
        return 1;
    }
    if ((sem_juge_in_building = sem_open("/xhurta04.sem_juge_in_building.semafor", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED)
    {
        sem_close(sem_serial_number_of_action);
        sem_unlink("/xhurta04.sem_serial_number_of_action.semafor");
        return 1;
    }
    if ((sem_judge_confirmation = sem_open("/xhurta04.sem_judge_confirmation.semafor", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED)
    {
        sem_close(sem_serial_number_of_action);
        sem_close(sem_juge_in_building);
        sem_unlink("/xhurta04.sem_serial_number_of_action.semafor");
        sem_unlink("/xhurta04.sem_juge_in_building.semafor");
        return 1;
    }
    if ((sem_judge_conf_sec_lock = sem_open("/xhurta04.sem_judge_conf_sec_lock.semafor", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED)
    {
        sem_close(sem_serial_number_of_action);
        sem_close(sem_juge_in_building);
        sem_close(sem_judge_confirmation);
        sem_unlink("/xhurta04.sem_serial_number_of_action.semafor");
        sem_unlink("/xhurta04.sem_juge_in_building.semafor");
        sem_unlink("/xhurta04.sem_judge_confirmation.semafor");
        return 1;
    }
    if ((sem_can_judge_start_confirmation = sem_open("/xhurta04.sem_can_judge_start_confirmation.semafor", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED)
    {
        sem_close(sem_serial_number_of_action);
        sem_close(sem_juge_in_building);
        sem_close(sem_judge_confirmation);
        sem_close(sem_judge_conf_sec_lock);
        sem_unlink("/xhurta04.sem_serial_number_of_action.semafor");
        sem_unlink("/xhurta04.sem_juge_in_building.semafor");
        sem_unlink("/xhurta04.sem_judge_confirmation.semafor");
        sem_unlink("/xhurta04.sem_judge_conf_sec_lock.semafor");
        return 1;
    }
    if ((sem_imm_left_conf_room = sem_open("/xhurta04.sem_imm_left_conf_room.semafor", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED)
    {
        sem_close(sem_serial_number_of_action);
        sem_close(sem_juge_in_building);
        sem_close(sem_judge_confirmation);
        sem_close(sem_judge_conf_sec_lock);
        sem_close(sem_can_judge_start_confirmation);
        sem_unlink("/xhurta04.sem_serial_number_of_action.semafor");
        sem_unlink("/xhurta04.sem_juge_in_building.semafor");
        sem_unlink("/xhurta04.sem_judge_confirmation.semafor");
        sem_unlink("/xhurta04.sem_judge_conf_sec_lock.semafor");
        sem_unlink("/xhurta04.sem_can_judge_start_confirmation.semafor");
        return 1;
    }
    return 0;
}

// Function creates shared variables and initializes them
// Function doesn't return value
void init_global_variables()
{
    serial_number_of_action = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    number_of_imm_confirmated = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    imm_currently_in_building = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    imm_entered_and_not_confirmated = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    imm_checked_and_not_confirmated = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    first_imm_in_building = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    is_judge_waitinig = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    counter_of_imm_left_conf = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    *serial_number_of_action = 1;
    *number_of_imm_confirmated = 0;
    *imm_currently_in_building = 0;
    *imm_entered_and_not_confirmated = 0;
    *imm_checked_and_not_confirmated = 0;
    *counter_of_imm_left_conf = 0;
    *is_judge_waitinig = 0;
    *first_imm_in_building = 0;

    sem_wait(sem_judge_confirmation);
}

// Function closes semaphores and file stream
// Function doesn't return value
void free_variables()
{
    sem_close(sem_serial_number_of_action);
    sem_close(sem_juge_in_building);
    sem_close(sem_judge_confirmation);
    sem_close(sem_judge_conf_sec_lock);
    sem_close(sem_can_judge_start_confirmation);
    sem_close(sem_imm_left_conf_room);

    sem_unlink("/xhurta04.sem_serial_number_of_action.semafor");
    sem_unlink("/xhurta04.sem_juge_in_building.semafor");
    sem_unlink("/xhurta04.sem_judge_confirmation.semafor");
    sem_unlink("/xhurta04.sem_judge_conf_sec_lock.semafor");
    sem_unlink("/xhurta04.sem_can_judge_start_confirmation.semafor");
    sem_unlink("/xhurta04.sem_imm_left_conf_room.semafor");

    munmap(serial_number_of_action, sizeof(serial_number_of_action));
    munmap(number_of_imm_confirmated, sizeof(number_of_imm_confirmated));
    munmap(imm_currently_in_building, sizeof(imm_currently_in_building));
    munmap(imm_entered_and_not_confirmated, sizeof(imm_entered_and_not_confirmated));
    munmap(imm_checked_and_not_confirmated, sizeof(imm_checked_and_not_confirmated));
    munmap(first_imm_in_building, sizeof(first_imm_in_building));
    munmap(is_judge_waitinig, sizeof(is_judge_waitinig));
    munmap(counter_of_imm_left_conf, sizeof(counter_of_imm_left_conf));
    munmap(file, sizeof(file));
    fclose(file);
}

int main(int argc, char *argv[])
{
    /*******************************ANALYSIS OF ARGUMENTS*******************************/
    // Checking arguments count
    if (argc != NUMBER_OF_ARGS + 1)
    {
        fprintf(stderr, "Error: Wrong number of argumetns\n");
        return 1;
    }

    // Checking if arguments are number
    for (int i = 0; i < NUMBER_OF_ARGS; i++)
    {
        char *str = argv[i];
        double number = strtod(argv[i + 1], &str);

        if (strcmp(str, "") || fmod(number, 1) != 0)
        {
            fprintf(stderr, "Error: Wrong format of argumetns\n");
            return 1;
        }
    }

    int number_of_imm = strtol(argv[1], NULL, 10);
    int imm_new_max_delay = strtol(argv[2], NULL, 10);
    int judge_enter_max_delay = strtol(argv[3], NULL, 10);
    int imm_certificate_max_delay = strtol(argv[4], NULL, 10);
    int judge_confirmation_max_delay = strtol(argv[5], NULL, 10);

    // Checking if arguments are in range
    if (number_of_imm <= 0 
    || imm_new_max_delay < 0 || imm_new_max_delay > 2000 
    || judge_enter_max_delay < 0 || judge_enter_max_delay > 2000 
    || imm_certificate_max_delay < 0 || imm_certificate_max_delay > 2000 
    || judge_confirmation_max_delay < 0 || judge_confirmation_max_delay > 2000)
    {
        fprintf(stderr, "Error: Invalid range of argumetns value\n");
        return 1;
    }

    /*******************************INITIALIZING FILE STREAM*******************************/
    file = mmap(NULL, sizeof(FILE*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    file = fopen("proj2.out", "w");
    if (file == NULL)
    {
        fprintf(stderr, "Error: Couldn't create file proj2.out\n");
        return 1;
    }

    /*******************************INITIALIZING VARIABLES*******************************/
    if (init_semaphores() == 1)
    {
        fprintf(stderr, "Error: Semaphore couldn't be created\n");
        fclose(file);
        return 1;
    }

    init_global_variables();

    /***********************************MAIN PROGRAM*************************************/
    int pid_imm_generator = 0;
    int pid_judge = 0;

    pid_judge = fork();
    if (pid_judge == 0)
    {
        // Child's process of first fork()
        process_judge(judge_enter_max_delay, judge_confirmation_max_delay, number_of_imm);
    }
    else if (pid_judge > 0)
    {
        // Parent's process of first fork()
        pid_imm_generator = fork();
        if (pid_imm_generator == 0)
        {
            // Child's process of second fork()
            generate_immigrants(number_of_imm, imm_new_max_delay, imm_certificate_max_delay);
        }
        else if (pid_imm_generator < 0)
        {
            // Second fork() failed
            fprintf(stderr, "Error: fork() has failed\n");
            kill(pid_judge, SIGKILL);
            free_variables();
            return 1;
        }
    }
    else
    {
        // First fork() failed
        fprintf(stderr, "Error: fork() has failed\n");
        free_variables();
        return 1;
    }

    // Main process waits for all children to finish and checks exit status of imm_generator
    int status = 0;
    waitpid(pid_imm_generator, &status, 0);
    if (WEXITSTATUS(status) == 1)
    {
        free_variables();
        fprintf(stderr, "Error: fork() has failed\n");
        return 1;
    }
    
    waitpid(pid_judge, NULL, 0);

    free_variables();
    return 0;
}