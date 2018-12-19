#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "xshell.h"

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
    return EXIT_SUCCESS;
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

int cmds_exec(char **args)
{
    for (int i = 0; i < cmds_len(); i++) {
        if (strcmp(cmds[i], args[0]) == 0) {
            return (*cmds_fn[i])(args);
        }
    }
       
    pid_t process;
    int status;

    process = fork();
    if (process == 0) {
        if (execvp(args[0], args) == -1) {
            perror(args[0]);
        } 

        exit(EXIT_FAILURE);
    } else if (process < 0) {
        perror("fail proc");
    } else {
        do {
            waitpid(process, &status, WUNTRACED);
        } while (!WIFEXITED(process) && !WIFSIGNALED(process));
    }

    return 1;
}

int boot_xshell(char **args)
{
    if (args[0] == NULL) {
        return 1;
    }

    return cmds_exec(args);
}

char **parse_arguments(char *line)
{
    char **args = malloc(MAX_LINE_SIZE * sizeof(char *));
    char delim[2] = " ";
    char *arg = NULL;
    int index = 0;
    arg = strtok(line, delim);

    while (arg != NULL) {
        args[index] = arg;
        index++;

        arg = strtok(NULL, delim);
    }

    args[index] = NULL;

    return args;
}

void init_xshell()
{
    int status;
    do {
        printf("> ");
        char *line = NULL;
        ssize_t chars = xgetline(&line, stdin);
  
        char **args = parse_arguments(line);
        status = boot_xshell(args);

        free(args);
    } while(status);
}