#include <stdio.h>
#include <string.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include "MW_framework/master_worker.c"
#include "timer.h"

#define BUFF 8
#define N_WORKERS 6

void next(char* c);
long perm();
void zeros_padding(char* c);                       

char* bruteforce(void** args)
{
    unsigned char* hash = (char*) args[0];
    long min   = (long) args[1];
    long max   = (long) args[2];

    char *c = malloc(BUFF * sizeof(char));
    int err = 0;
    unsigned char hash2[MD5_DIGEST_LENGTH];

    for (long k = min; k < max; k++){
        sprintf(c, "%ld", k);
        zeros_padding(c);
        //printf("testo il pin: %s\n", c);
        MD5(c, strlen(c), hash2);
        err = 0;
        for (int i=0; i < MD5_DIGEST_LENGTH; i++){
            if(hash[i] != hash2[i]){
                err = 1;
                break;
            }
        }
        if(!err){
            return c;
        }
    }
    return NULL;
}

int main(int argc, char const *argv[])
{

    char* data1 = argv[1];
    size_t length = strlen(data1); 
    unsigned char hash1[MD5_DIGEST_LENGTH];
    MD5(data1, length, hash1);

    long n = perm();

    double start, finish, elapsed;

    MW_init();    

    struct worker_s* workers_handle[N_WORKERS];

    Create_workers(workers_handle, N_WORKERS);

    long local_n = n / N_WORKERS;
    long remainder = n % N_WORKERS;
   
    printf("local_n: %ld\n", local_n);
    printf("remainder: %ld\n", remainder);


    GET_TIME(start);

    for (int i = 0; i < N_WORKERS; i++){
        struct task_s* task;
        void** args = malloc(sizeof(void*)*4);
        long min = local_n * i;
        long max = local_n + min;
        args[0] = (void*) hash1;
        args[1] = (void*) min;
        args[2] = (void*) max;
        task = Create_task(bruteforce, args);
        Allocate_task(workers_handle[i], task);
    }

    for (int w = 0; w < N_WORKERS; w++){
        Destroy_worker(workers_handle[w]);
    }    

    GET_TIME(finish);

    void** results = malloc(sizeof(void*)*N_WORKERS);

    results = Gather_from_workers();


    elapsed = finish - start;


    for (int i = 0; i < N_WORKERS; i++){
        if (results[i] != NULL){
            printf("il PIN e': %s\n", (char*) results[i]);
        }
    }

    printf("elapsed time : %.20f\n", elapsed);
}

void next(char *c)
{                       
    if(c[0] != '\0'){
        if(c[0] == '9'){
            c[0] = '0';
            next(c + sizeof(char));
        }
        else
            c[0]++;
    }
}

long perm()
{
    long y = 0;
    long x = 1;
    for(int i=0;i<BUFF;i++)
        x = 10*x;
    y = y + x;  
    return y;
}

void zeros_padding(char* c)
{
    int zeros = BUFF-strlen(c);
    char* x = malloc(zeros * sizeof(char));
    for(int i=0;i<zeros;i++)
        x[i] = '0';
    strcat(x, c);
    strcpy(c, x);           
}