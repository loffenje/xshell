#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xshell.h"
#include "pipes.h"

#ifndef VERSION
    #define VERSION 1
#endif

char *history[MAX_HISTORY_STACK];

int head = 0;

ssize_t xgetline(char **lineptr, FILE *stream)
{
  size_t len = 0; 

  ssize_t chars = getline(lineptr, &len, stream);

  if((*lineptr)[chars - 1] == '\n') {
    (*lineptr)[chars - 1] = '\0';
    --chars;
  }

  return chars;
}

char *cmds[] = {
    "cd",
    "history",
    "--help",
    "q"
};

char *cmds_info[] = {
    "Change a diretory. Usage cd <dir>",
    "history of recently called commands",
    "Usage --help <fn?>",
};

int (*cmds_fn[]) (char **) = {
    &fn_cd,
    &fn_history,
    &fn_help,
    &fn_q
};

int cmds_len() 
{
    return sizeof(cmds) / sizeof(char *);
}

int fn_cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "cd expected at least 1 argument");
    } else {
        push_h(args[0]);
        if (chdir(args[1]) != 0) {
            perror("cd");
        }
    }

    return 1;
}

int fn_history(char **args __attribute__ ((unused)))
{
    for (int i = 0; i < len_h(); i++) {
        if (history[i] == NULL) {
            return 1;
        }

        printf("%s\n", history[i]);
    }

    return 1;
}

int fn_q(char **args __attribute__ ((unused)))
{    
    exit(EXIT_SUCCESS);
}

int fn_help(char **args)
{
    if (strcmp(args[0], "--help") != 0) {
        return 1;
    }

    push_h(args[0]);


    if (args[1] == NULL) {
        printf("Xshell version %d\n", VERSION);
        return 1;
    }

    for (int i = 0; i < cmds_len(); i++) {
        if (strcmp(cmds[i], args[1]) == 0) {
            printf("%s\n", cmds_info[i]);
            return 1;
        }
    }

    return 1;
}

int cmds_exec(char *args[])
{
    for (int i = 0; i < cmds_len(); i++) {
        if (strcmp(cmds[i], args[0]) == 0) {
            return (*cmds_fn[i])(args);
        }
    } 

    return -1;
}

void close_pipes(int pipes_len, int (*pipes)[2])
{
    for (int i = 0; i < pipes_len; ++i) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
}

int exec_through_pipes(Command *cmd, int pipes_len, int (*pipes)[2])
{
    int fd = -1;
    if ((fd = cmd->io_redirect[0]) != -1) {
        dup2(fd, STDIN_FILENO);
    }

    if ((fd = cmd->io_redirect[1]) != -1) {
        dup2(fd, STDOUT_FILENO);
    }

    close_pipes(pipes_len, pipes);
    return execvp(cmd->name, cmd->args);
}

void wait_childs(int process)
{
    int status;

     do {
        waitpid(process, &status, WUNTRACED);
    } while (!WIFEXITED(process) && !WIFSIGNALED(process));
}

pid_t run_xshell(Command *cmd, int pipes_len, int(*pipes)[2]) 
{
    if (cmds_exec(cmd->args) != -1) return 1;

    pid_t process;

    process = fork();
    if (process) {
        switch(process) {
        case -1:
            return -1;
        default:
            cmd->process = process;
            if (pipes_len == 0) wait_childs(process);
            return process;
        }
    } else {
        exec_through_pipes(cmd, pipes_len, pipes);
        exit(EXIT_FAILURE);
    }
}

void init_xshell()
{
    char *line = NULL;

    do {
        printf("> ");
        char *line = NULL;
        xgetline(&line, stdin);

        if (*line == '\0') {
            continue;
        }
        
        Pipeline *pipeline = parse_pipeline(line);
        int pipes_len = pipeline->cmds_len - 1;

        int (*pipes)[2] = calloc(sizeof(int[2]), pipes_len);

        for (int i = 1; i < pipeline->cmds_len; ++i) {
            pipe(pipes[i -1]);
            pipeline->cmds[i]->io_redirect[STDIN_FILENO] = pipes[i-1][0];
            pipeline->cmds[i-1]->io_redirect[STDOUT_FILENO] = pipes[i-1][1];
        }

        for (int i = 0; i < pipeline->cmds_len; ++i) {
            run_xshell(pipeline->cmds[i], pipes_len, pipes);
        }

        close_pipes(pipes_len, pipes);

        for (int i = 0; i < pipes_len; ++i) {
            wait_childs(pipeline->cmds[i]->process);
        }

    } while(true);
}