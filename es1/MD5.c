#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md4.h>
#include <unistd.h>
#include "../mw_framework/mw_fw.c"
#include "../timer.h"

int buff;
size_t length;

long perm()
{
    long y = 0;
    long x = 1;
    for(int i=0;i<length;i++)
        x = 10*x;
    y = y + x;  
    return y;
}

void zeros_padding(unsigned char* c)
{
    int zeros = length-strlen((char*)c);
    char* x = malloc(zeros * sizeof(char));
    for(int i=0;i<zeros;i++)
        x[i] = '0';
    strcat(x, (char*)c);
    strcpy((char*)c, x);
    free(x);        
}

unsigned char* bruteforce(void** args)
{
    /* Esclusivamente per la simulazione di errori
    static cnt = 0;
    Worker * w = (Worker*) args[0]; 

    if (cnt <= 2) {
        cnt++;
        pthread_mutex_lock(&w->lock);
        w->error = 1;
        pthread_mutex_unlock(&w->lock);
    }
    */

    /* inizio funzione effettiva */
    long min = (long) args[1];
    long max = (long) args[2]; 
    unsigned char* hash = (unsigned char*) args[3]; 

    unsigned char *c = malloc(length * sizeof(char));
    int err = 0;
    unsigned char hash2[MD4_DIGEST_LENGTH];

    for (long k = min; k < max; k++){
        sprintf((char*)c, "%ld", k);
        zeros_padding(c);
        MD4(c, strlen((char*)c), hash2);
        err = 0;
        for (int i=0; i < MD4_DIGEST_LENGTH; i++){ 
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
    double start, finish, elapsed;
    int workers = strtol(argv[1], NULL, 10);
    unsigned char* data1 = argv[2];
    buff = strtol(argv[3], NULL, 10);
    
    mw_init();
    
    unsigned char hash1[MD4_DIGEST_LENGTH];
    
    length = strlen((char*)data1); 
    
    long n = perm();

    zeros_padding(data1);

    MD4(data1, length, hash1);

    void* args1[3];
    Task* task1;

    int min = 0;
    long max = n;

    GET_TIME(start);

   	args1[0] = (void*) min;
   	args1[1] = (void*) max;
   	args1[2] = (void*) hash1;
   	task1 = create_task(bruteforce, args1, 3);

    //change_alloc_mode(STATIC);

	allocate_master(task1, workers);

    char result[buff];

    gather(result);
    printf("il PIN e': %s\n", result);

    GET_TIME(finish);
    elapsed = finish - start;

    printf("elapsed time: %.20f\n", elapsed);

	mw_terminate();

	return 0;
}