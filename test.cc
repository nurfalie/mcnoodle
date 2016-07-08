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

  char *buffer = new char[16 * m.pSize() * m.pSize()];

  m.serializeP(buffer,  16 * m.pSize() * m.pSize());
  delete []buffer;
  buffer = new char[16 * m.pSize() * m.pSize()];
  m.serializePinv(buffer,  16 * m.pSize() * m.pSize());
  delete []buffer;
  buffer = new char[16 * m.sSize() * m.sSize()];
  m.serializeS(buffer,  16 * m.sSize() * m.sSize());
  delete []buffer;
  buffer = new char[16 * m.sSize() * m.sSize()];
  m.serializeSinv(buffer,  16 * m.sSize() * m.sSize());
  delete []buffer;
  return EXIT_SUCCESS;
}
