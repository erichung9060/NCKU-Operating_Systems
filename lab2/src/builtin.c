#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include "../include/builtin.h"



/**
 * @brief 
 * Determine whether cmd is a built-in command
 * @param cmd Command structure
 * @return int 
 * If command is built-in command return function number
 * If command is external command return -1 
 */
int searchBuiltInCommand(struct cmd_node *cmd)
{
	for (int i = 0; i < num_builtins(); ++i){
		if (strcmp(cmd->args[0], builtin_str[i]) == 0){
			return i;
		}
	}
	return -1;
}
/**
 * @brief Execute built-in command
 * 
 * @param status Choose which built-in command to execute
 * @param cmd Command structure
 * @return int 
 * Return execution status
 */
int execBuiltInCommand(int status,struct cmd_node *cmd){
	status = (*builtin_func[status])(cmd->args);
	return status;
}

int help(char **args)
{
	int i;
    printf("--------------------------------------------------\n");
  	printf("My Little Shell!!\n");
	printf("The following are built in:\n");
	for (i = 0; i < num_builtins(); i++) {
    	printf("%d: %s\n", i, builtin_str[i]);
  	}
    printf("--------------------------------------------------\n");
	return 1;
}
// ======================= requirement 2.1 =======================
int cd(char **args)
{
	return 1;
}
// ===============================================================

int pwd(char **args)
{
	char cwd[BUF_SIZE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("pwd");
    }
    return 1;
}

int echo(char **args)
{
	bool newline = true;
	for (int i = 1; args[i]; ++i) {
		if (i == 1 && strcmp(args[i], "-n") == 0) {
			newline = false;
			continue;
		}
		printf("%s", args[i]);
		if (args[i + 1])
			printf(" ");
	}
	if (newline)
		printf("\n");

	return 1;
}

int exit_shell(char **args)
{
	return 0;
}

int record(char **args)
{
	if (history_count < MAX_RECORD_NUM) {
		for (int i = 0; i < history_count; ++i)
			printf("%2d: %s\n", i + 1, history[i]);
	} else {
		for (int i = history_count % MAX_RECORD_NUM; i < history_count % MAX_RECORD_NUM + MAX_RECORD_NUM; ++i)
			printf("%2d: %s\n", i - history_count % MAX_RECORD_NUM + 1, history[i % MAX_RECORD_NUM]);
	}
	return 1;
}

const char *builtin_str[] = {
 	"help",
 	"cd",
	"pwd",
	"echo",
 	"exit",
 	"record",
};

const int (*builtin_func[]) (char **) = {
	&help,
	&cd,
	&pwd,
	&echo,
	&exit_shell,
  	&record,
};

int num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}
