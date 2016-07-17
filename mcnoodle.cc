extern "C"
{
#include <inttypes.h>
}

#include <bitset>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/nondet_random.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/random.hpp>
#include <map>

#include "mcnoodle.h"

mcnoodle::mcnoodle(const size_t k,
		   const size_t n,
		   const size_t t)
{
  /*
  ** For generating the private and public keys.
  */

  m_k = minimumK(k);
  m_n = minimumN(n);
  m_t = minimumT(t);

  try
    {
      m_G.resize(m_k, m_n);
      m_Gcar.resize(m_k, m_n);
      m_P.resize(m_n, m_n);
      m_Pinv.resize(m_P.size1(), m_P.size2());
      m_S.resize(m_k, m_k);
      m_Sinv.resize(m_S.size1(), m_S.size2());
    }
  catch(...)
    {
    }
}

mcnoodle::mcnoodle
(const boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> &Gcar,
 const size_t k,
 const size_t n,
 const size_t t)
{
  /*
  ** For public-key encryption.
  */

  m_k = minimumK(k);
  m_n = minimumN(n);
  m_t = minimumT(t);

  try
    {
      m_Gcar = Gcar;
    }
  catch(...)
    {
    }
}

mcnoodle::~mcnoodle()
{
}

bool mcnoodle::decrypt(const char *ciphertext, const size_t ciphertext_size,
		       char *&plaintext, size_t *plaintext_size)
{
  if(!ciphertext || ciphertext_size <= 0 || plaintext || !plaintext_size)
    return false;

  /*
  ** Store the ciphertext into an 1-by-m_n vector.
  */

  boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> c(1, m_n);
  char *buffer = new char[ciphertext_size];

  if(!deserialize(buffer, ciphertext_size, c))
    {
      delete []buffer;
      return false;
    }

  delete []buffer;
  return true;
}

bool mcnoodle::deserialize
(const char *buffer, const size_t buffer_size,
 boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> &m)
{
  if(!buffer || buffer_size <= 0)
    return false;

  try
    {
      boost::iostreams::array_source source(buffer, buffer_size);
      boost::iostreams::stream<boost::iostreams::array_source> stream(source);
      boost::archive::binary_iarchive archive(stream);

      archive >> m;
    }
  catch(...)
    {
      return false;
    }

  return true;
}

bool mcnoodle::encrypt(const char *plaintext, const size_t plaintext_size,
		       char *&ciphertext, size_t *ciphertext_size)
{
  if(ciphertext || !ciphertext_size || !plaintext || plaintext_size <= 0)
    return false;

  if(CHAR_BIT * plaintext_size > m_k)
    return false;

  try
    {
      /*
      ** Represent the message as a binary vector of length k.
      */

      boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> m
	(1, m_k, 0);

      for(size_t i = 0, k = 0; i < plaintext_size; i++)
	{
	  std::bitset<CHAR_BIT> b(plaintext[i]);

	  for(size_t j = 0; j < b.size(); j++, k++)
	    m(0, k) = b[j];
	}

#ifdef MCNOODLE_ARTIFICIAL_GENERATOR
      /*
      ** This will allow us to prove that decryption and encryption
      ** are correct without a generator G matrix.
      */

      boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> c(1, m_n);

      c = boost::numeric::ublas::prod(m, m_Gcar);
#else
      /*
      ** Generate a random binary vector of length n having at most t 1s.
      */

      boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> z
	(1, m_n, 0);
      boost::random::uniform_int_distribution<uint64_t> distribution;
      boost::random_device random_device;
      size_t ts = 0;

      for(size_t i = 0; i < z.size2(); i++)
	{
	  mcnoodle_matrix_element_type_t a = distribution(random_device) % 2;

	  if(a == 1)
	    {
	      ts += 1;
	      z(0, i) = 1;

	      if(m_t == ts)
		break;
	    }
	}

      /*
      ** Compute c = mGcar + z, an 1-by-m_n vector.
      */

      boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> c(1, m_n);

      c = boost::numeric::ublas::prod(m, m_Gcar) + z;
#endif

      /*
      ** Place c into ciphertext. The user is responsible for restoring memory.
      */

      return serialize(ciphertext, ciphertext_size, c);
    }
  catch(...)
    {
      delete []ciphertext;
      ciphertext = 0;
      *ciphertext_size = 0;
      return false;
    }
}

bool mcnoodle::equal
(const boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> &m1,
 const boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> &m2)
{
  if(m1.size1() != m2.size1() || m1.size2() != m2.size2())
    return false;

  for(size_t i = 0; i < m1.size1(); i++)
    for(size_t j = 0; j < m1.size2(); j++)
      if(m1(i, j) != m2(i, j))
	return false;

  return true;
}

bool mcnoodle::serialize
(char *&buffer,
 size_t *buffer_size,
 const boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> &m)
{
  if(buffer || !buffer_size)
    return false;

  /*
  ** We'd like support char types (sizeof(...) + 1).
  */

  *buffer_size = (sizeof(mcnoodle_matrix_element_type_t) + 1) *
    (sizeof(mcnoodle_matrix_element_type_t) + 1) * m.size1() * m.size2();

  if(*buffer_size == 0) // Possible?
    return false;

  try
    {
      buffer = new char[*buffer_size];
      memset(buffer, 0, *buffer_size);

      boost::iostreams::array_sink sink(buffer, *buffer_size);
      boost::iostreams::stream<boost::iostreams::array_sink> stream(sink);
      boost::archive::binary_oarchive archive(stream);

      archive << m;
    }
  catch(...)
    {
      delete []buffer;
      buffer = 0;
      *buffer_size = 0;
      return false;
    }

  return true;
}

bool mcnoodle::prepareG(void)
{
  try
    {
#ifdef MCNOODLE_ARTIFICIAL_GENERATOR
      for(size_t i = 0; i < m_G.size1(); i++)
	for(size_t j = 0; j < m_G.size2(); j++)
	  m_G(i, j) = 1;
#endif
    }
  catch(...)
    {
      return false;
    }

  return true;
}

bool mcnoodle::prepareGcar(void)
{
  try
    {
      m_Gcar = boost::numeric::ublas::prod(m_S, m_G);
      m_Gcar = boost::numeric::ublas::prod(m_Gcar, m_P);
    }
  catch(...)
    {
      return false;
    }

  return true;
}

bool mcnoodle::prepareP(void)
{
  try
    {
      /*
      ** Generate an n x n random permutation matrix and discover its inverse.
      */

      boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> P_
	(m_n, m_n, 0);
      boost::random::uniform_int_distribution<uint64_t> distribution;
      boost::random_device random_device;
      std::map<size_t, char> indexes;

      /*
      ** 0 ... 1 ... 0 ... 0 ...
      ** 1 ... 0 ... 0 ... 0 ...
      ** 0 ... 0 ... 1 ... 0 ...
      ** 0 ... 0 ... 0 ... 0 ...
      ** 0 ... 0 ... 0 ... 1 ...
      ** ...
      */

      for(size_t i = 0; i < P_.size1(); i++)
	do
	  {
	    size_t j = distribution(random_device) % P_.size2();

	    if(indexes.find(j) == indexes.end())
	      {
		P_(i, j) = 1;
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
      ** That is, PP^T = I or the inverse of P is equal to P's transpose.
      */

      m_P = P_;
      m_Pinv = boost::numeric::ublas::trans(m_P);
    }
  catch(...)
    {
      return false;
    }

  return true;
}

bool mcnoodle::prepareS(void)
{
  try
    {
      /*
      ** Generate a random k x k binary non-singular matrix and compute
      ** its inverse.
      */

      boost::numeric::ublas::matrix<float> S; /*
					      ** Boost type-checking may
					      ** raise an exception unless
					      ** we're using real numbers.
					      */
      boost::random::uniform_int_distribution<uint64_t> distribution;
      boost::random_device random_device;

      S.resize(m_S.size1(), m_S.size2());

    restart_label:

      for(size_t i = 0; i < S.size1(); i++)
	for(size_t j = 0; j < S.size2(); j++)
	  S(i, j) = static_cast<float> (distribution(random_device) % 2);

      /*
      ** Now, let's compute S's inverse.
      */

      boost::numeric::ublas::permutation_matrix<size_t> pm(S.size1());

      if(boost::numeric::ublas::lu_factorize(S, pm) != 0)
	{
	  std::cerr << "mcnoodle::prepareS(): lu_factorize() returned zero. "
		    << "Restarting." << std::endl;
	  goto restart_label;
	}
      else
	std::cout << "mcnoodle::prepareS(): lu_factorize() completed."
		  << std::endl;

      boost::numeric::ublas::matrix<float> Sinv;

      Sinv.resize(S.size1(), S.size2());
      Sinv.assign(boost::numeric::ublas::identity_matrix<float> (Sinv.size1()));
      lu_substitute(S, pm, Sinv);
      m_S = S;
      m_Sinv = Sinv;
    }
  catch(...)
    {
      return false;
    }

  return true;
}
