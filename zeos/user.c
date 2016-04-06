#include <libc.h>

char buff[24];

int pid; int errno; int i;

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
    //runjp_rank(30,31);
    int p = fork();
    if (p < 0) perror();
    if (p == 0) {
      p = getpid();
      itoa(p, buff);
      write(1,"\nEl meu PID es ",strlen("\nEl meu PID es "));
      write(1,buff, strlen(buff));
      p = fork();
      if (p < 0) perror();
      if (p == 0) {
        p = getpid();
        itoa(p, buff);
        write(1,"\nEl meu PID es ",strlen("\nEl meu PID es "));
        write(1,buff, strlen(buff));
      } else {
        itoa(p, buff);
        write(1,"\nEl PID del net es ",strlen("\nEl PID del net es "));
        write(1,buff, strlen(buff));
      }
      exit();
    } else {
      itoa(p, buff);
      write(1,"\nEl PID del meu fill es ",strlen("\nEl PID del meu fill es "));
      write(1,buff, strlen(buff));
      p = getpid();
      itoa(p, buff);
      write(1,"\nEl meu PID es ",strlen("\nEl meu PID es "));
      write(1,buff, strlen(buff));
    }
  	while(1);
  	return 0;
}
