// judge.c
// IOS - Projekt 2
// Date: 21. 4. 2020
// Author: David Hurta, FIT
// Compiled: gcc 7.5.0
// Source file for operating module judge.c

#include "judge.h"
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

void judge_wants_to_enter()
{
    sem_wait(sem_serial_number_of_action);
    fprintf(file, "%-15d: %s %-23s: %s\n", *serial_number_of_action, CATEGORIE_JUDGE, "", "wants to enter");
    fflush(file);
    *serial_number_of_action += 1;
    sem_post(sem_serial_number_of_action);
}

void judge_enters()
{
    sem_wait(sem_serial_number_of_action);
    fprintf(file, "%-15d: %s %-23s: %-25s: %-15d: %-15d: %d\n", *serial_number_of_action, CATEGORIE_JUDGE, "", "enters", *imm_entered_and_not_confirmated, *imm_checked_and_not_confirmated, *imm_currently_in_building);
    fflush(file);
    *serial_number_of_action += 1;
    sem_post(sem_serial_number_of_action);
}

void judge_waits()
{
    sem_wait(sem_serial_number_of_action);
    if (*imm_entered_and_not_confirmated != *imm_checked_and_not_confirmated)
    {
        fprintf(file, "%-15d: %s %-23s: %-25s: %-15d: %-15d: %d\n", *serial_number_of_action, CATEGORIE_JUDGE, "", "waits for imm", *imm_entered_and_not_confirmated, *imm_checked_and_not_confirmated, *imm_currently_in_building);
        fflush(file);
        *serial_number_of_action += 1;
    }
    sem_post(sem_serial_number_of_action);
}

void judge_starts_conf()
{
    sem_wait(sem_serial_number_of_action);
    fprintf(file, "%-15d: %s %-23s: %-25s: %-15d: %-15d: %d\n", *serial_number_of_action, CATEGORIE_JUDGE, "", "starts confirmation", *imm_entered_and_not_confirmated, *imm_checked_and_not_confirmated, *imm_currently_in_building);
    fflush(file);
    *serial_number_of_action += 1;
    sem_post(sem_serial_number_of_action);
}

void judge_ends_conf()
{
    sem_wait(sem_serial_number_of_action);
    *counter_of_imm_left_conf = *imm_checked_and_not_confirmated;
    *number_of_imm_confirmated += *imm_checked_and_not_confirmated;

    *imm_entered_and_not_confirmated = 0;
    *imm_checked_and_not_confirmated = 0;

    fprintf(file, "%-15d: %s %-23s: %-25s: %-15d: %-15d: %d\n", *serial_number_of_action, CATEGORIE_JUDGE, "", "ends confirmation", *imm_entered_and_not_confirmated, *imm_checked_and_not_confirmated, *imm_currently_in_building);
    fflush(file);

    if (*counter_of_imm_left_conf != 0)
    {
        sem_wait(sem_judge_conf_sec_lock);

        sem_post(sem_judge_confirmation);
    }

    *serial_number_of_action += 1;
    sem_post(sem_serial_number_of_action);
}

void judge_leaves()
{
    sem_wait(sem_serial_number_of_action);
    fprintf(file, "%-15d: %s %-23s: %-25s: %-15d: %-15d: %d\n", *serial_number_of_action, CATEGORIE_JUDGE, "", "leaves", *imm_entered_and_not_confirmated, *imm_checked_and_not_confirmated, *imm_currently_in_building);
    fflush(file);
    *serial_number_of_action += 1;
    sem_post(sem_serial_number_of_action);
}

void judge_finishes()
{
    sem_wait(sem_serial_number_of_action);
    fprintf(file, "%-15d: %s %-23s: %s\n", *serial_number_of_action, CATEGORIE_JUDGE, "", "finishes");
    fflush(file);
    *serial_number_of_action += 1;
    sem_post(sem_serial_number_of_action);
}

void process_judge(int judge_enter_max_delay, int judge_confirmation_max_delay, int number_of_imm)
{
    int min_delay = 0;
    int random_delay = 0;

    while (*number_of_imm_confirmated != number_of_imm)
    {
        /************************************JUDGE STARTS************************************/
        random_delay = rand() % (judge_enter_max_delay + 1 - min_delay) + min_delay; // random integer in range <minDelay, judge_enter_max_delay>
        usleep(random_delay * 1000);

        /*********************************JUDGE WANTS TO ENTER********************************/
        judge_wants_to_enter();

        /*******************************JUDGE ENTERS BUILDING*********************************/
        sem_wait(sem_juge_in_building);
        judge_enters();

        /*******************************JUDGE WAITS (IF NEEDED)*******************************/
        judge_waits();

        // if judge is alone in the buidling skip
        if (*imm_entered_and_not_confirmated != 0)
        {
            sem_wait(sem_can_judge_start_confirmation);
            sem_post(sem_can_judge_start_confirmation);
        }

        /******************************JUDGE STARTS CONFIRMATION******************************/
        judge_starts_conf();

        random_delay = rand() % (judge_confirmation_max_delay + 1 - min_delay) + min_delay; // random integer in range <minDelay, judge_confirmation_max_delay>
        usleep(random_delay * 1000);

        /*******************************JUDGE ENDS CONFIRMATION*******************************/
        judge_ends_conf();

        random_delay = rand() % (judge_confirmation_max_delay + 1 - min_delay) + min_delay; // random integer in range <minDelay, judge_confirmation_max_delay>
        usleep(random_delay * 1000);

        // Synchonizing processes
        // Only waits for imms to leave the "confirmation room"
        sem_wait(sem_imm_left_conf_room);
        sem_post(sem_imm_left_conf_room);

        /************************************JUDGE LEAVES************************************/
        judge_leaves();
        sem_post(sem_juge_in_building);
    }

    /**********************************JUDGE FINISHES**********************************/
    judge_finishes();
    exit(0);
}