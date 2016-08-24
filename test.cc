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
  mcnoodle m(11, 115);

  rc = m.generatePrivatePublicKeys();

  char plaintext[] = "The test is empty.";
  std::stringstream c;
  std::stringstream p;

  rc &= m.encrypt(plaintext, strlen(plaintext), c);
  rc &= m.decrypt(c, p);

  if(rc &= (p.str() == std::string(plaintext)))
    std::cout << "p equals plaintext!" << std::endl;
  else
    std::cout << "p does not equal plaintext!" << std::endl;

  return rc;
}

int test2(void)
{
  int rc = 1;
  mcnoodle m(12, 200);

  rc = m.generatePrivatePublicKeys();

  char plaintext[] = "A longer sentence. Some things are fine.";
  std::stringstream c;
  std::stringstream p;

  rc &= m.encrypt(plaintext, strlen(plaintext), c);
  rc &= m.decrypt(c, p);

  if(rc &= (p.str() == std::string(plaintext)))
    std::cout << "p equals plaintext!" << std::endl;
  else
    std::cout << "p does not equal plaintext!" << std::endl;

  return rc;
}

int main(void)
{
  int rc = 1;

  rc &= test1();
  rc &= test2();
  return !rc;
}
