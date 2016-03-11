/*
 * sched.c - initializes struct for task 0 and task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

/**
 * Container for the Task array and 2 additional pages (the first and the last one)
 * to protect against out of bound accesses.
 */
union task_union protected_tasks[NR_TASKS+2]
  __attribute__((__section__(".data.task")));

union task_union *task = &protected_tasks[1]; /* == union task_union task[NR_TASKS] */


struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}

struct task_struct *idle_task;
extern struct list_head blocked;
struct list_head freequeue;
struct list_head readyqueue;

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	;
	}
}

void init_idle (void) {
	struct list_head *idle = list_first(&freequeue);
	list_del(&freequeue);
	struct task_struct *pcb = list_head_to_task_struct(idle);
	union task_union *aux = (union task_union*)pcb;
	pcb->PID = 0;
	allocate_DIR(pcb);
	unsigned long *stack = (unsigned long *)aux + 0x1000;
	--stack;
	*stack = (unsigned long *)cpu_idle;
	--stack;
	*stack = 0;
	pcb->kernel_esp = stack;
	idle_task = pcb;
}

void init_task1(void)
{
}


void init_sched(){
	INIT_LIST_HEAD(&freequeue);
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		list_add(&(task[i].task.list), &freequeue);
	}
	INIT_LIST_HEAD(&readyqueue);
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

