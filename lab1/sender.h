#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <mqueue.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#define SHM_SIZE 1024
#define MQ_NAME "/mq_posix"
#define SHM_NAME "/shm_posix"
#define SENDER_SEM_NAME "/sender_sem"
#define RECEIVER_SEM_NAME "/receiver_sem"

typedef struct {
    int flag;  // 1 for message passing, 2 for shared memory
    union {
        int mq;
        char* shm_addr;
    } storage;
    sem_t *sender_sem, *receiver_sem;
} mailbox_t;

typedef struct {
    /*  TODO:
        Message structure for wrapper
    */
    char msg_text[SHM_SIZE];
} message_t;

void send(message_t message, mailbox_t* mailbox_ptr);

const char* RED = "\033[0;31m";
const char* BLUE = "\033[0;34m";
const char* GREEN = "\033[0;32m";
const char* RESET = "\033[0m";