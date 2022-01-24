/* File:     queue.c
 * Purpose:  Implement a queue of tasks
 *           using a linked list.  
 *
 * Input:    Operations (first letter of op name) and, when necessary, keys
 * Output:   Prompts for input and results of operations
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

Queue* Allocate_queue() {
   Queue* q_p = malloc(sizeof(Queue));
   q_p->enqueued = q_p->dequeued = 0;
   q_p->front_p = NULL;
   q_p->tail_p = NULL;
   pthread_mutex_init(&(q_p->lock), NULL);   
   return q_p;
}  /* Allocate_queue */

/* Frees nodes in queue:  leaves queue struct allocated */
void Free_queue(Queue* q_p) {
   Queue_node* curr_p = q_p->front_p;
   Queue_node* temp_p;

   while(curr_p != NULL) {
      temp_p = curr_p;
      curr_p = curr_p->next_p;
      free(temp_p);
   }
   q_p->enqueued = q_p->dequeued = 0;
   q_p->front_p = q_p->tail_p = NULL;
}   /* Free_queue */

void Enqueue(Queue* q_p, Task task) {
   Queue_node* n_p = malloc(sizeof(Queue_node));
   n_p->task = task;
   n_p->next_p = NULL;
   if (q_p->tail_p == NULL) { /* Empty Queue */
      q_p->front_p = n_p;
      q_p->tail_p = n_p;
   } else {
      q_p->tail_p->next_p = n_p;
      q_p->tail_p = n_p;
   }
   q_p->enqueued++;
}  /* Enqueue */

int Dequeue(Queue* q_p, Task* task) {
   Queue_node* temp_p;

   if (q_p->front_p == NULL) /* Nothing do dequeue */
      return 0;
   *task = q_p->front_p->task;
   temp_p = q_p->front_p;
   if (q_p->front_p == q_p->tail_p)  /* One node in list */
      q_p->front_p = q_p->tail_p = NULL;
   else
      q_p->front_p = temp_p->next_p;
   free(temp_p);
   q_p->dequeued++;
   return 1;
}  /* Dequeue */
