/* File:     queue.h
 * Purpose:  Header file for queue.c which implements a queue of tasks for the workers
 */
#ifndef _QUEUE_H_
#define _QUEUE_H_

struct task_s;

struct queue_node_s {
   struct task_s task;
   struct queue_node_s* next_p;
};

struct queue_s{
   pthread_mutex_t lock;
   int enqueued;
   int dequeued;
   struct queue_node_s* front_p;
   struct queue_node_s* tail_p;
};

struct queue_s* Allocate_queue(void);
void Free_queue(struct queue_s* q_p);
void Print_queue(struct queue_s* q_p);
void Enqueue(struct queue_s* q_p, struct task_s task);
int Dequeue(struct queue_s* q_p, struct task_s* task);
int Search(struct queue_s* q_p, int mesg, int* src_p);

#endif