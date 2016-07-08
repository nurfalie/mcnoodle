extern "C"
{
#include <inttypes.h>
}

#include <boost/nondet_random.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/random.hpp>

#include "mcnoodle.h"

mcnoodle::mcnoodle(const size_t k, const size_t n, const size_t t)
{
  m_k = k;
  m_n = n;
  m_t = t;
}

mcnoodle::~mcnoodle()
{
}

void mcnoodle::prepareP(void)
{
  m_P.resize(m_n, m_n);

  /*
  ** A permutation matrix always has an inverse.
  */

  boost::numeric::ublas::matrix<float> mLu(m_P);
  boost::numeric::ublas::permutation_matrix<size_t> pivots(m_P.size1());
  boost::numeric::ublas::lu_factorize(mLu, pivots);
}

void mcnoodle::prepareS(void)
{
  boost::random::uniform_int_distribution<uint64_t> distribution;
  boost::random_device random_device;

  m_S.resize(m_k, m_k);

  for(size_t i = 0; i < m_S.size1(); i++)
    for(size_t j = 0; j < m_S.size2(); j++)
      m_S(i, j) = static_cast<float> (distribution(random_device) % 2);
}
