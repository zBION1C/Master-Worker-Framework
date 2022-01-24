#include "worker.h"

Task* create_task(void* function, void** args, int n_args);

/*Funzione di "idle" per i worker, qui i worker aspettano le task*/
void* wait_work (void* worker){
	Worker* w = (Worker*) worker;
	Queue* q = w->task_q;
	Task* task = malloc(sizeof(task));

	int q_size = q->enqueued - q->dequeued;
	int state = w->state;

	while (state != Killed || q_size > 0){
		if (q_size >= 1){
			pthread_mutex_lock(&w->lock);
			if (w->state != Killed)
				w->state = Running;
			pthread_mutex_unlock(&w->lock);

			pthread_mutex_lock(&w->task_q->lock);
			Dequeue(w->task_q, task);
			pthread_mutex_unlock(&w->task_q->lock);				

			void* (*fun)(void**) = task->function;
			void** args = task->args;

			void* result;

			result = fun(args);

			/* Se dopo l'esecuzione della sottotask il flag di errore del worker e' 1, si riaccoda la task nella coda per una futura esecuzione da parte del worker */
			if (w->error){
				printf("Error Worker %d, the task is re-scheduled for future computation.\n", w->id);
				pthread_mutex_lock(&w->task_q->lock);
				Enqueue(w->task_q, *(task));
				pthread_mutex_unlock(&w->task_q->lock);	
				w->error = 0;
			}

			send_to_master(result);
		}

		pthread_mutex_lock(&w->lock);
		if (w->state != Killed)
			w->state = Waiting;
		pthread_mutex_unlock(&w->lock);

		q_size = q->enqueued - q->dequeued;
		state = w->state;

	}
	
	free(task);
	return NULL;
}

/*Funzione che permette ai workers di mandare i risultati calcolati al master*/
void send_to_master(void* value){
	Task* result;
	result = create_task(NULL, value, 1);

	pthread_mutex_lock(&master->result_q->lock);
	Enqueue(master->result_q, *result);
	pthread_mutex_unlock(&master->result_q->lock);
}