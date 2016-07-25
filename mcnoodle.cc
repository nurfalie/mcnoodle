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

    restart_label:

      for(size_t i = 0; i < m_k; i++)
	for(size_t j = 0; j < m_k; j++)
	  if(i == j)
	    m_S[i][j] = 1;
	  else
	    m_S[i][j] = NTL::RandomBnd(2);

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
