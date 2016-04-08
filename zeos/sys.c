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

#include <stats.h>

#define LECTURA 0
#define ESCRIPTURA 1
#define buff_size 512
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
    char sysbuff[buff_size];
    while (ret < size) {
        int n = (buff_size > (size - ret)) ? size - ret: buff_size;
        int res = copy_from_user(buffer, sysbuff, n);
        if (res < 0) return ret;
        res = sys_write_console(sysbuff, n);
        ret += res;
    }
    return ret;
}

int ret_from_fork() {
  return 0;
}

int sys_fork() {
  int pid = (globalPID+=5);

  if (list_empty(&freequeue)) return -EAGAIN;
  struct list_head *lliure = list_first(&freequeue);
  list_del(lliure);

  struct task_struct *child_pcb = list_head_to_task_struct(lliure);
  struct task_struct *act = current();
  union task_union *child = (union task_union*)child_pcb;
  union task_union *father = (union task_union*)act;
  copy_data(father, child, sizeof(union task_union));

  child_pcb->PID = pid;
  child_pcb->quantum = act->quantum;
  init_stats(&child_pcb->proc_stats);
  child_pcb->estat = ST_READY;
  allocate_DIR(child_pcb);

  page_table_entry *child_PT = get_PT(child_pcb);
  int phys_page[NUM_PAG_DATA]; int i;
  for (i = 0; i < NUM_PAG_DATA; ++i) {
    phys_page[i] = alloc_frame();

    if (phys_page[i] < 0) {
      --i;
      for (; i >= 0; --i) {
        del_ss_pag(child_PT, i+NUM_PAG_KERNEL+NUM_PAG_CODE);
        free_frame(phys_page[i]);
      }
      list_add_tail(lliure, &freequeue);
      return -ENOMEM;
    } else {
      set_ss_pag(child_PT, i+NUM_PAG_KERNEL+NUM_PAG_CODE, phys_page[i]);
    }
  }
  
  page_table_entry *father_PT = get_PT(act);
  for (i = 0; i < NUM_PAG_KERNEL+NUM_PAG_CODE; ++i) {
    set_ss_pag(child_PT, i, get_frame(father_PT, i));
  }

  for (i = NUM_PAG_KERNEL+NUM_PAG_CODE; i < NUM_PAG_DATA+NUM_PAG_KERNEL+NUM_PAG_CODE; ++i) {
    set_ss_pag(father_PT, i+NUM_PAG_DATA, phys_page[i-NUM_PAG_KERNEL-NUM_PAG_CODE]);     //get_frame(child_PT, i)
    copy_data((void *)(i<<12), (void *)((i+NUM_PAG_DATA)<<12), PAGE_SIZE);
    del_ss_pag(father_PT, i+NUM_PAG_DATA);
  }
  set_cr3(get_DIR(act));

  int ebp;
  __asm__ __volatile__(
    "movl %%ebp, %0 \t\n"
    : "=g" (ebp)
    :
  );
  int child_ebp = (ebp - (int)act) + (int)child;

  *(unsigned long*)child_ebp = (unsigned long)&ret_from_fork;
  child_ebp -= sizeof(unsigned long);
  *(unsigned long*)child_ebp = 0;
  child_pcb->kernel_esp = child_ebp;



  list_add_tail(&(child_pcb->list), &readyqueue);
  return pid;
}

void sys_exit() {
  int i; page_table_entry *PT = get_PT(current());
  for (i = 0; i < NUM_PAG_DATA; ++i) {
    free_frame(get_frame(PT, i+NUM_PAG_KERNEL+NUM_PAG_CODE));
    del_ss_pag(PT, i+NUM_PAG_KERNEL+NUM_PAG_CODE);
  }

  list_add_tail(&(current()->list), &freequeue);

  sched_next_rr();
}

int sys_get_stats(int pid, struct stats *st) {
  if (pid < 1) return -EINVAL;
  if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT;

  int i;
  for (i = 0; i < NR_TASKS; i++) {
    if (task[i].task.PID == pid) {
      copy_to_user(&(task[i].task.proc_stats), st, sizeof(struct stats));
      return 0;
    }
  }
  return -ESRCH;
}
