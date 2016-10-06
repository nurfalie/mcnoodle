/*
** Miss M.
*/

extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

#include <ctime>

#include "mcnoodle.h"

int test1(void)
{
  int rc = 1;
  mcnoodle m(11, 51);

  rc = m.generatePrivatePublicKeys();

  char plaintext[] = "The encoding of calendar time in std::time_t "
    "is unspecified, but most systems conform to POSIX specification.";

  std::stringstream c;
  std::stringstream p;

  std::cout << "encrypt() before: "
	    << std::time(0) << "." << std::endl;
  rc &= m.encrypt(plaintext, strlen(plaintext), c);
  std::cout << "decrypt() before: "
	    << std::time(0) << "." << std::endl;
  rc &= m.decrypt(c, p);
  std::cout << "decrypt() after: "
	    << std::time(0) << "." << std::endl;

  if(rc &= (p.str() == std::string(plaintext)))
    std::cout << "p equals plaintext!" << std::endl;
  else
    std::cout << "p does not equal plaintext!" << std::endl;

  return rc;
}

int test2(void)
{
  int rc = 1;
  mcnoodle m(11, 51);

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

  std::cout << "NTL version " << NTL_VERSION << "." << std::endl;
  rc &= test1();
  rc &= test2();
  return !rc;
}
