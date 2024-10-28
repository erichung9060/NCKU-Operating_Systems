#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/builtin.h"
#include "../include/command.h"

// ======================= requirement 2.3 =======================
/**
 * @brief
 * Redirect command's stdin and stdout to the specified file descriptor
 * If you want to implement ( < , > ), use "in_file" and "out_file" included the cmd_node structure
 * If you want to implement ( | ), use "in" and "out" included the cmd_node structure.
 *
 * @param p cmd_node structure
 *
 */
void redirection(struct cmd_node *cmd) {
    if (cmd->in_file) {
        int fd = open(cmd->in_file, O_RDONLY);
        if (fd == -1) {
            perror(cmd->in_file);
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    } else if (cmd->in != STDIN_FILENO) {
        dup2(cmd->in, STDIN_FILENO);
    }

    if (cmd->out_file) {
        int fd = open(cmd->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror(cmd->out_file);
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    } else if (cmd->out != STDOUT_FILENO) {
        dup2(cmd->out, STDOUT_FILENO);
    }
}
// ===============================================================

// ======================= requirement 2.2 =======================
/**
 * @brief
 * Execute external command
 * The external command is mainly divided into the following two steps:
 * 1. Call "fork()" to create child process
 * 2. Call "execvp()" to execute the corresponding executable file
 * @param p cmd_node structure
 * @return int
 * Return execution status
 */
int spawn_proc(struct cmd_node *p) {
    pid_t pid = fork();

    if (pid == 0) {  // child process
        redirection(p);
        int status = execvp(p->args[0], p->args);
        if (status == -1) {
            perror("execvp");  // execvp only returns on failure
            exit(EXIT_FAILURE);
        }
    } else {  // parent process
        waitpid(pid, NULL, 0);
    }
    return 0;
}
// ===============================================================

// ======================= requirement 2.4 =======================
/**
 * @brief
 * Use "pipe()" to create a communication bridge between processes
 * Call "spawn_proc()" in order according to the number of cmd_node
 * @param cmd Command structure
 * @return int
 * Return execution status
 */
int fork_cmd_node(struct cmd *cmd) {
    struct cmd_node *p = cmd->head;
    while (p) {
        if (p->next) {
            int fd[2];
            pipe(fd);

            p->out = fd[1];
            p->next->in = fd[0];
        }

        spawn_proc(p);

        if (p != cmd->head) close(p->in);
        if (p->next != NULL) close(p->out);

        p = p->next;
    }

    return 0;
}
// ===============================================================

void shell() {
    while (1) {
        printf(">>> $ ");
        char *buffer = read_line();
        if (buffer == NULL) continue;

        struct cmd *cmd = split_line(buffer);

        int status = -1;

        struct cmd_node *first = cmd->head;
        if (first->next == NULL) {  // only a single command
            int build_in_command_id = searchBuiltInCommand(first);
            if (build_in_command_id != -1) {
                int in = dup(STDIN_FILENO), out = dup(STDOUT_FILENO);  // backup STDIN and STDOUT
                if ((in == -1) | (out == -1)) perror("dup");

                redirection(first);
                status = execBuiltInCommand(build_in_command_id, first);

                // recover shell stdin and stdout
                if (first->in_file) dup2(in, STDIN_FILENO);
                if (first->out_file) dup2(out, STDOUT_FILENO);

                close(in);
                close(out);
            } else {
                status = spawn_proc(first);
            }
        } else {  // There are multiple commands ( | )
            status = fork_cmd_node(cmd);
        }

        // free space
        while (cmd->head) {
            struct cmd_node *temp = cmd->head;
            cmd->head = cmd->head->next;
            free(temp->args);
            free(temp);
        }
        free(cmd);
        free(buffer);

        if (status != 0) break;
    }
}
