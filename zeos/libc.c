/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

#include <errno.h>

int errno;

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

int get_stats(int pid, struct stats *st) {
  int ret;

  __asm__ __volatile__(
    "int $0x80 \t\n"
    : "=g" (ret)
    : "b" (pid), "c" (st), "a" (35)
  );

  if (ret < 0) {
    errno = -ret;
    ret = -1; 
  }

  return ret;
}

void exit() {

  __asm__ __volatile__(
    "int $0x80 \t\n"
    :
    : "a" (1)
  );

}

int fork() {
  int ret;

  __asm__ __volatile__(
    "int $0x80 \t\n"
    : "=g" (ret)
    : "a" (2)
  );

  if (ret < 0) {
    errno = -ret;
    ret = -1; 
  }

  return ret;
}

int getpid() {
  int ret;

  __asm__ __volatile__(
    "int $0x80 \t\n"
    : "=g" (ret)
    : "a" (20)
  );

  if (ret < 0) {
    errno = -ret;
    ret = -1; 
  }

  return ret;
}

int gettime() {
  int ret;

  __asm__ __volatile__(
    "int $0x80 \t\n"
    : "=g" (ret)
    : "a" (10)
  );

  if (ret < 0) {
    errno = -ret;
    ret = -1; 
  }

  return ret;
}

int write(int fd, char* buffer, int size) {
  int ret;

  __asm__ __volatile__(
    "int $0x80 \t\n"
    : "=g" (ret)
    : "a" (4), "b" (fd), "c" (buffer), "d" (size)
  );

  if (ret < 0) {
    errno = -ret;
    ret = -1; 
  }

  return ret;
}

void perror() {
  switch (errno) {
    case ENOSYS:
      write(1, "Function not implemented", strlen("Function not implemented"));
    break;

    case EACCES:
      write(1, "Permission denied", strlen("Permission denied"));
    break;

    case EAGAIN:
      write(1, "No PCB available, try later", strlen("No PCB available, try later"));
    break;

    case ENOMEM:
      write(1, "Out of memory", strlen("Out of memory"));
    break;

    case EBADF:
      write(1, "Bad file descriptor", strlen("Bad file descriptor"));
    break;

    case EFAULT:
      write(1, "Bad address", strlen("Bad address"));
    break;

    case EINVAL:
      write(1, "Invalid argument", strlen("Invalid argument"));
    break;
    
    default:
      write(1, "Unknown error", strlen("Unknown error"));
    break;
  }
}
