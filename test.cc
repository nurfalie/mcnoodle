extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

#include "mcnoodle.h"

int main(void)
{
  mcnoodle m(1269, 1632, 34);
  // mcnoodle m(644, 1024, 38);

  m.prepareP();
  m.prepareS();

  char *c = 0;
  char p[] = "Hello. This is a test.";
  size_t c_size = 0;

  m.encrypt(p, strlen(p), c, &c_size);
  delete []c;
  return EXIT_SUCCESS;
}
