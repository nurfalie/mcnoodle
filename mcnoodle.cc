#include "mcnoodle.h"

mcnoodle::mcnoodle(const size_t k, const size_t n, const size_t t)
{
  m_k = k;
  m_n = n;
  m_t = t;
  prepareS();
}

mcnoodle::~mcnoodle()
{
}

void mcnoodle::prepareS(void)
{
  S.resize(m_k, m_k);
}
