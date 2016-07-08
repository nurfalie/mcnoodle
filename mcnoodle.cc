extern "C"
{
#include <inttypes.h>
}

#include <boost/nondet_random.hpp>
#include <boost/random.hpp>

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
  boost::random::uniform_int_distribution<uint64_t> distribution;
  boost::random_device random_device;

  S.resize(m_k, m_k);

  for(size_t i = 0; i < S.size1(); i++)
    for(size_t j = 0; j < S.size2(); j++)
      S(i, j) = distribution(random_device) % 2;
}
