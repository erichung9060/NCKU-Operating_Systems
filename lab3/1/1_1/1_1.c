#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

volatile int a = 0;
pthread_spinlock_t lock;

void *thread(void *arg) {
    for (int i = 0; i < 10000; i++) {
        pthread_spin_lock(&lock);
        a = a + 1;
        pthread_spin_unlock(&lock);
    }
    return NULL;
}

int main() {
    FILE *fptr;
    fptr = fopen("1.txt", "a");
    pthread_t t1, t2;

    pthread_spin_init(&lock, 0);
    pthread_create(&t1, NULL, thread, NULL);
    pthread_create(&t2, NULL, thread, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_spin_destroy(&lock);

    fprintf(fptr, "%d ", a);
    fclose(fptr);
}
