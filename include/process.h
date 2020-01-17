#ifndef INCLUDE_PROCESS_H
#define INCLUDE_PROCESS_H

#include "file.h"
#include <adt.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define WNOHANG ((int) 0x1)

enum proc_state {
    EMPTY = 0,
    READY = 1,
    RUNNING = 2,
    BLOCKED,
    ZOMBIE
};

#define PROC_STATE_READY_STR "1"
#define PROC_STATE_RUNNING_STR "2"

struct process {
    p_list_t list;
    uint16_t sp;
    uint8_t cbr;
    enum proc_state state;
    pid_t pid;
    pid_t ppid;
    int status;
    struct open_file *open_files[MAX_OPEN_FILES];
};

#define PROCESS_OFFSETOF_SP_STR "4"
#define PROCESS_OFFSETOF_CBR_STR "6"
#define PROCESS_OFFSETOF_STATE_STR "7"

extern volatile p_list_t process_ready_list;
extern volatile p_list_t process_zombie_list;

extern volatile struct process *current_proc;

void process_init(void);
struct process *process_new(void);
// Must enter with interrupts disabled
void process_switch(struct process *next_proc) __z88dk_fastcall;
void process_schedule(void);
void process_tick(void);
int sys_fork(void);
pid_t sys_waitpid(pid_t pid, uintptr_t /* int * */ wstatus, int options);
void syscall_leave(void);

#endif
