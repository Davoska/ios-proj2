// imm.c
// IOS - Projekt 2
// Date: 24. 4. 2020
// Author: David Hurta, FIT
// Compiled: gcc 7.5.0
// Source file for operating module imm.c

#include "imm.h"
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

void imm_starts(int id_of_process)
{
    sem_wait(sem_serial_number_of_action);
    fprintf(file, "%-15d: %s %-25d: %s\n", *serial_number_of_action, CATEGORIE_IMM, id_of_process, "starts");
    fflush(file);
    *serial_number_of_action = *serial_number_of_action + 1;
    sem_post(sem_serial_number_of_action);
}

void imm_enters(int id_of_process)
{
    sem_wait(sem_serial_number_of_action);
    *imm_currently_in_building += 1;
    *imm_entered_and_not_confirmated += 1;
    fprintf(file, "%-15d: %s %-25d: %-25s: %-15d: %-15d: %d\n", *serial_number_of_action, CATEGORIE_IMM, id_of_process, "enters", *imm_entered_and_not_confirmated, *imm_checked_and_not_confirmated, *imm_currently_in_building);
    fflush(file);
    *serial_number_of_action += 1;
    sem_post(sem_serial_number_of_action);
}

void imm_checks(int id_of_process)
{
    sem_wait(sem_serial_number_of_action);
    *imm_checked_and_not_confirmated += 1;
    fprintf(file, "%-15d: %s %-25d: %-25s: %-15d: %-15d: %d\n", *serial_number_of_action, CATEGORIE_IMM, id_of_process, "checks", *imm_entered_and_not_confirmated, *imm_checked_and_not_confirmated, *imm_currently_in_building);
    fflush(file);
    *serial_number_of_action += 1;
    sem_post(sem_serial_number_of_action);
}

void imm_wants_cert(int id_of_process)
{
    sem_wait(sem_serial_number_of_action);
    fprintf(file, "%-15d: %s %-25d: %-25s: %-15d: %-15d: %d\n", *serial_number_of_action, CATEGORIE_IMM, id_of_process, "wants certificate", *imm_entered_and_not_confirmated, *imm_checked_and_not_confirmated, *imm_currently_in_building);
    fflush(file);
    *serial_number_of_action += 1;
    sem_post(sem_serial_number_of_action);
}

void imm_got_cert(int id_of_process)
{
    sem_wait(sem_serial_number_of_action);
    fprintf(file, "%-15d: %s %-25d: %-25s: %-15d: %-15d: %d\n", *serial_number_of_action, CATEGORIE_IMM, id_of_process, "got certificate", *imm_entered_and_not_confirmated, *imm_checked_and_not_confirmated, *imm_currently_in_building);
    fflush(file);
    *serial_number_of_action += 1;
    sem_post(sem_serial_number_of_action);
}

void imm_leaves(int id_of_process)
{
    sem_wait(sem_serial_number_of_action);
    *imm_currently_in_building -= 1;
    fprintf(file, "%-15d: %s %-25d: %-25s: %-15d: %-15d: %d\n", *serial_number_of_action, CATEGORIE_IMM, id_of_process, "leaves", *imm_entered_and_not_confirmated, *imm_checked_and_not_confirmated, *imm_currently_in_building);
    fflush(file);
    *serial_number_of_action += 1;
    sem_post(sem_serial_number_of_action);
}

void process_immigrant(int id_of_process, int imm_certificate_max_delay)
{
    int min_delay = 0;

    /*******************************IMM STARTS*******************************/
    imm_starts(id_of_process);

    /*******************************IMM ENTERS*******************************/
    sem_wait(sem_juge_in_building);

    sem_wait(sem_serial_number_of_action);
    // Imms sets semaphore for judge to wait until imm checks in
    if (*imm_checked_and_not_confirmated == *imm_entered_and_not_confirmated && *is_judge_waitinig == 0)
    {
        sem_wait(sem_can_judge_start_confirmation);
    }
    *is_judge_waitinig += 1;
    // First imm sets semaphore for judge to wait until all imms left confirmation room (synchonizing purposes)
    if (*first_imm_in_building == 0)
    {
        sem_wait(sem_imm_left_conf_room);
        *first_imm_in_building = 1;
    }
    sem_post(sem_serial_number_of_action);

    imm_enters(id_of_process);

    sem_post(sem_juge_in_building);

    /*******************************IMM CHECKS*******************************/
    imm_checks(id_of_process);

    /************************WAITING FOR CONFIRMATION************************/
    // Synchonizing processes (acts as entrance doors to the confirmation room)
    sem_wait(sem_judge_conf_sec_lock);
    sem_post(sem_judge_conf_sec_lock);

    // All imms have checked in, judge can start confirmation
    sem_wait(sem_serial_number_of_action);
    *is_judge_waitinig -= 1;
    if (*imm_checked_and_not_confirmated == *imm_entered_and_not_confirmated && *is_judge_waitinig == 0)
    {
        sem_post(sem_can_judge_start_confirmation);
    }
    sem_post(sem_serial_number_of_action);

    // Synchonizing processes (acts as exit doors to the confirmation room, judge opens them when conf. ends)
    sem_wait(sem_judge_confirmation);
    sem_post(sem_judge_confirmation);

    // Synchonizing processes
    // All confirmated imms left confirmation room
    // Exit doors close, entrance doors open
    // New imms (not confirmated) can enter conf. room to wait for the judge
    sem_wait(sem_serial_number_of_action);
    *counter_of_imm_left_conf -= 1;
    if (*counter_of_imm_left_conf == 0)
    {
        sem_wait(sem_judge_confirmation);
        sem_post(sem_judge_conf_sec_lock);

        *first_imm_in_building = 0;
        sem_post(sem_imm_left_conf_room);
    }
    sem_post(sem_serial_number_of_action);

    /*******************************IMM WANTS CERTIFICATE*******************************/
    imm_wants_cert(id_of_process);

    int random_delay = rand() % (imm_certificate_max_delay + 1 - min_delay) + min_delay; // random integer in range <minDelay, imm_certificate_max_delay>
    usleep(random_delay * 1000);

    /*******************************IMM GETS CERTIFICATE*******************************/
    imm_got_cert(id_of_process);

    /************************************IMM LEAVES************************************/
    sem_wait(sem_juge_in_building);
    imm_leaves(id_of_process);
    sem_post(sem_juge_in_building);

    exit(0);
}

void terminate_processes(int* arr_of_pid, int end_index)
{
    for (int i = 0; i < end_index; i++)
    {
        kill(arr_of_pid[i], SIGKILL);
    }
}

void generate_immigrants(int number_of_imm, int max_delay, int imm_certificate_max_delay)
{
    int min_delay = 0;
    int random_delay = 0;
    int pid_immigrant = 0;
    int *arr_of_pid = malloc(number_of_imm * (sizeof(int)));

    for (int i = 0; i < number_of_imm; i++)
    {
        if (max_delay != 0)
        {
            random_delay = rand() % (max_delay + 1 - min_delay) + min_delay;    // random integer in range <minDelay, maxDelay>
            usleep(random_delay * 1000);                                     // miliseconds to microseconds
        }
        
        pid_immigrant = fork();
        arr_of_pid[i] = pid_immigrant;
        if (pid_immigrant == 0)
        {
            // Immigrant's process of fork()
            process_immigrant(i + 1, imm_certificate_max_delay);
        }
        else if (pid_immigrant < 0)
        {
            // fork() failed
            terminate_processes(arr_of_pid, i);
            free(arr_of_pid);
            exit(1);
        }
    }

    // Parent process waits for its children (imms) to finish
    for (int i = 0; i < number_of_imm; i++)
    {
        wait(NULL);
    }

    free(arr_of_pid);
    exit(0);
}