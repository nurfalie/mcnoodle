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
  mcnoodle m(1269, 1632, 34);

  m.prepareG();
  m.prepareP();
  m.prepareS();
  m.prepareGcar();

  char p[] = "Hello. This is a friendly test.";
  std::stringstream cstream;
  std::stringstream pstream;

  rc &= m.encrypt(p, strlen(p), cstream);
  rc &= m.decrypt(cstream, pstream);
  std::cout << "After decryption: \"" << pstream.str() << "\".\n";
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
