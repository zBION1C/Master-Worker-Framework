#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <unistd.h>
#include "../mw_framework/mw_fw.c"
#include "../timer.h"

#define BUFF 5

long perm()
{
	long y = 0;
	long x = 1;
	for(int i=0;i<BUFF;i++)
	    x = 10*x;
	y = y + x;  
	return y;
}

void zeros_padding(unsigned char* c)
{
    int zeros = BUFF-strlen((char*)c);
    char* x = malloc(zeros * sizeof(char));
    for(int i=0;i<zeros;i++)
        x[i] = '0';
    strcat(x, (char*)c);
    strcpy((char*)c, x);
    free(x);        
}

unsigned char* bruteforce(void** args)
{
    long min = (long) args[0];
    long max = (long) args[1]; 
    unsigned char* hash = (unsigned char*) args[2];

    unsigned char *c = malloc(BUFF * sizeof(char));
    int err = 0;
    unsigned char hash2[SHA_DIGEST_LENGTH];

    for (long k = min; k < max; k++){
        sprintf((char*)c, "%ld", k);
        zeros_padding(c);
        
        SHA_CTX sctx;
        SHA1_Init(&sctx);
        SHA1_Update(&sctx, c, strlen((char*)c));
        SHA1_Final(hash2, &sctx);
        
        err = 0;
        for (int i=0; i < SHA_DIGEST_LENGTH; i++){
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
    mw_init();
    double start, finish, elapsed;
	
	const unsigned char* data1 = "5645389";
    long n = perm();

    size_t length = strlen((char*)data1); 
    
    unsigned char hash1[SHA_DIGEST_LENGTH];
    
    SHA_CTX sctx;
    SHA1_Init(&sctx);
    SHA1_Update(&sctx, data1, length);
    SHA1_Final(hash1, &sctx);
    
    void* args1[3];
    Task* task1;

    int min = 0;
    int max = n;

    GET_TIME(start);

   	args1[0] = (void*) min;
   	args1[1] = (void*) max;
   	args1[2] = (void*) hash1;
   	task1 = create_task(bruteforce, args1, 3);

    change_alloc_mode(DYNAMIC);

	allocate_master(task1, 5);

    char result[BUFF];

    gather(result);
    printf("il PIN e': %s\n", result);

    GET_TIME(finish);
    elapsed = finish - start;

    printf("elapsed time: %.20f\n", elapsed);

	mw_terminate();

	return 0;
}