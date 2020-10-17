// judge.h
// IOS - Projekt 2
// Date: 21. 4. 2020
// Author: David Hurta, FIT
// Compiled: gcc 7.5.0
// Header file for operating module judge.c

#ifndef __JUDGE_H__
#define __JUDGE_H__

#include <stdio.h>
#include <semaphore.h>

#define CATEGORIE_JUDGE "JUDGE"

extern sem_t *sem_serial_number_of_action;
extern sem_t *sem_judge_confirmation;
extern sem_t *sem_juge_in_building;
extern sem_t *sem_judge_conf_sec_lock;
extern sem_t *sem_can_judge_start_confirmation;
extern sem_t *sem_imm_left_conf_room;

extern int *serial_number_of_action;
extern int *number_of_imm_confirmated;
extern int *imm_entered_and_not_confirmated;
extern int *imm_checked_and_not_confirmated;
extern int *imm_currently_in_building;
extern int *first_imm_in_building;
extern int *is_judge_waitinig;
extern int *counter_of_imm_left_conf;

extern FILE *file;

// Judge wants to enter building
// Prints its action
// Increments serial_number_of_action
void judge_wants_to_enter();

// Judge is entering building
// Prints its action
// Increments serial_number_of_action
void judge_enters();

// Judge checks if he should wait
// Prints its action if at least one imm needs to check
// Increments serial_number_of_action if at least one imm needs to check
void judge_waits();

// Judge starts confirmation
// Prints its action
// Increments serial_number_of_action
void judge_starts_conf();

// Judge ends confirmation
// Prints its action
// Opens sem_judge_confirmation, locks sem_judge_conf_sec_lock (only if there were imms to confirm)
// Increments serial_number_of_action
// Increments number_of_imm_confirmated of the total of currently imm_checked_and_not_confirmated
// Sets imm_entered_and_not_confirmated to zero
// Sets imm_checked_and_not_confirmated to zero
void judge_ends_conf();

// Judge leaves
// Prints its action
// Increments serial_number_of_action
void judge_leaves();

// Judge finishes
// Prints its action
// Increments serial_number_of_action
void judge_finishes();

// Main process of judge
void process_judge(int judge_enter_max_delay, int judge_confirmation_max_delay, int number_of_imm);

#endif // __JUDGE_H__