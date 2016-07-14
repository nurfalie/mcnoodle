extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

#include "mcnoodle.h"

int main(void)
{
  // mcnoodle m(1269, 1632, 34);
  mcnoodle m(644, 1024, 38);

  m.prepareP();

  char *c = 0;
  size_t c_size = 0;

  m.serialize(c, &c_size, m.P());

  boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> matrix;

  m.deserialize(c, c_size, matrix);

  char *d = 0;
  size_t d_size = 0;

  m.serialize(d, &d_size, matrix);

  if(c && d && c_size == d_size && memcmp(c, d, c_size) == 0)
    std::cout << "P equals matrix!" << std::endl;
  else
    std::cout << "P does not equal matrix!" << std::endl;

  if(c)
    delete []c;

  if(d)
    delete []d;

  c = 0;
  d = 0;
  m.prepareS();

  char p[] = "Hello. This is a test.";

  m.encrypt(p, strlen(p), c, &c_size);

  if(c)
    delete []c;

  return EXIT_SUCCESS;
}
