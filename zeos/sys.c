/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1
int zeos_ticks;

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -EACCES; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -ENOSYS; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int sys_gettime() {
    return zeos_ticks;
}

int sys_write(int fd, char* buffer, int size) {
    int ret = check_fd(fd, ESCRIPTURA);
    if (ret < 0) return ret;
    if (buffer == NULL) return -EFAULT;
    if (size < 0) return -EINVAL;

    ret = 0;
    char sysbuff[512];
    while (ret < size) {
        int n = (512 > (size - ret)) ? size - ret: 512;
        ret = copy_from_user(buffer, sysbuff, n);
        if (ret < 0) return ret;
        ret = sys_write_console(sysbuff, n);
        ret += n;
    }
    return ret;
}

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
  return PID;
}

void sys_exit()
{  
}
