#include "sender.h"

void send(message_t message, mailbox_t* mailbox_ptr) {
    if (mailbox_ptr->flag == 1) {
        mq_send(mailbox_ptr->storage.mq, message.msg_text, SHM_SIZE, 0);
    } else {
        strcpy(mailbox_ptr->storage.shm_addr, message.msg_text);
    }
}

int main(int argc, char* argv[]) {
    /*  TODO:
        1) Call send(message, &mailbox) according to the flow in slide 4
        2) Measure the total sending time
        3) Get the mechanism and the input file from command line arguments
            • e.g. ./sender 1 input.txt
                    (1 for Message Passing, 2 for Shared Memory)
        4) Get the messages to be sent from the input file
        5) Print information on the console according to the output format
        6) If the message form the input file is EOF, send an exit message to the receiver.c
        7) Print the total sending time and terminate the sender.c
    */
    int method = atoi(argv[1]);
    const char* input_file = argv[2];

    FILE* file = fopen(input_file, "r");

    mailbox_t mailbox;
    mailbox.flag = method;

    if (mailbox.flag == 1) {  // Message Passing
        printf("%sMessage Passing%s\n", BLUE, RESET);
        struct mq_attr attr;
        attr.mq_flags = 0;
        attr.mq_maxmsg = 10;
        attr.mq_msgsize = SHM_SIZE;
        attr.mq_curmsgs = 0;
        mailbox.storage.mq = mq_open(MQ_NAME, O_CREAT | O_WRONLY, 0666, &attr);
    } else {  // Shared Memory
        printf("%sShared Memory%s\n", BLUE, RESET);
        int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
        if (ftruncate(shm_fd, SHM_SIZE) == -1) {
            perror("ftruncate");
            exit(EXIT_FAILURE);
        }
        mailbox.storage.shm_addr = mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    }

    mailbox.sender_sem = sem_open(SENDER_SEM_NAME, O_CREAT, 0666, 1);
    mailbox.receiver_sem = sem_open(RECEIVER_SEM_NAME, O_CREAT, 0666, 0);

    double time_taken = 0;
    struct timespec start, end;

    message_t message;
    while (fgets(message.msg_text, SHM_SIZE, file) != NULL) {
        sem_wait(mailbox.sender_sem);
        clock_gettime(CLOCK_MONOTONIC, &start);
        send(message, &mailbox);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sem_post(mailbox.receiver_sem);

        time_taken += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;

        printf("%sSending message:%s %s", BLUE, RESET, message.msg_text);
    }

    fclose(file);

    // sent an exit message
    strcpy(message.msg_text, "EXIT");
    sem_wait(mailbox.sender_sem);
    send(message, &mailbox);
    sem_post(mailbox.receiver_sem);
    printf("%sEnd of input file! exit!%s\n", RED, RESET);

    printf("Total time taken in sending msg: %f s\n", time_taken);

    if (mailbox.flag == 1) {
        mq_close(mailbox.storage.mq);
        mq_unlink(MQ_NAME);
    } else {
        munmap(mailbox.storage.shm_addr, SHM_SIZE);
        shm_unlink(SHM_NAME);
    }

    sem_close(mailbox.receiver_sem);
    sem_close(mailbox.sender_sem);
    sem_unlink(SENDER_SEM_NAME);
    sem_unlink(RECEIVER_SEM_NAME);
}