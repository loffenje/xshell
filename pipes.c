#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pipes.h"

Pipeline *parse_pipeline(char *cmd)
{
    char *dup = strndup(cmd, MAX_LINE);
    char *cmd_str;
    int cmds_len = 1;
    int i = 0;
    Pipeline *pipeline;

    for (char *c = dup; *c; c++) {
        if (*c == '|') ++cmds_len;
    }

    pipeline = calloc(sizeof(Pipeline) + cmds_len * sizeof(Command *), 1);
    pipeline->cmds_len = cmds_len;

    while((cmd_str = strsep(&dup, "|"))) {
        pipeline->cmds[i++] = parse_arguments(cmd_str);
    }

    return pipeline;
}

Command *parse_arguments(char *line)
{
    char **args = malloc(MAX_LINE * sizeof(char *));
    char delim[2] = " ";
    char *arg = NULL;
    int index = 0;
    int i = 0;
    arg = strtok(line, delim);
    Command *cmd = calloc(sizeof(Command) + MAX_LINE * sizeof(char *), 1);

    while (arg != NULL) {
        cmd->args[index++] = arg;
        cmd->name = cmd->args[0];
        cmd->io_redirect[0] = cmd->io_redirect[1] = -1;

        arg = strtok(NULL, delim);
    }

    return cmd;
}