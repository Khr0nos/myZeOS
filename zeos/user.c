#include <libc.h>

char buff[24];

int pid; int errno;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
    int ret = write(1, "\n\nProva de write", strlen("\n\nProva de write"));
    if (ret < 0) perror();
    //runjp();
  	while(1);
  	return 0;
}
