#include "armFunctions.h"

#define Z_FLAG	0x40000000
int main()
{
  int someValue = 1;
  unsigned int c;
  printf("Starting atomic_dec_and_test\n");
  printf("VAR = %x\n", someValue);
  ATOMIC_DEC_int(&someValue);
  //printf("VAR = %x\n", someValue);
  c = SET_FLAG(Z_FLAG);
  printf("Set Flag Status %x \n", c);
  return 0;
}
