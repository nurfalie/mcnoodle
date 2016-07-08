extern "C"
{
#include <inttypes.h>
}

#include <boost/nondet_random.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/random.hpp>
#include <map>

#include "mcnoodle.h"

mcnoodle::mcnoodle(const std::size_t k,
		   const std::size_t n,
		   const std::size_t t)
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
  boost::numeric::ublas::matrix<float> P
    (m_n, m_n, 0); /*
		   ** BOOST type-checking may
		   ** raise an exception unless
		   ** we're using real numbers.
		   */
  boost::random::uniform_int_distribution<uint64_t> distribution;
  boost::random_device random_device;
  std::map<std::size_t, char> indexes;

  /*
  ** 0 ... 1 ... 0 ... 0 ...
  ** 1 ... 0 ... 0 ... 0 ...
  ** 0 ... 0 ... 1 ... 0 ...
  ** 0 ... 0 ... 0 ... 0 ...
  ** 0 ... 0 ... 0 ... 1 ...
  */

  for(std::size_t i = 0; i < P.size1(); i++)
    {
      do
	{
	  std::size_t j = distribution(random_device) % P.size2();

	  if(indexes.find(j) == indexes.end())
	    {
	      P(i, j) = 1;
	      indexes[j] = 0;
	      break;
	    }
	}
      while(true);
    }

  /*
  ** A permutation matrix always has an inverse.
  */

  /*
  ** (PP^T)ij = Sum(Pik(P^T)kj, k = 1..n) = Sum(PikPjk, k = 1..n).
  ** Sum(PikPjk, k = 1..n) = 1 if i = j, and 0 otherwise (I).
  */

  m_P = P;
  m_Pinv = boost::numeric::ublas::trans(m_P);
}

void mcnoodle::prepareS(void)
{
  boost::numeric::ublas::matrix<float> S; /*
					  ** BOOST type-checking may
					  ** raise an exception unless
					  ** we're using real numbers.
					  */
  boost::random::uniform_int_distribution<uint64_t> distribution;
  boost::random_device random_device;

  S.resize(m_k, m_k);

  for(std::size_t i = 0; i < S.size1(); i++)
    for(std::size_t j = 0; j < S.size2(); j++)
      S(i, j) = static_cast<float> (distribution(random_device) % 2);

  m_S = S;
}
