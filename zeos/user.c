#include <libc.h>

char buff[50];

int pid; int errno;
struct stats s;
char* buf = "something";

long inner(long n) {
  int i;
  long suma;
  suma = 0;
  for (i = 0; i < n; i++) suma += i;
  return suma;
}

void workload() {
  long acum;
  int i;
  acum = 0;
  for (i = 0; i < 10000; ++i) {
    acum = acum + inner(i);
  }
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
    /*int ret = write(1, "\n\nProva de write", strlen("\n\nProva de write"));
    if (ret < 0) perror();
    int p = getpid();
    if (p < 0) perror();
    itoa(p,buff);
    write(1,"\nProces amb PID ",strlen("\nProces amb PID "));
    write(1,buff, strlen(buff));*/
    //runjp();
    //runjp_rank(0,31);
    /*pid = fork();
    if (pid < 0) perror();
    if (pid == 0) {
      pid = getpid();
      itoa(pid, buff);
      write(1,"\nEl meu PID es ",strlen("\nEl meu PID es "));
      write(1,buff, strlen(buff));
      pid = fork();
      if (pid < 0) perror();
      if (pid == 0) {
        pid = getpid();
        itoa(pid, buff);
        write(1,"\nEl meu PID es ",strlen("\nEl meu PID es "));
        write(1,buff, strlen(buff));
        exit();
      } else {
        itoa(pid, buff);
        write(1,"\nEl PID del net es ",strlen("\nEl PID del net es "));
        write(1,buff, strlen(buff));
        exit();
      }
    } else {
      if (pid < 0) perror();
      workload();
      write(1, "\nelapsed: \n", strlen("\nelapsed: \n"));
      pid = get_stats(getpid(), &s);
      if (pid < 0) perror();
      itoa(s.elapsed_total_ticks, buff);
      write(1, buff, strlen(buff));
      write(1, "\nremaining: \n", strlen("\nremaining: \n"));
      itoa(s.remaining_ticks, buff);
      write(1, buff, strlen(buff));
      write(1, "\ntotal_trans: \n", strlen("\ntotal_trans: \n"));
      itoa(s.total_trans, buff);
      write(1, buff, strlen(buff));
      write(1, "\nuser_ticks: \n", strlen("\nuser_ticks: \n"));
      itoa(s.user_ticks, buff);
      write(1, buff, strlen(buff));
      itoa(pid, buff);
      write(1,"\nEl PID del meu fill es ",strlen("\nEl PID del meu fill es "));
      write(1,buff, strlen(buff));
      pid = getpid();
      itoa(pid, buff);
      write(1,"\nEl meu PID es ",strlen("\nEl meu PID es "));
      write(1,buff, strlen(buff));
    }*/
    pid = set_sched_policy(RR);
    if (pid < 0) perror();
    pid = fork();
    if (pid < 0) perror();
    if (pid == 0) {
      workload();
      int n = read(0, buf, 1500);
      if (n < 0) perror();
      else write(1, "\nchild Read OK\n", strlen("\nchild Read OK\n"));
      pid = get_stats(getpid(), &s);
      if (pid < 0) perror();
      write(1, "\nblocked ticks: ", strlen("\nblocked ticks: "));
      itoa(s.blocked_ticks, buff);
      write(1, buff, strlen(buff));
      exit();
    } else {
      int n = read(0, buf, 100);
      if (n < 0) perror();
      else write(1, "\nfather Read OK\n", strlen("\nfather Read OK\n"));
    }
    //exit();
  	while(1);
  	return 0;
}
