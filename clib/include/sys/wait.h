#ifndef __SYS_WAIT_H__
#define __SYS_WAIT_H__

#define WCONTINUED ((int) 0x02)
#define WNOHANG ((int) 0x01)
#define WUNTRACED ((int) 0x04)

#define WEXITED ((int) 0x08)
#define WNOWAIT ((int) 0x10)
#define WSTOPPED ((int) 0x20)

enum idtype_t {
    P_ALL,
    P_PGID,
    P_PID
};

#ifndef _ID_T_DEFINED
#define _ID_T_DEFINED
typedef signed int id_t;
#endif

#ifndef _PID_T_DEFINED
#define _PID_T_DEFINED
typedef signed int pid_t;
#endif

pid_t wait(int *wstatus);
pid_t waitpid(pid_t pid, int *wstatus, int options);

#endif
