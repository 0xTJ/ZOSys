#ifndef INCLUDE_PROCESS_H
#define INCLUDE_PROCESS_H

#include <adt.h>
#include <stdint.h>
#include <sys/types.h>

#define WNOHANG ((int) 0x1)

enum proc_state {
    EMPTY,
    READY,
    RUNNING,
    ZOMBIE
};

struct process {
    p_list_t list;
    pid_t pid;
    pid_t ppid;
    enum proc_state state;
    uint8_t cbr;
    uint8_t cbar;
    uint16_t sp;
    int status;
};

extern volatile p_list_t proc_list;
extern volatile struct process *current_proc;

void process_init(void);
struct process *process_new(void);
int sys_fork(void);
pid_t sys_waitpid(pid_t pid, uintptr_t /* int * */ wstatus, int options);

#endif
