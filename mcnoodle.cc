extern "C"
{
#include <inttypes.h>
}

#include <bitset>
#include <map>

#include "mcnoodle.h"

mcnoodle::mcnoodle(const size_t k,
		   const size_t n,
		   const size_t t)
{
  NTL::ZZ p(2);

  NTL::ZZ_p::init(p);
  m_k = minimumK(k);
  m_n = minimumN(n);
  m_t = minimumT(t);

  try
    {
      m_S.SetDims(m_k, m_k);
      m_Sinv.SetDims(m_k, m_k);
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

  return true;
}

template<class T>
bool mcnoodle::deserialize
(const char *buffer, const size_t buffer_size,
 boost::numeric::ublas::matrix<T> &m)
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

  return true;
}

bool mcnoodle::prepareG(void)
{
  try
    {
#ifdef MCNOODLE_ARTIFICIAL_GENERATOR
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
      NTL::ZZ_p determinant;
      boost::random::uniform_int_distribution<uint64_t> distribution;
      boost::random_device random_device;

    restart_label:

      for(size_t i = 0; i < m_k; i++)
	for(size_t j = 0; j < m_k; j++)
	  if(i == j)
	    m_S[i][j] = 1;
	  else
	    m_S[i][j] = static_cast<int> (distribution(random_device) % 2);

      NTL::inv(determinant, m_Sinv, m_S);

      if(determinant == 0)
	goto restart_label;
    }
  catch(...)
    {
      return false;
    }

  return true;
}

template<class T>
bool mcnoodle::serialize(char *&buffer,
			 size_t *buffer_size,
			 const boost::numeric::ublas::matrix<T> &m)
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
