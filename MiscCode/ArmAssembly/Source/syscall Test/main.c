/* Idea is to create a simple program that opens a file using our ARM sys call
*/
#include "syscal.h"
int main()
{
  char theFile[] = "./test.txt";
  int mode = 0;
  int final = 0;
  int smelly = dynamorio_syscall(5,3,theFile, mode, final);
  printf("Value of file is: %d\n", smelly);
  //dynamorio_syscall(4, 3, 1, theFile,10);
  return 0;
}
