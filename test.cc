extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

#include "mcnoodle.h"

int test2(void)
{
  int rc = 0;

  std::cout << "Testing matrix equality... ";

  mcnoodle m(100, 100, 38);

  m.prepareP();

  char *c = 0;
  size_t c_size = 0;

  m.serialize(c, &c_size, m.P());

  boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> matrix;

  m.deserialize(c, c_size, matrix);

  if((rc = mcnoodle::equal(m.P(), matrix)))
    std::cout << "P equals matrix!" << std::endl;
  else
    std::cout << "P does not equal matrix!" << std::endl;

  delete []c;
  return rc;
}

int main(void)
{
  // mcnoodle m(1269, 1632, 34);
  // mcnoodle m(644, 1024, 38);

  int rc = 0;

  rc |= !test2();
  return rc;
}
