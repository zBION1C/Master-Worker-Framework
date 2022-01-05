/* File:     master_worker.h
 * Purpose:  Header file for master_worker.c which implements the master_worker framework fuctions
 */
#ifndef _MW_H_
#define _MW_H_

#include <pthread.h>

struct queue_s;

struct task_s {
    void* (*function)(void**);
    void** args;
};

struct worker_s {
    int id;
    int state;
    struct queue_s* queue;
};

enum Mode {Static, Dynamic};

int MW_init();
void Create_workers(struct worker_s* handle[], int n_workers);
int Destroy_worker(struct worker_s* w);
struct task_s* Create_task(void* function, void** args);
int Allocate_task(struct worker_s* w, struct task_s* task);
int Send_to_master(void* value);
void** Gather_from_workers();
/* Allocate_pool_task va fatta in questo modo:
 *      Allocate__pool_task(struct worker_s* handle[], struct task_s* task, long workload, int mode)
 */

#endif