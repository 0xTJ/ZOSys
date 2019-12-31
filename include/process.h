#ifndef INCLUDE_PROCESS_H
#define INCLUDE_PROCESS_H

#include <adt.h>
#include <stdint.h>
#include <sys/types.h>

struct process {
    p_list_t list;
    pid_t pid;
    uint16_t sp;
    uint8_t bbr;
};

extern volatile p_list_t proc_list;
extern volatile struct process *current_proc;

void process_init(void);
struct process *process_new(void);

#endif
