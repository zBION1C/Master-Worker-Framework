#include "mw_fw.h"

/*Funzione di inizializzazione framework*/
void mw_init(void){
	// Iniziallizzazione master
	master = malloc(sizeof(Master));
	master->state = Waiting;
	master->task_q = Allocate_queue();
	master->result_q = Allocate_queue();
	pthread_mutex_init(&(master->lock), NULL);

	// Allocazione coda client
	client_q = Allocate_queue();

	master_handle = malloc(sizeof(pthread_t));
	pthread_create(&master_handle[0], NULL, wait_master, (void*) master);
}

void mw_terminate(void)
{
	// Uccisione master
	pthread_mutex_lock(&master->lock);
	master->state = Killed;
	pthread_mutex_unlock(&master->lock);

	pthread_join(master_handle[0], 0);
	
	// Deallocazione code master e client
	Free_queue(master->task_q);
	Free_queue(master->result_q);

	free(master);
	free(master_handle);
}

/*Funzione per allocazione task al master*/
int allocate_master(Task* task, int n_workers)
{
	// si popola il campo n_workers per la task, ovvero il numero di worker specificato dall'utente
	task->n_workers = n_workers;

	// si accoda la task nella coda del master
	pthread_mutex_lock(&master->task_q->lock);
	Enqueue(master->task_q, *task);
	pthread_mutex_unlock(&master->task_q->lock);

	return 0;
}

/*Funzione creazione task
 * Input: 	-Funzione da eseguire
 * 			-Argomenti funzione
 *			-Numero di argomenti
 * Output:	Puntatore alla nuova task creata
*/
Task* create_task(void* function, void** args, int n_args)
{
	Task* task = malloc(sizeof(Task));

	task->function = function;
	task->args = args;
	task->args_size = n_args;
	task->n_workers = 0;

	return task;
}

/*Funzione per la raccolta dei risultati da parte del client*/
void gather(void* result)
{
	Task* res = malloc(sizeof(Task));
	int q_size = client_q->enqueued - client_q->dequeued; 

	// Se la coda del client e' vuota, aspetta e monitora la sua dimensione...
	while (q_size <= 0){
		q_size = client_q->enqueued - client_q->dequeued;
	}

	// Appena c'e un elemento raccoglilo e ritorna
	pthread_mutex_lock(&client_q->lock);
	Dequeue(client_q, res);
	pthread_mutex_unlock(&client_q->lock);
	strcpy(result, (char*) res->args);
}

void change_alloc_mode(int mode)
{
	alloc_mode = mode;
}
