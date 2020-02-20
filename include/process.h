#ifndef INCLUDE_PROCESS_H
#define INCLUDE_PROCESS_H

#include "file.h"
#include "mem.h"
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

#define PROC_STATE_RUNNING_STR "2"

struct process {
    p_list_t list;
    uint16_t sp;
    uint8_t cbr;
    enum proc_state state;
    pid_t pid;
    pid_t ppid;
    int status;
    pid_t wait_pid;
    int wait_options;
    struct process *wait_process;
    struct open_file *open_files[MAX_OPEN_FILES];
};

#define PROCESS_OFFSETOF_SP_STR "4"
#define PROCESS_OFFSETOF_CBR_STR "6"
#define PROCESS_OFFSETOF_STATE_STR "7"

extern volatile p_list_t process_ready_list;
extern volatile p_list_t process_zombie_list;

extern struct process * volatile current_proc;

void process_init(void);
struct process *process_new(void);
void process_destroy(struct process *destroy_proc);
// Must enter with interrupts disabled
void process_switch(struct process *next_proc) __z88dk_fastcall;
// Must enter with interrupts disabled
void process_schedule(void);
void process_tick(void);
int process_clone_files(struct process *to, struct process *from);

int sys_fork(void);
pid_t sys_waitpid(pid_t pid, USER_PTR(int) wstatus, int options);
void sys_exit(int status);

void syscall_leave(void);

#endif
