#include "process.h"
#include "context.h"
#include "dma.h"
#include "mutex.h"
#include <arch/scz180.h>
#include <cpu.h>
#include <z180.h>
#include <errno.h>
#include <intrinsic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#pragma portmode z180

extern uintptr_t syscall_sp;

struct created_process {
    p_list_t proc_list;
    struct process proc;
};

volatile p_list_t process_list;         /* struct created_process */
volatile p_list_t process_ready_list;   /* struct process */
volatile p_list_t process_zombie_list;  /* struct process */
volatile p_list_t process_wait_list;    /* struct process */

struct process * volatile current_proc = NULL;
volatile pid_t next_pid = 0;

void process_init(void) {
    p_list_init(&process_list);
    p_list_init(&process_ready_list);
    p_list_init(&process_zombie_list);
    p_list_init(&process_wait_list);

    current_proc = process_new();
    if (!current_proc) {
        // panic();
        // TODO: Add panic()
    }
    current_proc->state = RUNNING;
    current_proc->cbr = CBR;
    current_proc->cwd = NULL;
}

struct process *process_new(void) {
    // Allocate new process struct
    struct created_process *new_created_proc = malloc(sizeof(struct created_process));
    if (!new_created_proc)
        return NULL;

    struct process *new_proc = &new_created_proc->proc;

    // Setup process fields
    if (next_pid > 0) {
        // Only allocate a page block if it's not the first process
        new_proc->cbr = mem_alloc_page_block();
    }
    new_proc->ppid = 0;
    new_proc->state = EMPTY;
    memset(new_proc->open_files, 0, sizeof(new_proc->open_files));

    uint8_t int_state = cpu_get_int_state();
    intrinsic_di();

    // Get new PID and assign it to new process
    new_proc->pid = next_pid++;

    // Add process to list of all existing processes
    p_list_push_back(&process_list, new_created_proc);

    cpu_set_int_state(int_state);

    return new_proc;
}

void process_destroy(struct process *destroy_proc) __critical {
    mem_free_page_block(destroy_proc->cbr);
    struct created_process *destroy_created_proc = (struct created_process *) ((char *) destroy_proc - offsetof(struct created_process, proc));
    p_list_remove(&process_list, destroy_created_proc);
    free(destroy_created_proc);
}

/*
+--------+
| Return |
+--------+
|   IX   |
+--------+
|   IY   |
+--------+
*/
// Must enter with interrupts disabled
// Task to be switched from must already have set to not running
void process_switch(struct process *next_proc) __z88dk_fastcall {
    __asm__("\
        ld bc, hl \n\
        push ix \n\
        push iy \n\
        ld iy, (_current_proc) \n\
        in0 a, (_CBR) \n\
        ld (iy+" PROCESS_OFFSETOF_CBR_STR "), a \n\
        ld hl, 0 \n\
        add hl, sp \n\
        ld (iy+" PROCESS_OFFSETOF_SP_STR "), l \n\
        ld (iy+" PROCESS_OFFSETOF_SP_STR "+1), h \n\
        ");
    __asm__("\
        ld (_current_proc), bc \n\
        ");
    __asm__("\
        ld iy, (_current_proc) \n\
        ld l, (iy+" PROCESS_OFFSETOF_SP_STR ") \n\
        ld h, (iy+" PROCESS_OFFSETOF_SP_STR "+1) \n\
        ld sp, hl \n\
        ld a, (iy+" PROCESS_OFFSETOF_CBR_STR ") \n\
        out0 (_CBR), a \n\
        ld a, " PROC_STATE_RUNNING_STR "\n\
        ld (iy+" PROCESS_OFFSETOF_STATE_STR "), a \n\
        pop iy \n\
        pop ix \n\
        ");
    io_led_output = current_proc->pid;
    (void) next_proc;
}

// Must enter with interrupts disabled
void process_schedule(void) {
    struct process *next_proc = p_list_pop_front(&process_ready_list);

    if (next_proc->pid == 0) {
        // Is idle process
        if (!p_list_empty(&process_ready_list)) {
            // Another process can run
            p_list_push_back(&process_ready_list, next_proc);
            next_proc = p_list_pop_front(&process_ready_list);
        }
    }

    if (next_proc) {
        process_switch(next_proc);
    } else {
        // TODO: Panic, idle task is not available
        while (1)
            ;
    }
}

// Must enter with interrupts disabled
void process_tick(void) {
    // Add current process to ready list
    current_proc->state = READY;
    p_list_push_back(&process_ready_list, current_proc);

    process_schedule();
}

int process_clone_files(struct process *dest, struct process *src) {
    int result = 0;
    for (size_t i = 0; i < MAX_OPEN_FILES; ++i) {
        if (src->open_files[i]) {
            dest->open_files[i] = file_open_file_clone(src->open_files[i]);
            if (!dest->open_files[i]) {
                // Failed to allocate open_file
                result = -1;
                // Free all open files that have been copied so far
                for (size_t j = 0; j < i; ++j) {
                    file_open_file_free(dest->open_files[j]);
                }
                break;
            }
        } else {
            dest->open_files[i] = NULL;
        }
    }
    return result;
}

uintptr_t sys_fork_helper(unsigned char child_cbr) __z88dk_fastcall __naked {
    __asm__("\
        di \n\
        \n\
        pop de \n\
        \n\
        in0 a, (_CBR) \n\
        out0 (_CBR), l \n\
        \n\
        ld hl, sys_fork_child_reentry \n\
        push de \n\
        push hl \n\
        push ix \n\
        push iy \n\
        \n\
        ld hl, 0 \n\
        add hl, sp \n\
        \n\
        out0 (_CBR), a \n\
        \n\
        pop bc \n\
        pop bc \n\
        pop bc \n\
        pop bc \n\
        \n\
        push de \n\
        \n\
sys_fork_child_reentry: \n\
        ei \n\
        ret \n\
        ");
    (void) child_cbr;
}

pid_t sys_fork(void) {
    struct process *child_proc = process_new();
    if (!child_proc)
        return -1;

    pid_t parent_pid = current_proc->pid;
    pid_t child_pid = child_proc->pid;

    // Setup child fields
    child_proc->ppid = parent_pid;
    file_file_ref(current_proc->cwd);
    child_proc->cwd = current_proc->cwd;
    if (process_clone_files(child_proc, current_proc) != 0) {
        process_destroy(child_proc);
        return -1;
    }

    dma_memcpy(pa_from_pfn(child_proc->cbr), pa_from_pfn(CBR), 0);
    // Bad things may happen if anything important on the stack changes between these lines
    child_proc->sp = sys_fork_helper(child_proc->cbr);

    if (current_proc->pid == parent_pid) {
        // Add child to ready list
        uint8_t int_state = cpu_get_int_state();
        intrinsic_di();
        child_proc->state = READY;
        p_list_push_back(&process_ready_list, child_proc);
        cpu_set_int_state(int_state);

        return child_pid;
    } else {
        return 0;
    }
}

pid_t sys_waitpid(pid_t pid, USER_PTR(int) wstatus, int options) {
    if (options & ~(WNOHANG))
        return -1;
    bool nohang = options & WNOHANG;

    pid_t current_pid = current_proc->pid;

    struct process *found_process = NULL;
    bool child_exists = false;

    uint8_t int_state = cpu_get_int_state();
    intrinsic_di();

    // Try finding matching child in process_zombie_list
    struct process *search_proc = p_list_front(&process_zombie_list);
    while (search_proc) {
        if (search_proc->ppid == current_pid && (pid == -1 || search_proc->pid == pid)) {
            child_exists = true;
            if (search_proc->state == ZOMBIE) {
                found_process = search_proc;
                p_list_remove(&process_zombie_list, found_process);
                break;
            } else {
                // TODO: Add panic()
                // panic();
            }
        }

        search_proc = p_list_next(search_proc);
    }
    
    if (!child_exists) {
        // Try finding any matching child in process_list
        struct created_process *search_created_proc = p_list_front(&process_list);
        while (search_created_proc) {
            if (search_created_proc->proc.ppid == current_pid && (pid == -1 || search_created_proc->proc.pid == pid)) {
                child_exists = true;
                break;
            }

            search_created_proc = p_list_next(search_created_proc);
        }
    }

    // Hang if no zombie child was found, but a matching child exists, and WNOHANG was not set
    if (!found_process && child_exists && !nohang) {
        current_proc->wait_pid = pid;
        current_proc->wait_options = options;
        current_proc->wait_process = NULL;

        // Add to list waiting
        current_proc->state = BLOCKED;
        p_list_push_back(&process_wait_list, current_proc);
        process_schedule();

        found_process = current_proc->wait_process;
    }

    cpu_set_int_state(int_state);

    if (found_process) {
        if (wstatus)
            dma_memcpy(pa_from_pfn(CBR) + wstatus, pa_from_pfn(BBR) + (uintptr_t) &found_process->status, sizeof(found_process->status));
        pid_t reaped_pid = found_process->pid;
        process_destroy(found_process);
        return reaped_pid;
    } else if (child_exists) {
        return 0;
    } else {
        return -1;
    }
}

void sys_exit(int status) {
    // Close all files
    for (size_t i = 0; i < MAX_OPEN_FILES; ++i) {
        sys_close(i);
    }

    intrinsic_di();

    file_file_unref(current_proc->cwd);
    current_proc->status = status & 0377;
    current_proc->state = ZOMBIE;

    // Make all children of this process take PPID 1
    struct created_process *search_created_proc = p_list_front(&process_list);
    while (search_created_proc) {
        if (search_created_proc->proc.ppid == current_proc->pid) {
            search_created_proc->proc.ppid = 1;
        }
        search_created_proc = p_list_next(search_created_proc);
    }

    // If a waiting process can reap this process, allow it to
    struct process *search_proc = p_list_front(&process_wait_list);
    while (search_proc) {
        if (search_proc->pid == current_proc->ppid) {
            // Process is the current processes's parent
            if (search_proc->wait_pid == -1 || search_proc->wait_pid == current_proc->pid) {
                // Process is waiting for the current process
                search_proc->wait_process = current_proc;
                p_list_remove(&process_wait_list, search_proc);
                process_switch(search_proc);
                // TODO: Add panic()
                // panic();    // Should never return to here
            }
        }
        search_proc = p_list_next(search_proc);
    }

    // Add to list of zombie processes and yield to other processes
    p_list_push_back(&process_zombie_list, current_proc); 
    process_schedule();
    // TODO: Add panic()
    // panic();    // Should never return to here
}

// ssize_t user_string_vector_space(USER_PTR(USER_PTR(char) const) vect) {


//     size_t argv_table_size = (argc + 1) * sizeof(char *);
// }

int sys_execve(USER_PTR(const char) pathname, USER_PTR(USER_PTR(char) const) argv, USER_PTR(USER_PTR(char) const) envp) {
    uintptr_t new_sp = 0xF000;

    uintptr_t tmp_ptr;

    unsigned int i;
    ssize_t string_len;

    // Get number of elements in vectors
    int argc = mem_user_vector_len((USER_PTR(void *)) argv);
    int envc = mem_user_vector_len((USER_PTR(void *)) envp);
    if (argc < 0 || envc < 0) {
        // Either table was invalid
        return -1;
    }

    // Copy argv to a dynamic buffer
    const size_t argv_size = (argc + 1) * sizeof(USER_PTR(char) const);
    USER_PTR(char) *dynamic_argv = malloc(argv_size);
    if (!dynamic_argv) {
        return -1;
    }
    mem_memcpy_kernel_from_user(dynamic_argv, argv, argv_size);

    // Verify argv pointers and accumulate string size
    size_t argv_strings_size = 0;
    for (i = 0; i < argc; ++i) {
        string_len = mem_user_strlen(dynamic_argv[i]);
        if (string_len < 0) {
            // Invalid pointer or string too long
            free(dynamic_argv);
            return -1;
        }
        // TODO: Ensure that we don't overflow argv_strings_size; possible by using overlapping strings
        argv_strings_size += string_len + 1;
    }

    // Copy envp to a dynamic buffer
    const size_t envp_size = (envc + 1) * sizeof(USER_PTR(char) const);
    USER_PTR(char) *dynamic_envp = malloc(envp_size);
    if (!dynamic_envp) {
        free(dynamic_argv);
        return -1;
    }
    mem_memcpy_kernel_from_user(dynamic_envp, envp, envp_size);

    // Verify envp pointers and accumulate string size
    size_t envp_strings_size = 0;
    for (i = 0; i < envc; ++i) {
        string_len = mem_user_strlen(dynamic_envp[i]);
        if (string_len < 0) {
            // Invalid pointer or string too long
            free(dynamic_argv);
            free(dynamic_envp);
            return -1;
        }
        // TODO: Ensure that we don't overflow envp_strings_size; possible by using overlapping strings
        envp_strings_size += string_len + 1;
    }

    // Setup offsets into vectors_buffer
    const size_t argv_offset = 0;
    const size_t envp_offset = argv_offset + (argc + 1) * sizeof(USER_PTR(char) const);
    const size_t argv_strings_offset = envp_offset + (envc + 1) * sizeof(USER_PTR(char) const);
    const size_t envp_strings_offset = argv_strings_offset + argv_strings_size;
    const size_t vectors_buffer_size = envp_strings_offset + envp_strings_size;

    // Adjust SP to fit vectors buffer
    new_sp -= vectors_buffer_size;

    // Allocate space for string vectors
    char *vectors_buffer = malloc(vectors_buffer_size);
    if (!vectors_buffer) {
        free(dynamic_argv);
        free(dynamic_envp);
        return -1;
    }

    // Create argv and envp table pointers
    USER_PTR(char) *argv_buffer = (USER_PTR(char) *) (vectors_buffer + argv_offset);
    USER_PTR(char) *envp_buffer = (USER_PTR(char) *) (vectors_buffer + envp_offset);

    // Copy strings to strings_buffer
    size_t strings_offset = 0;
    for (i = 0; i < argc; ++i) {
        string_len = mem_user_strlen(dynamic_argv[i]);
        if (string_len < 0) {
            // Invalid pointer or string too long
            free(dynamic_argv);
            free(dynamic_envp);
            free(vectors_buffer);
            return -1;
        }
        argv_buffer[i] = new_sp + argv_strings_offset + strings_offset;
        mem_memcpy_kernel_from_user(vectors_buffer + argv_strings_offset + strings_offset, dynamic_argv[i], string_len + 1);
        strings_offset += string_len + 1;
    }
    argv_buffer[argc] = (uintptr_t) NULL;
    strings_offset = 0;
    for (i = 0; i < envc; ++i) {
        string_len = mem_user_strlen(dynamic_envp[i]);
        if (string_len < 0) {
            // Invalid pointer or string too long
            free(dynamic_argv);
            free(dynamic_envp);
            free(vectors_buffer);
            return -1;
        }
        envp_buffer[i] = new_sp + envp_strings_offset + strings_offset;
        mem_memcpy_kernel_from_user(vectors_buffer + envp_strings_offset + strings_offset, dynamic_envp[i], string_len + 1);
        strings_offset += string_len + 1;
    }
    envp_buffer[envc] = (uintptr_t) NULL;

    // Free dynamic vector tables
    free(dynamic_argv);
    free(dynamic_envp);

    // From this point on, the existing process is destroyed; failure is not acceptable

    // Copy vectors buffer to user stack
    const uintptr_t vectors_buffer_base = new_sp;
    mem_memcpy_user_from_kernel(vectors_buffer_base, vectors_buffer, vectors_buffer_size);

    // Free vectors buffer
    free(vectors_buffer);

    // Push envp, argv, and argc
    tmp_ptr = vectors_buffer_base + envp_offset;
    new_sp -= sizeof(tmp_ptr);
    mem_memcpy_user_from_kernel(new_sp, &tmp_ptr, sizeof(tmp_ptr));
    tmp_ptr = vectors_buffer_base + argv_offset;
    new_sp -= sizeof(tmp_ptr);
    mem_memcpy_user_from_kernel(new_sp, &tmp_ptr, sizeof(tmp_ptr));
    new_sp -= sizeof(argc);
    mem_memcpy_user_from_kernel(new_sp, &argc, sizeof(argc));

    // Copy binary to run location
    int exec_fd = sys_open(pathname, 0);
    if (exec_fd < 0) {
        return -1;
    }
    // TODO: Check that binary and stack don't overlap
    sys_read(exec_fd, 0x1000, 0xE000);
    sys_close(exec_fd);

    // Address to start execution
    tmp_ptr = 0x1000;
    new_sp -= sizeof(tmp_ptr);
    mem_memcpy_user_from_kernel(new_sp, &tmp_ptr, sizeof(tmp_ptr));

    syscall_sp = new_sp;

    return 0;
}
