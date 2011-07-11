// Simple Hello World Application
#include <stdio.h>
#include <math.h>
#include "TutorialConfig.h"

#ifdef USE_ALTERNATIVE
#include "newOutput.h"
#endif

int main (int argc, char *argv[])
{
  printf("Hello World\n");
  printf("This is version %d.%d\n", Tutorial_VERSION_MAJOR, Tutorial_VERSION_MINOR);

#ifdef USE_ALTERNATIVE
  alternateOutput();
#else
  printf("This is the original and best output!!\n");
#endif

  return 0;
}

