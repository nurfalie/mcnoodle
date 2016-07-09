extern "C"
{
#include <inttypes.h>
}

#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
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
  m_P.resize(m_n, m_n);
  m_Pinv.resize(m_n, m_n);
  m_S.resize(m_k, m_k);
  m_Sinv.resize(m_k, m_k);
}

mcnoodle::mcnoodle
(const boost::numeric::ublas::matrix<mcnoodble_matrix_element_type_t> &Gcar,
 const std::size_t t)
{
  m_Gcar = Gcar;
  m_t = t;
}

mcnoodle::~mcnoodle()
{
}

void mcnoodle::prepareP(void)
{
  /*
  ** Generate an n x n random permutation matrix and its inverse.
  */

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
  ** ...
  */

  for(std::size_t i = 0; i < P.size1(); i++)
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
  /*
  ** Generate a random k x k binary non-singular matrix and its inverse.
  */

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

void mcnoodle::serialize
(char *buffer,
 const size_t buffer_size,
 const boost::numeric::ublas::matrix<mcnoodble_matrix_element_type_t> &m)
{
  if(!buffer || buffer_size <= 0)
    return;

  boost::iostreams::array_sink sink(buffer, buffer_size);
  boost::iostreams::stream<boost::iostreams::array_sink> source(sink);
  boost::archive::binary_oarchive archive(source);

  archive << m;
}

void mcnoodle::serializeP(char *buffer, const size_t buffer_size)
{
  serialize(buffer, buffer_size, m_P);
}

void mcnoodle::serializePinv(char *buffer, const size_t buffer_size)
{
  serialize(buffer, buffer_size, m_Pinv);
}

void mcnoodle::serializeS(char *buffer, const size_t buffer_size)
{
  serialize(buffer, buffer_size, m_S);
}

void mcnoodle::serializeSinv(char *buffer, const size_t buffer_size)
{
  serialize(buffer, buffer_size, m_Sinv);
}
