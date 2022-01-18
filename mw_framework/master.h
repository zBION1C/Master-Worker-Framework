void* wait_master (void* master);
void allocate_task(Worker* handle[], Task* task, int mode);
void create_workers(Worker* handle[], int n);
void gather_results();
void wait_workers(Worker* handle[], int n_workers);
void send_to_client(void* result);
void destroy_workers(Worker* handle[], int n_workers);
