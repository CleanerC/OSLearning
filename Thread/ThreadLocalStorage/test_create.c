#include "tls.h"

void *f(void* size ) {
 
    int fail = tls_create(0);
    assert(fail == -1);

    int* retval = malloc(sizeof(int));

    *retval = tls_create(*(unsigned int*)size);
    
    fail = tls_create(1);
    assert(fail == -1);
   
    pthread_exit(retval);
}

int main() {
    //have to use gdb to check the tls_create function. 

    pthread_t id1, id2;
    
    void* arg1 = malloc(sizeof(int));
    void* arg2 = malloc(sizeof(int));

    *(int*)arg1 = 5;
    *(int*)arg2 = 2 * getpagesize(); 
    
    pthread_create(&id1, NULL, f, arg1);
    pthread_create(&id2, NULL, f, arg2);

    void* pret1 = NULL;
    void* pret2 = NULL;

    pthread_join(id1, &pret1);
    pthread_join(id2, &pret2);

    int ret1 = *(int*)pret1;
    int ret2 = *(int*)pret2;

    assert(ret1 == 0);
    assert(ret2 == 0);
     
    free(pret1);
    free(pret2);
    free(arg1);
    free(arg2);

    exit(0);
}
