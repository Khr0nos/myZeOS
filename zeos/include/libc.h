/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#define RR 0
#define FCFS 1

#include <stats.h>

extern int errno;

int write(int fd, char *buffer, int size);

void itoa(int a, char *b);

int strlen(char *a);

int getpid();

int fork();

void exit();

int write(int fd, char* buffer, int size);

int read(int fd, char* buffer, int nbytes);

int set_sched_policy(int policy);

int get_stats(int pid, struct stats *st);

void perror();

#endif  /* __LIBC_H__ */
