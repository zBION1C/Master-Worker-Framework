#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>
#include "mw_types.h"
#include "../queue/queue.c"

#define DEFAULT 0
#define STATIC 1

Queue* client_q;
Master* master;
pthread_t* master_handle;
int alloc_mode = DEFAULT;
sem_t client_sem;

#include "worker.c"
#include "master.c"

void mw_init(void);
void mw_terminate(void);
void gather(void* result);
int allocate_master(Task* task, int n_workers);
void change_alloc_mode(int mode);
Task* create_task(void* function, void** args, int n_args);
