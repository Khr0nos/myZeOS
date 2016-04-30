#include <libc.h>

char buff[50];

int pid;
char* buf = "something";

void print_stats(int PID) {
  struct stats s;
  int r = get_stats(PID, &s);
  if (r < 0) perror();
  write(1, "\nProces amb pid: ", strlen("\nProces amb pid: "));
  itoa(PID, buff);
  write(1, buff, strlen(buff));
  write(1, "\nsystem ticks: ", strlen("\nsystem ticks: "));
  itoa(s.system_ticks, buff);
  write(1, buff, strlen(buff));
  write(1, "\nblocked ticks: ", strlen("\nblocked ticks: "));
  itoa(s.blocked_ticks, buff);
  write(1, buff, strlen(buff));
  write(1, "\nready -> run: ", strlen("\nready -> run: "));
  itoa(s.total_trans, buff);
  write(1, buff, strlen(buff));
  write(1, "\nuser ticks: ", strlen("\nuser ticks: "));
  itoa(s.user_ticks, buff);
  write(1, buff, strlen(buff));
  write(1, "\nready ticks: ", strlen("\nready ticks: "));
  itoa(s.ready_ticks, buff);
  write(1, buff, strlen(buff));
}

int factorial(int n) {
  if (n <= 1) return 1;
  return n*factorial(n-1);
}

int workload_CPU() {  //per a obtenir stats mesurables cal fer calculs aburdament grans (raonablement!)
  int n, i;
  for (i = 0; i < 10000; ++i) {
    n = factorial(10000);
  }
  return n;
}

int workload_forks() {
  int i;
  for (i = 0; i < 10000; ++i) {
    int pid = fork();
    //if (pid < 0) perror();
    if (pid == 0) exit();
  }
  return i;
}

int workload_blocks() {
  //workload bloquejant
  int i;
  for (i = 0; i < 10; ++i) {
    int n = read(0, buf, 2000);
    if (n < 0) perror();
    else write(1, "\nOK", strlen("\nOK"));
  }
  return i;
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
    pid = set_sched_policy(RR);
    if (pid < 0) perror();

    pid = fork();
    if (pid < 0) perror();
    if (pid == 0) {
      pid = fork();
      if (pid == 0) {
        int f = workload_CPU(); //quan es provi un workload forçar utilització de resultats
        itoa(f, buff);          //sino per la optimitzacio (-O2) el workload es com si no es fes
        write(1, "\n", strlen("\n"));
        write(1, buff, strlen(buff));
        write(1, "\n", strlen("\n"));
        print_stats(getpid());
        exit();
      } else {
        int f = workload_CPU();
        itoa(f, buff);
        write(1, buff, strlen(buff));
        write(1, "\n", strlen("\n"));
        print_stats(getpid());
        exit();
      }
    }

    exit();
  	while(1);
  	return 0;
}
