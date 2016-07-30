extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

#include "mcnoodle.h"

int test1(void)
{
  int rc = 1;
  mcnoodle m(13, 115);

  m.generateKeyPair();
  return rc;
}

int test2(void)
{
  int rc = 1;

  return rc;
}

int main(void)
{
  int rc = 1;

  rc &= test1();
  rc &= test2();
  return !rc;
}
