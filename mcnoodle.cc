extern "C"
{
#include <inttypes.h>
}

#include <bitset>
#include <map>

#include "mcnoodle.h"

static Eigen::MatrixXd random_permutation_matrix
(const long int m, const long int n)
{
  /*
  ** Generate an m x n random permutation matrix and discover its inverse.
  */

  Eigen::MatrixXd P(m, n);
  boost::random::uniform_int_distribution<uint64_t> distribution;
  boost::random_device random_device;
  std::map<long int, char> indexes;

  P = Eigen::MatrixXd::Zero(m, n);

  /*
  ** 0 ... 1 ... 0 ... 0 ...
  ** 1 ... 0 ... 0 ... 0 ...
  ** 0 ... 0 ... 1 ... 0 ...
  ** 0 ... 0 ... 0 ... 0 ...
  ** 0 ... 0 ... 0 ... 1 ...
  ** ...
  */

  for(long int i = 0; i < P.rows(); i++)
    do
      {
	long int j = static_cast<long int>
	  (distribution(random_device) % P.cols());

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

mcnoodle::mcnoodle(const long int k, const long int n, const long int t)
{
  /*
  ** For generating the private and public keys.
  */

  m_k = minimumK(k);
  m_n = minimumN(n);
  m_t = minimumT(t);

  try
    {
    }
  catch(...)
    {
    }
}

mcnoodle::mcnoodle(const Eigen::MatrixXd &Gcar,
		   const long int k,
		   const long int n,
		   const long int t)
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

      boost::numeric::ublas::matrix<double> c_(1, m_n);

      if(!deserialize(ciphertext, ciphertext_size, c_))
	{
	  rc = false;
	  goto done_label;
	}

      Eigen::MatrixXd c(1, m_n);

      for(size_t i = 0; i < c_.size1(); i++)
	for(size_t j = 0; j < c_.size2(); j++)
	  c(static_cast<long int> (i),
	    static_cast<long int> (j)) = c_(i, j);

      Eigen::MatrixXd ccar(1, m_n);
      Eigen::MatrixXd m(1, m_k);
      Eigen::MatrixXd mcar(1, m_k);

      ccar = c * m_Pinv;

#ifdef MCNOODLE_ARTIFICIAL_GENERATOR
      m = ccar * m_Ginv * m_Sinv;
#else
#endif

      /*
      ** Now convert the matrix m into a character array.
      */

      *plaintext_size = static_cast<size_t>
	(std::ceil(m.cols() / CHAR_BIT)); /*
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

      for(long int i = 0, k = 0; i < m.cols(); k++)
	{
	  std::bitset<CHAR_BIT> b;

	  for(long int j = 0; j < CHAR_BIT && i < m.cols(); i++, j++)
	    b[static_cast<size_t> (j)] = m(0, i);

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
 boost::numeric::ublas::matrix<double> &m)
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

  if(CHAR_BIT * plaintext_size > static_cast<size_t> (m_k))
    return false;

  try
    {
      /*
      ** Represent the message as a binary vector of length k.
      */

      Eigen::MatrixXd m(1, m_k);

      m = Eigen::MatrixXd::Zero(1, m_k);

      for(size_t i = 0, k = 0; i < plaintext_size; i++)
	{
	  std::bitset<CHAR_BIT> b(plaintext[i]);

	  for(long int j = 0; static_cast<size_t> (j) < b.size() &&
		static_cast<long> (k) < m.cols(); j++, k++)
	    m(0, k) = b[static_cast<size_t> (j)];
	}

#ifdef MCNOODLE_ARTIFICIAL_GENERATOR
      /*
      ** This will allow us to prove that decryption and encryption
      ** are correct without a generator G matrix.
      */

      Eigen::MatrixXd c(1, m_n);

      c = m * m_Gcar;
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

      boost::numeric::ublas::matrix<double> c_(1, m_n);

      for(long int i = 0; i < c.rows(); i++)
	for(long int j = 0; j < c.cols(); j++)
	  c_(static_cast<size_t> (i), static_cast<size_t> (j)) = c(i, j);

      return serialize(ciphertext, ciphertext_size, c_);
    }
  catch(...)
    {
      delete []ciphertext;
      ciphertext = 0;
      *ciphertext_size = 0;
      return false;
    }
}

bool mcnoodle::equal(const Eigen::MatrixXd &m1, const Eigen::MatrixXd &m2)
{
  return m1 == m2;
}

bool mcnoodle::equal(const Eigen::MatrixXd &m1,
		     const boost::numeric::ublas::matrix<double> &m2)
{
  if(m1.cols() != static_cast<long int> (m2.size2()) ||
     m1.rows() != static_cast<long int> (m2.size1()))
    return false;

  for(long int i = 0; i < m1.rows(); i++)
    for(long int j = 0; j < m1.cols(); j++)
      if(m1(i, j) != m2(static_cast<long int> (i), static_cast<long int> (j)))
	return false;

  return true;
}

bool mcnoodle::prepareG(void)
{
  try
    {
#ifdef MCNOODLE_ARTIFICIAL_GENERATOR
      m_G = Eigen::MatrixXd::Identity(m_k, m_n);
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
      m_Gcar = m_S * m_G * m_P;
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

      m_P = random_permutation_matrix(m_n, m_n);
      m_Pinv = m_P.transpose();
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

      Eigen::MatrixXd lt(m_k, m_k);
      Eigen::MatrixXd ut(m_k, m_k);
      boost::random::uniform_int_distribution<uint64_t> distribution;
      boost::random_device random_device;

      lt = ut = Eigen::MatrixXd::Zero(m_k, m_k);

      /*
      **
      ** https://en.wikipedia.org/wiki/LU_decomposition
      */

      for(long int i = 0; i < lt.rows(); i++)
	for(long int j = 0; j <= i; j++)
	  if(i == j)
	    lt(i, j) = 1;
	  else
	    lt(i, j) = static_cast<double> (distribution(random_device) % 2);

      for(long int i = 0; i < ut.rows(); i++)
	for(long int j = i; j < ut.cols(); j++)
	  if(i == j)
	    ut(i, j) = 1;
	  else
	    ut(i, j) = static_cast<double> (distribution(random_device) % 2);

      /*
      ** PS = LU, S is our random binary matrix.
      ** S = P^-1LU = P^TLU.
      */

      Eigen::MatrixXd P_(m_k, m_k);
      Eigen::MatrixXd S(m_k, m_k);
      Eigen::MatrixXd m(m_k, m_k);

      m = random_permutation_matrix(m_k, m_k);

      for(long int i = 0; i < m.rows(); i++)
	for(long int j = 0; j < m.cols(); j++)
	  P_(i, j) = m(i, j);

      m_S = S = P_.transpose() * lt * ut;

      Eigen::MatrixXd ltinv(m_k, m_k);
      Eigen::MatrixXd utinv(m_k, m_k);

      ltinv = lt.inverse();
      utinv = ut.inverse();

      /*
      ** S^-1 = U^-1L^-1^(P^-1)^-1.
      */

      Eigen::MatrixXd Sinv(m_k, m_k);

      m_Sinv = Sinv = utinv * ltinv * P_;
    }
  catch(...)
    {
      return false;
    }

  return true;
}

bool mcnoodle::serialize(char *&buffer,
			 size_t *buffer_size,
			 const Eigen::MatrixXd &m)
{
  boost::numeric::ublas::matrix<double> m_
    (static_cast<size_t> (m.rows()), static_cast<size_t> (m.cols()));

  for(long int i = 0; i < m.rows(); i++)
    for(long int j = 0; j < m.cols(); j++)
      m_(static_cast<size_t> (i), static_cast<size_t> (j)) = m(i, j);

  return serialize(buffer, buffer_size, m_);
}

bool mcnoodle::serialize(char *&buffer,
			 size_t *buffer_size,
			 const boost::numeric::ublas::matrix<double> &m)
{
  if(buffer || !buffer_size)
    return false;

  *buffer_size = sizeof(double) * sizeof(double) * m.size1() * m.size2();

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
