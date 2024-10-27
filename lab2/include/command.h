#ifndef COMMAND_H
#define COMMAND_H

#define MAX_RECORD_NUM 16
#define BUF_SIZE 1024

#include <stdbool.h>

struct cmd_node {
	char **args;
	int length;
	char *in_file, *out_file;
	int in,out;
	struct cmd_node *next;
	
};

struct cmd {
	struct cmd_node *head;
	int pipe_num;
};

extern char *history[MAX_RECORD_NUM];
extern int history_count;

char *read_line();
struct cmd *split_line(char *);
void test_cmd_struct(struct cmd *);
void test_pipe_struct(struct cmd_node *pipe);
#endif
