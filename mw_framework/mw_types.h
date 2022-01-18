typedef enum {Waiting, Running, Killed, Done} State;

typedef struct task_s {
	void* (*function)(void**);
	void** args;
	int args_size;
	int n_workers;
} Task;

struct queue_node_s {
   Task task;
   struct queue_node_s* next_p;
};

struct queue_s{
   pthread_mutex_t lock;
   int enqueued;
   int dequeued;
   struct queue_node_s* front_p;
   struct queue_node_s* tail_p;
};

typedef struct queue_node_s Queue_node;
typedef struct queue_s Queue;

typedef struct master_s
{
	pthread_mutex_t lock;
	State state;
	Queue* task_q;
	Queue* result_q;
} Master;

typedef struct worker_s
{
	pthread_mutex_t lock;
	int id;
	State state;
	Queue* task_q;
} Worker;