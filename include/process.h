#ifndef INCLUDE_PROCESS_H
#define INCLUDE_PROCESS_H

#include <adt.h>
#include <stdint.h>
#include <sys/types.h>

enum proc_state {
    EMPTY,
    READY,
    RUNNING,
    ZOMBIE
};

struct process {
    p_list_t list;
    pid_t pid;
    enum proc_state state;
    uint8_t cbr;
    uint8_t cbar;
    uint16_t sp;
};

extern volatile p_list_t proc_list;
extern volatile struct process *current_proc;

void process_init(void);
struct process *process_new(void);
int sys_fork(void);

#endif
