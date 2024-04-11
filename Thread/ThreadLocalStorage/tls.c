#include "tls.h"

pthread_mutex_t MUTEX;

int page_size = 0;

struct TLS* head = NULL;
struct TLS* tail = NULL;

/* signal handler */
void tls_handle_page_fault(int sig, siginfo_t *si, void *context) {
    unsigned long int p_fault = ((unsigned long int) si->si_addr & ~(page_size - 1));
    int ii = 0;
    struct TLS* itr = head;
    
    while(itr) {
        for(ii = 0; ii < itr->page_num; ii++) {
            if(p_fault == (unsigned long int)itr->pages[ii]->address) {
                pthread_exit(NULL);
            }
        }
        itr = itr->next;
    }

    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    raise(sig);
}

void tls_init()
{
    struct sigaction sa = { NULL };

    pthread_mutex_init(&MUTEX, NULL);

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = tls_handle_page_fault;

    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);

    page_size = getpagesize();
}


int tls_create(unsigned int size)
{
    if(size == 0) {
        return -1;  
    }
    //setup signal handler
    static bool initialized = false;
    if(!initialized) {
        tls_init();
        initialized = true;
    }

    struct TLS* itr = head;

    while(itr) {
        if( itr->TID == pthread_self() ) {
            return -1;  //there is already a tls for this thread
        }
        itr = itr->next;
    }

    itr = head;
    
    if(!head) {
        itr = malloc(sizeof(struct TLS));
        itr->next = NULL;
        itr->prev = NULL;
        head = itr;
        tail = itr;
    } else {
        while(itr->next) {
            itr = itr->next;
        }
        struct TLS* tmp = itr;
        itr->next = malloc(sizeof(struct TLS));
        itr = itr->next;
        itr->next = NULL;
        itr->prev = tmp;
        tail = itr;
    }
    
    itr->TID = pthread_self();

    itr->size = size;

    itr->page_num = size / page_size + (size % page_size != 0);
    
    itr->pages = (struct page**) calloc(itr->page_num, sizeof(struct page*)); 
    
    int ii = 0;
    for(ii = 0; ii < itr->page_num; ii++) {
        itr->pages[ii] = malloc(sizeof(struct page));

        itr->pages[ii]->address = mmap(NULL, page_size, PROT_NONE, MAP_ANON | MAP_PRIVATE, -1, (off_t)0);
        if(itr->pages[ii]->address == MAP_FAILED) {
            perror("COULD NOT MAP");
        }

        itr->pages[ii]->ref_cnt = 1;
    }
    
    itr->next = NULL;

    return 0;
}


int tls_read(unsigned int offset, unsigned int length, char* buffer)
{   
    struct TLS* LSA = search(pthread_self());
    if(!LSA) {  //the thread does not have a LSA
        return -1;
    }
    if(LSA->size < offset + length) {
        return -1;      //exceed the size of the limit;
    }
    pthread_mutex_lock(&MUTEX);
    for(int ii = offset/page_size ; ii < (offset+length)/page_size + ((offset + length)%page_size != 0); ii++) {
        tls_unprotect(LSA->pages[ii]);
    }


    int cnt = 0, idx = offset;
    for(cnt = 0, idx = offset; idx < (offset + length); ++cnt, ++idx) {
        struct page *p;
        unsigned int pn, poff;
        
        pn = idx / page_size;
        poff = idx % page_size;

        p = LSA->pages[pn]; 
        char* src = ((char*)p->address) + poff;

        buffer[cnt] = *src;
    }

    pthread_mutex_unlock(&MUTEX);
    for(int ii = offset/page_size ; ii < (offset+length)/page_size + ((offset + length)%page_size != 0); ii++) {
        tls_protect(LSA->pages[ii]);
    }

    return 0;
}


int tls_write(unsigned int offset, unsigned int length, const char* buffer)
{
    struct TLS* LSA = search(pthread_self());
    if(!LSA) {
        return -1;
    }

    if((length + offset) > LSA->size) {
        return -1;
    }
    pthread_mutex_lock(&MUTEX);
    for(int ii = offset/page_size ; ii < (offset+length)/page_size + ((offset + length)%page_size != 0); ii++) {
        tls_unprotect(LSA->pages[ii]);
    }
    int cnt = 0, idx = offset;
    struct page *p, *copy;
    for(cnt = 0, idx = offset; idx < (offset + length); ++cnt, ++idx) {
        unsigned int pn, poff;
        pn = idx / page_size;
        poff = idx % page_size;
        p = LSA->pages[pn];

        if(p->ref_cnt > 1) {
            //COW FAULT
            copy = malloc(sizeof(struct page));
            copy->address = mmap(NULL, page_size, PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);
            memcpy(copy->address, p->address, page_size);
            copy->ref_cnt = 1;
            LSA->pages[pn] = copy;
            
            //update original
            (p->ref_cnt)--;
            tls_protect(p);
            p = copy;
        }
        
        char* dst = ((char *) p->address) + poff;
        *dst = buffer[cnt];
    }

    pthread_mutex_unlock(&MUTEX);
    for(int ii = offset/page_size ; ii < (offset+length)/page_size + ((offset + length)%page_size != 0); ii++) {
        tls_protect(LSA->pages[ii]);
    }

    return 0;
}

int tls_clone(pthread_t tid)
{
    struct TLS* itr = head;
    while(itr) {
        if( itr->TID == pthread_self() ) {
            return -1;  //there is already a tls for this thread
        }
        itr = itr->next;
    }
    itr = head;
    bool targetF = false;
    while(itr) {
        if(itr->TID == tid) {
            targetF = true;
            break;
        }
        itr = itr->next;
    }
    if(!targetF) {
        return -1; //target to clone does not have a tls;
    }
    
    struct TLS* target = search(tid);

    itr = (struct TLS*)malloc(sizeof(struct TLS));
    tail->next = itr;
    itr->prev = tail;
    tail = itr;

    itr->next = NULL;
    itr->TID = pthread_self();
    itr->size = target->size;
    itr->page_num = target->page_num;
    itr->pages = (struct page**) calloc(target->page_num, sizeof(struct page*));
    
    int ii = 0;
    for(ii = 0; ii < target->page_num; ii++) {
        itr->pages[ii] = target->pages[ii];
        itr->pages[ii]->ref_cnt++;
    }
    
    return 0;
}


int tls_destroy()
{
    struct TLS* itr = head;
    bool fnd = false;
    while(itr) {
        if(itr->TID == pthread_self()) {
            fnd = true;
            break;
        }
        itr = itr->next;
    }
    if(!fnd) {
        return -1; //target to clone does not have a tls;
    }
    
    if(itr->prev) {
        itr->prev->next = itr->next;
    }
    if(itr->next) {
        itr->next->prev = itr->prev;
    }
    pthread_mutex_lock(&MUTEX);
    int ii = 0;
    for(ii = 0; ii < itr->page_num; ii++) {
        if(itr->pages[ii]->ref_cnt == 1) {
            munmap(itr->pages[ii]->address, page_size);
            free(itr->pages[ii]);
        } else {
            (itr->pages[ii]->ref_cnt)--;
        }
    }
    
    free(itr->pages);
    free(itr);
    pthread_mutex_unlock(&MUTEX);
    return 0;
}

/* helper */

struct TLS* search(pthread_t id)
{
    struct TLS* itr = head;
    while(itr) {
        if(itr->TID == id) {
            break;
        }
        if(itr == tail && itr->TID != id) {
            itr = NULL;
            break;
        }   
        itr = itr->next;
    }
    return itr;
}


void tls_protect(struct page *p)
{
    if(mprotect((void*)p->address, page_size, 0)) {
        fprintf(stderr, "tls_protect: could not protect page\n");
        exit(1);
    }
}

void tls_unprotect(struct page *p)
{
    if(mprotect((void*)p->address, page_size, PROT_READ | PROT_WRITE)) {
        fprintf(stderr, "tls_unprotect: could not unprotect page\n");
        exit(1);
    }
}
