#define MAX_LINE 1024

typedef struct Command_t {
    char *name;
    int io_redirect[2];
    int process;
    char *args[];
} Command;

typedef struct Pipeline_t {
    int cmds_len;
    Command* cmds[];
} Pipeline;

Pipeline *parse_pipeline(char *cmd);

Command *parse_arguments(char *line);