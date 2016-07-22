extern "C"
{
#include <inttypes.h>
}

#include <bitset>
#include <map>

#include "mcnoodle.h"

template<class T>
static bool matrix_inverse
(const boost::numeric::ublas::matrix<T> &m,
 boost::numeric::ublas::matrix<T> &inverse)
{
  try
    {
      boost::numeric::ublas::matrix<T> a(m);
      boost::numeric::ublas::permutation_matrix<size_t> pm(a.size1());

      if(boost::numeric::ublas::lu_factorize(a, pm) != 0)
	return false;

      inverse.assign(boost::numeric::ublas::identity_matrix<T> (a.size1()));
      boost::numeric::ublas::lu_substitute(a, pm, inverse);
    }
  catch(...)
    {
      return false;
    }

  return true;
}

template<class T>
static boost::numeric::ublas::matrix<T> random_permutation_matrix
(const size_t m, const size_t n)
{
  /*
  ** Generate an m x n random permutation matrix and discover its inverse.
  */

  boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> P(m, n, 0);
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

  for(size_t i = 0; i < P.size1(); i++)
    do
      {
	size_t j = distribution(random_device) % P.size2();

	if(indexes.find(j) == indexes.end())
	  {
	    P(i, j) = 1;
	    indexes[j] = 0;
	    break;
	  }
      }
    while(true);

  return P;
}

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

  bool rc = true;

  try
    {
      /*
      ** Store the ciphertext into an 1-by-m_n vector.
      */

      boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> c(1, m_n);

      if(!deserialize(ciphertext, ciphertext_size, c))
	{
	  rc = false;
	  goto done_label;
	}

      boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> ccar
	(1, m_n);
      boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> m
	(1, m_k);
      boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> mcar
	(1, m_k);

      ccar = boost::numeric::ublas::prod(c, m_Pinv);

#ifdef MCNOODLE_ARTIFICIAL_GENERATOR
      mcar = boost::numeric::ublas::prod(ccar, m_Ginv);
      m = boost::numeric::ublas::prod(mcar, m_Sinv);
#else
#endif

      /*
      ** Now convert the matrix m into a character array.
      */

      *plaintext_size = static_cast<size_t>
	(std::ceil(m.size2() / CHAR_BIT)); /*
					   ** m_n is not necessarily a multiple
					   ** of CHAR_BIT.
					   */

      if(*plaintext_size <= 0) // Unlikely.
	{
	  rc = false;
	  goto done_label;
	}

      plaintext = new char[*plaintext_size];
      memset(plaintext, 0, *plaintext_size);

      for(size_t i = 0, k = 0; i < m.size2(); k++)
	{
	  std::bitset<CHAR_BIT> b;

	  for(size_t j = 0; j < CHAR_BIT && i < m.size2(); i++, j++)
	    b[j] = m(0, i);

	  plaintext[k] = static_cast<char> (b.to_ulong());
	}
    }
  catch(...)
    {
      rc = false;
    }

 done_label:

  if(!rc)
    {
      delete []plaintext;
      *plaintext_size = 0;
    }

  return rc;
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

	  for(size_t j = 0; j < b.size() && k < m.size2(); j++, k++)
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

bool mcnoodle::prepareG(void)
{
  try
    {
#ifdef MCNOODLE_ARTIFICIAL_GENERATOR
      m_G = boost::numeric::ublas::identity_matrix
	<mcnoodle_matrix_element_type_t> (m_G.size1());
      m_Ginv = m_G;
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
      ** A permutation matrix always has an inverse.
      */

      /*
      ** (PP^T)ij = Sum(Pik(P^T)kj, k = 1..n) = Sum(PikPjk, k = 1..n).
      ** Sum(PikPjk, k = 1..n) = 1 if i = j, and 0 otherwise (I).
      ** That is, PP^T = I or the inverse of P is equal to P's transpose.
      */

      m_P = random_permutation_matrix<mcnoodle_matrix_element_type_t>
	(m_n, m_n);
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
      ** Generate random lower and upper triangular matrices.
      */

      boost::numeric::ublas::matrix<double> lt(m_n, m_n, 0);
      boost::numeric::ublas::matrix<double> ut(m_n, m_n, 0);
      boost::random::uniform_int_distribution<uint64_t> distribution;
      boost::random_device random_device;

      /*
      **
      ** https://en.wikipedia.org/wiki/LU_decomposition
      */

      for(size_t i = 0; i < lt.size1(); i++)
	for(size_t j = 0; j <= i; j++)
	  if(i == j)
	    lt(i, j) = 1;
	  else
	    lt(i, j) = static_cast<double> (distribution(random_device) % 2);

      for(size_t i = 0; i < ut.size1(); i++)
	for(size_t j = i; j < ut.size2(); j++)
	  if(i == j)
	    ut(i, j) = 1;
	  else
	    ut(i, j) = static_cast<double> (distribution(random_device) % 2);

      /*
      ** PS = LU, S is our random binary matrix.
      ** S = P^-1LU = P^TLU.
      */

      boost::numeric::ublas::matrix<double> P_(m_n, m_n);
      boost::numeric::ublas::matrix<double> S(m_n, m_n);

      P_ = random_permutation_matrix<double>(m_n, m_n);
      S = boost::numeric::ublas::prod(boost::numeric::ublas::trans(P_), lt);
      S = boost::numeric::ublas::prod(S, ut);

      boost::numeric::ublas::matrix<double> ltinv(m_n, m_n);
      boost::numeric::ublas::matrix<double> utinv(m_n, m_n);

      matrix_inverse(lt, ltinv);
      matrix_inverse(ut, utinv);

      /*
      ** S^-1 = U^-1L^-1^(P^-1)^-1.
      */

      boost::numeric::ublas::matrix<double> Sinv(m_n, m_n);

      Sinv = boost::numeric::ublas::prod(utinv, ltinv);
      Sinv = boost::numeric::ublas::prod(Sinv, P_);

      for(size_t i = 0; i < S.size1(); i++)
	for(size_t j = 0; j < S.size2(); j++)
	  {
	    m_S(i, j) = static_cast<mcnoodle_matrix_element_type_t> (S(i, j));
	    m_Sinv(i, j) = Sinv(i, j);
	  }
    }
  catch(...)
    {
      return false;
    }

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

  buffer = new char[*buffer_size];
  memset(buffer, 0, *buffer_size);

  try
    {
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
