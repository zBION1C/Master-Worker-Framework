#include "master.h"
#define STD_DYNAMIC_ALLOC 10000

Task* create_task(void* function, void** args, int n_args);

pthread_t* threads_handle;

/*Funzione di "idle" del master, qui aspetta le task da allocare ai workers*/
void* wait_master(void* master)
{
	int state, q_size, n_workers;

	Master* m = (Master*) master;
	state = m->state;
	
	Queue* q = m->task_q;  
	q_size = q->enqueued - q->dequeued;
	
	Worker** workers_handle;
	
	Task* task = malloc(sizeof(task));

	// while fatto in modo tale che, se il client uccide il master, ma nella coda ci sono ancora task da svolgere, 
	// si completano prima tutte le task fino a svuotare la coda e poi il master viene effettivamente ucciso.
	while(state != Killed || q_size > 0){
		if (q_size >= 1){
			// Se il master non e' stato ucciso, allora sta lavorando
			pthread_mutex_lock(&m->lock);
			if (m->state != Killed)
				m->state = Running;
			pthread_mutex_unlock(&m->lock);


			// prendi la task dall coda...
			pthread_mutex_lock(&m->task_q->lock);
			Dequeue(m->task_q, task);
			pthread_mutex_unlock(&m->task_q->lock);

			n_workers = task->n_workers;
			workers_handle = malloc(sizeof(Worker*)*n_workers);

			// crea i workers in accordo con quanto specificato dal client
			create_workers(workers_handle, n_workers);

			// Alloca la task all'insieme di workers creati...
			allocate_task(workers_handle, task, alloc_mode);

			wait_workers(workers_handle, n_workers);
		}
		pthread_mutex_lock(&m->lock);
		if (m->state != Killed)
			m->state = Waiting;
		pthread_mutex_unlock(&m->lock);

		q_size = q->enqueued - q->dequeued;
		state = m->state;
	}

	destroy_workers(workers_handle, n_workers);

	free(task);
	return NULL;
}

/*Funzione per permettere al master di mandare al client il risultato finale della task*/
void send_to_client(void* value)
{
	Task* result; 
	result = create_task(NULL, value, 1);

	pthread_mutex_lock(&client_q->lock);
	Enqueue(client_q, *result);
	pthread_mutex_unlock(&client_q->lock);
}

/*Funzione nella quale il master aspetta il completamento dei workers per la raccolta dei risultati parziali*/
void wait_workers(Worker* handle[], int n_workers)
{
	for (int i = 0; i < n_workers; i++){
		Worker* worker = handle[i];

		pthread_mutex_lock(&worker->lock);
		worker->state = Killed;
		pthread_mutex_unlock(&worker->lock);

		pthread_join(threads_handle[i], NULL);
	}
	
	gather_results();
}

/*Funzione che permette al master di raccogliere i risultati dei vari workers per la task corrente*/
void gather_results()
{
	Task* result = malloc(sizeof(Task));

	int q_size = master->result_q->enqueued - master->result_q->dequeued;
	void** results = malloc(sizeof(void*)*q_size);

	for (int i = 0; i < q_size; i++){
		pthread_mutex_lock(&master->result_q->lock);
		Dequeue(master->result_q, result);
		pthread_mutex_unlock(&master->result_q->lock);

		results[i] = result->args;
	}

	free(result);
	void* final;

	for (int i = 0; i < q_size; i++){
        if (results[i] != NULL){
            final = (void*) results[i];
        }
    }
    send_to_client(final);

    free(results);
}

/*Funzione per la creazione dei workers, i puntatori ad essi per un riferimento futuro messi nell' array handle*/
void create_workers(Worker* handle[], int n)
{
	threads_handle = malloc(sizeof(pthread_t)*n);

	for (int i = 0; i < n; i++){
		Worker* worker = malloc(sizeof(Worker));
		worker->id = i;
		worker->state = 0;
		worker->task_q = Allocate_queue();
		pthread_mutex_init(&(worker->lock), NULL);

		handle[i] = worker;

		pthread_create(&threads_handle[i], NULL, wait_work, (void*) worker);
	}
}

void destroy_workers(Worker* handle[], int n_workers)
{
	for (int i = 0; i < n_workers; i++){
		Worker* w = handle[i];
		Free_queue(w->task_q);
		free(handle[i]);
	}
	free(threads_handle);
	free(handle);
}

/* Funzione per la allocazione di task ad un insieme di workers, l'allocazione attualmente avviene in modo "statico"
 * Si assume che il lavoro da fare e' specificato nei primi due argomenti della funzione specificata dalla task *SEMPRE*
*/
void allocate_task(Worker* handle[], Task* task, int mode)
{
	int n_args, n_workers;
	long workload, local_workload, remainder, min = 0, max;
	// Funzione, array di argomenti, numero di argomenti e numero di workers specificati dalla task
	void* (*function)(void**) = task->function;
	void** args = task->args;
	n_args = task->args_size;
	n_workers = task->n_workers;
	
	// Calcolo del workload totale
	workload = args[1] - args[0];
	
	if (mode == STATIC) { // Se modalita scelta statica

		// Suddivisione statica del workload tra i vari workers
		local_workload = workload/n_workers;
		remainder = workload%n_workers;

		for (int i = 0; i < n_workers; i++){
			Worker* w = handle[i];

			min = local_workload * i;
			max = min + local_workload;

			// Se la divisione ha un resto diverso da 0, a tutti i workers con id <= remainder viene assegnata un iterazione in piu
			if (i < remainder)
				max += 1;
			
			void** sub_args = malloc(sizeof(void*)*n_args);

			// Oltre ai primi due argomenti, i restanti argomenti rimangono inviariati
			sub_args[0] = (void*) min;
			sub_args[1] = (void*) max;
			for (int k = 2; k < n_args; k++){
				sub_args[k] = args[k];
			}

			Task* sub_task = create_task(function, sub_args, n_args);

			pthread_mutex_lock(&w->task_q->lock);
			Enqueue(w->task_q, *sub_task);
			pthread_mutex_unlock(&w->task_q->lock);
		}
	} else if (mode == DYNAMIC) {

		while (workload > 0) {
			for (int i = 0; i < n_workers; i++){
				Worker* w = handle[i];
				
				if (workload >= STD_DYNAMIC_ALLOC){
					workload -= STD_DYNAMIC_ALLOC;
					max = min + STD_DYNAMIC_ALLOC;	
				} else {
					max = min + workload ;
					workload = 0;
				}

				void** sub_args = malloc(sizeof(void*)*n_args);
				
				sub_args[0] = (void*) min;
				sub_args[1] = (void*) max;

				for (int k = 2; k < n_args; k++){
					sub_args[k] = args[k];
				}
				
				Task* sub_task = create_task(function, sub_args, n_args);

				pthread_mutex_lock(&w->task_q->lock);
				Enqueue(w->task_q, *sub_task);
				pthread_mutex_unlock(&w->task_q->lock);

				min += STD_DYNAMIC_ALLOC;
			}
		}
	}
}