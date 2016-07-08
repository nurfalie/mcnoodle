extern "C"
{
#include <stdio.h>
#include <stdlib.h>
}

#include "mcnoodle.h"

int main(void)
{
  // mcnoodle m(2048, 2804, 66);
  mcnoodle m(524, 1024, 50);

  m.prepareP();
  m.prepareS();
  return EXIT_SUCCESS;
}
