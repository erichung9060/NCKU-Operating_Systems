#include "receiver.h"

void receive(message_t* message_ptr, mailbox_t* mailbox_ptr) {
    if (mailbox_ptr->flag == 1) {
        mq_receive(mailbox_ptr->storage.mq, message_ptr->msg_text, SHM_SIZE, NULL);
    } else {
        strcpy(message_ptr->msg_text, mailbox_ptr->storage.shm_addr);
    }
}

int main(int argc, char* argv[]) {
    /*  TODO:
        1) Call receive(&message, &mailbox) according to the flow in slide 4
        2) Measure the total receiving time
        3) Get the mechanism from command line arguments
            • e.g. ./receiver 1
        4) Print information on the console according to the output format
        5) If the exit message is received, print the total receiving time and terminate the receiver.c
    */
    int method = atoi(argv[1]);

    mailbox_t mailbox;
    mailbox.flag = method;

    if (mailbox.flag == 1) {  // Message Passing
        printf("%sMessage Passing%s\n", BLUE, RESET);
        mailbox.storage.mq = mq_open(MQ_NAME, O_RDONLY);

    } else {  // Shared Memory
        printf("%sShared Memory%s\n", BLUE, RESET);
        int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
        mailbox.storage.shm_addr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    }

    mailbox.sender_sem = sem_open(SENDER_SEM_NAME, 0);
    mailbox.receiver_sem = sem_open(RECEIVER_SEM_NAME, 0);

    double time_taken = 0;
    struct timespec start, end;

    message_t message;
    while (1) {
        sem_wait(mailbox.receiver_sem);
        clock_gettime(CLOCK_MONOTONIC, &start);
        receive(&message, &mailbox);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sem_post(mailbox.sender_sem);

        time_taken += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;

        if (strcmp(message.msg_text, "EXIT") == 0) break;
        printf("%sReceiving message:%s %s", BLUE, RESET, message.msg_text);
    }
    printf("%sSender exit!%s\n", RED, RESET);

    printf("Total time taken in receiving msg: %f s\n", time_taken);

    if (mailbox.flag == 1) {
        mq_close(mailbox.storage.mq);
    } else {
        munmap(mailbox.storage.shm_addr, SHM_SIZE);
    }

    sem_close(mailbox.receiver_sem);
    sem_close(mailbox.sender_sem);
}