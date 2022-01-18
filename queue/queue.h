/* File:     queue.h
 * Purpose:  Header file for queue.c which implements a queue of tasks for the workers
 */
#ifndef _QUEUE_H_
#define _QUEUE_H_

Queue* Allocate_queue(void);
void Free_queue(Queue* q_p);
void Enqueue(Queue* q_p, Task task);
int Dequeue(Queue* q_p, Task* task);

#endif