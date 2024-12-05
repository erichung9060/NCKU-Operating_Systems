#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*Note: Value of LOCK is 0 and value of UNLOCK is 1.*/
#define LOCK 0
#define UNLOCK 1

volatile int a = 0;
volatile int lock = UNLOCK;
pthread_mutex_t mutex;

void spin_lock() {
    asm volatile(
        "loop:\n\t"
        "mov $0, %%eax\n\t"
        "xchg %%eax, %[lock]\n\t"
        "cmpl $1, %%eax\n\t"
        "jne loop\n\t"
        :
        : [lock] "m"(lock)
        : "eax", "memory"
    );
}

void spin_unlock() {
    asm volatile(
        "mov $1, %%eax\n\t"
        "xchg %%eax, %[lock]\n\t"
        :
        : [lock] "m"(lock)
        : "eax", "memory"
    );
}

void *thread(void *arg) {
    for (int i = 0; i < 10000; i++) {
        spin_lock();
        a = a + 1;
        spin_unlock();
    }
    return NULL;
}

int main() {
    FILE *fptr;
    fptr = fopen("1.txt", "a");
    pthread_t t1, t2;

    pthread_mutex_init(&mutex, 0);
    pthread_create(&t1, NULL, thread, NULL);
    pthread_create(&t2, NULL, thread, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_mutex_destroy(&mutex);

    fprintf(fptr, "%d ", a);
    fclose(fptr);
}
