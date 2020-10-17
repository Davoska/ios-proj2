// imm.h
// IOS - Projekt 2
// Date: 21. 4. 2020
// Author: David Hurta, FIT
// Compiled: gcc 7.5.0
// Header file for operating module imm.c

#ifndef __IMM_H__
#define __IMM_H__

#include <stdio.h>
#include <semaphore.h>

#define CATEGORIE_IMM "IMM"

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

// Imm starts
// Prints its action
// Increments serial_number_of_action
void imm_starts(int id_of_process);

// Imm enters
// Prints its action
// Increments serial_number_of_action
// Increments imm_currently_in_building
// Increments imm_entered_and_not_confirmated
void imm_enters(int id_of_process);

// Imm checks
// Prints its action
// Increments serial_number_of_action
// Increments imm_checked_and_not_confirmated
void imm_checks(int id_of_process);

// Imm wants certification
// Prints its action
// Increments serial_number_of_action
void imm_wants_cert(int id_of_process);

// Imm gets certification
// Prints its action
// Increments serial_number_of_action
void imm_got_cert(int id_of_process);

// Imm leaves building
// Prints its action
// Increments serial_number_of_action
// Decrements imm_currently_in_building
void imm_leaves(int id_of_process);

// Main process of immigrant
// Returns 0 if successful
void process_immigrant(int id_of_process, int imm_certificate_max_delay);

// Process for generating processes of immigrants
// Returns 0 if successful
void generate_immigrants(int number_of_imm, int max_delay, int imm_certificate_max_delay);

// Terminates processes in the array arr_of_pid up until the end_index (excluding)
void terminate_processes(int* arr_of_pid, int end_index);

#endif // __IMM_H__