#define MAX_HISTORY_STACK 512

extern char *history[MAX_HISTORY_STACK];
extern int head;

#define len_h() (sizeof(history) / sizeof(char))
#define fits__h() (head != MAX_HISTORY_STACK - 1)
#define push_h(p) if (fits__h()) {\
        history[head] = p;\
        head++;\
    }

void init_xshell();

int boot_xshell(char **args);

int fn_cd(char **args);

int fn_history(char **args);

int fn_q(char **args);

int fn_help(char **args);
