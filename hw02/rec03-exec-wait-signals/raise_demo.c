#include <signal.h>
#include <stdio.h>

int main()
{
  puts("Going to sleep");
  raise(SIGSTOP);
  puts("Running again");
  return 0;
}
