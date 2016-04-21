/*
 * sched.c - initializes struct for task 0 and task 1
 */

#include <sched.h>
#include <schedperf.h>
#include <mm.h>
#include <io.h>
#include <utils.h>

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
int globalPID = 15;
int current_quantum;

void init_stats(struct stats *st) {
	st->user_ticks = 0;
	st->system_ticks = 0;
	st->blocked_ticks = 0;
	st->ready_ticks = 0;
	st->elapsed_total_ticks = get_ticks();
	st->total_trans = 0;
	st->remaining_ticks = QUANT;
}

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
	list_del(idle);
	struct task_struct *pcb = list_head_to_task_struct(idle);
	union task_union *aux = (union task_union*)pcb;
	pcb->PID = 0;
	pcb->quantum = QUANT;
	init_stats(&pcb->proc_stats);
	allocate_DIR(pcb);
	aux->stack[KERNEL_STACK_SIZE-1] = (unsigned long)&cpu_idle;
	aux->stack[KERNEL_STACK_SIZE-2] = 0;
	pcb->kernel_esp = (int)&(aux->stack[KERNEL_STACK_SIZE-2]);
	/*unsigned long *stack = (unsigned long *)aux + 0x1000;
	--stack;
	*stack = (unsigned long)&cpu_idle;
	--stack;
	*stack = 0;
	pcb->kernel_esp = stack;*/
	idle_task = pcb;
}

void init_task1(void) {
	struct list_head *init = list_first(&freequeue);
	list_del(init);
	struct task_struct *pcb = list_head_to_task_struct(init);
	union task_union *aux = (union task_union*)pcb;
	pcb->PID = 1;
	pcb->quantum = QUANT;
	current_quantum = QUANT;
	init_stats(&pcb->proc_stats);
	pcb->estat = ST_RUN;
	allocate_DIR(pcb);
	set_user_pages(pcb);
	pcb->kernel_esp = (int)&(aux->stack[KERNEL_STACK_SIZE]);
	tss.esp0 = (unsigned long)&(aux->stack[KERNEL_STACK_SIZE]);
	set_cr3(pcb->dir_pages_baseAddr);
}


void init_sched(){
	init_sched_policy();
	INIT_LIST_HEAD(&freequeue);
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		task[i].task.PID = -1;
		list_add_tail(&(task[i].task.list), &freequeue);
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

void task_switch(union task_union*t) {
	__asm__ __volatile__(
		"pushl %esi \t\n"
		"pushl %edi \t\n"
		"pushl %ebx \t\n"
	);
	inner_task_switch(t);
	__asm__ __volatile__(
		"popl %ebx \t\n"
		"popl %edi \t\n"
		"popl %esi \t\n"
	);
}

void inner_task_switch(union task_union*t) {
	page_table_entry *nou_DIR = get_DIR(&t->task);
	tss.esp0 = (unsigned long)&(t->stack[KERNEL_STACK_SIZE]);
	set_cr3(nou_DIR);

	__asm__ __volatile__(
		"movl %%ebp, %0 \t\n"
		: "=g" (current()->kernel_esp)
		: /* no input */
	);

	__asm__ __volatile__(
		"movl %0, %%esp \t\n"
		: /* no output */
		: "g" (t->task.kernel_esp)
	);

	__asm__ __volatile__(
		"popl %%ebp \t\n"
		"ret \t\n"
		:
		:
	);
}

int get_quantum(struct task_struct *t) {
	return t->quantum;
}

void set_quantum(struct task_struct *t, int new_quantum) {
	t->quantum = new_quantum;
}

struct stats* get_task_stats(struct task_struct *t) {
	return &(t->proc_stats);
}

struct list_head* get_task_list(struct task_struct *t) {
	return &(t->list);
}

void block_process(struct list_head *block_queue) {
	struct task_struct *t = current();
	struct stats *st = get_task_stats(t);

	update_process_state(t, block_queue);
	st->system_ticks = (get_ticks() - st->elapsed_total_ticks);
	st->elapsed_total_ticks = get_ticks();
	sched_next();
}

void unblock_process(struct task_struct *blocked) {
	struct stats *st = get_task_stats(blocked);
	//struct list_head *l = get_task_list(blocked);

	update_process_state(blocked, &readyqueue);
	st->blocked_ticks += (get_ticks() - st->elapsed_total_ticks);
	st->elapsed_total_ticks = get_ticks();
	if (needs_sched()) {
		update_process_state(current(), &readyqueue);
		sched_next();
	}
}

void schedule() {
	update_sched_data();
	if (needs_sched()) {
		update_process_state(current(), &readyqueue);
		sched_next();
	}
}

void sched_next_rr() {
	struct task_struct *next;
	if (list_empty(&readyqueue)) next = idle_task;
	else {
		struct list_head *n = list_first(&readyqueue);
		list_del(n);
		next = list_head_to_task_struct(n);
	}
	next->estat = ST_RUN;
	current_quantum = get_quantum(next);
	unsigned long now = get_ticks();
	next->proc_stats.ready_ticks += (now - next->proc_stats.elapsed_total_ticks);
	next->proc_stats.elapsed_total_ticks = now;
	next->proc_stats.total_trans += 1;
	next->proc_stats.remaining_ticks = current_quantum;

	task_switch((union task_union*)next);
}

void update_process_state_rr(struct task_struct *t, struct list_head *dest) {
	if (t->estat != ST_RUN) list_del(&(t->list));
	if (dest != NULL) {
		list_add_tail(&(t->list), dest);
		t->estat = ST_READY;
		unsigned long now = get_ticks();
		t->proc_stats.system_ticks += (now - t->proc_stats.elapsed_total_ticks);
		t->proc_stats.elapsed_total_ticks = now;
	}
	else t->estat = ST_RUN;
}

int needs_sched_rr() {
	if (current_quantum == 0 && !list_empty(&readyqueue)) return 1;
	if (current_quantum == 0) {
		current_quantum = get_quantum(current());
		current()->proc_stats.remaining_ticks = current_quantum;
	}
	return 0;
}

void update_sched_data_rr() {
	--current_quantum;
	current()->proc_stats.remaining_ticks -= 1;
}

void user_to_system() {
	struct task_struct *t = current();
	unsigned long now = get_ticks();
	t->proc_stats.user_ticks += (now - t->proc_stats.elapsed_total_ticks);
	t->proc_stats.elapsed_total_ticks = now;
}

void system_to_user() {
	struct task_struct *t = current();
	unsigned long now = get_ticks();
	t->proc_stats.system_ticks += (now - t->proc_stats.elapsed_total_ticks);
	t->proc_stats.elapsed_total_ticks = now;
}