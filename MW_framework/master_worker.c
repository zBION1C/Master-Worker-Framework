/* File:    master_worker.c
 *
 * Purpose: Implements the essential function for the master/worker framework
 *
 * Compile: gcc -g -Wall -o esempio queue.c queue.h master_worker.h esempio.c
 */


/*TODO:
 *      
 */

#include <stdlib.h>
#include "master_worker.h"
#include "../queue/queue.c"

static struct queue_s* master_q;
static pthread_t* threads_handle = NULL;

int MW_init()
{
    master_q = Allocate_queue();
    return 0;
}

int Send_to_master(void* value)
{
    struct task_s* ret = Create_task(NULL, value);

    pthread_mutex_lock(&master_q->lock);
    Enqueue(master_q, *ret);
    pthread_mutex_unlock(&master_q->lock);
    
    return 0;
}

void *wait(void *worker)
{
    struct worker_s* w = (struct worker_s*) worker;
    
    /* Extraction of worker fields */
    struct queue_s* q = w->queue;
    int q_size = q->enqueued - q->dequeued;
    int state;

    while(state != -1 || q_size > 0){
        if (q_size >= 1){
            struct task_s* curr_task = malloc(sizeof(struct task_s));
            pthread_mutex_lock(&q->lock);
            Dequeue(q, curr_task);
            pthread_mutex_unlock(&q->lock);
            void** args = curr_task->args;
            void* (*fun)(void**) = curr_task->function;
            void* result = fun(args);

            Send_to_master(result);
        }       
        state = w->state;
        q_size = q->enqueued - q->dequeued;
    }
    return NULL;
}

void** Gather_from_workers()
{
    int q_size = master_q->enqueued - master_q->dequeued;
    void** arr = malloc(sizeof(void*)*q_size);
    int i = 0;
    while(q_size > 0){
        struct task_s* curr_val = malloc(sizeof(struct task_s));
        
        pthread_mutex_lock(&master_q->lock);
        Dequeue(master_q, curr_val);
        pthread_mutex_unlock(&master_q->lock);
        
        arr[i] = curr_val->args;
        q_size = master_q->enqueued - master_q->dequeued;

        i++;
    }
    return arr;
}

void Create_workers(struct worker_s* handle[], int n_workers)
{
    threads_handle = malloc((n_workers)*sizeof(pthread_t));

    for (int i = 0; i<n_workers; i++){
        struct worker_s* worker = malloc(sizeof(struct worker_s));
        struct queue_s* worker_q = Allocate_queue();

        /* Fill the worker attributes */
        worker->id = i;
        worker->state = 0;
        worker->queue = worker_q;

        /* Store worker pointer in the handle provided by the user*/
        handle[i] = worker;

        pthread_create(&threads_handle[i], NULL, wait, (void*) worker);
    }
}

int Destroy_worker(struct worker_s* w)
{
    int id = w->id;
    w->state = -1;
    pthread_join(threads_handle[id], NULL);
    return 0;
}

struct task_s* Create_task(void* function, void** args)
{
    struct task_s* task = malloc(sizeof(struct task_s));
    task->function = function;
    task->args = args;
    return task;
}

int Allocate_task(struct worker_s* w, struct task_s* task)
{
    struct queue_s* q = w->queue;

    pthread_mutex_lock(&q->lock);
    Enqueue(w->queue, *task);
    pthread_mutex_unlock(&q->lock);

    return 0;
}