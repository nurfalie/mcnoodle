extern "C"
{
#include <inttypes.h>
#include <limits.h>
#include <string.h>
}

#include <bitset>
#include <limits>
#include <map>

#include "mcnoodle.h"

mcnoodle_private_key::mcnoodle_private_key(const size_t m, const size_t t)
{
  m_d = 0;
  m_k = 0;
  m_m = mcnoodle::minimumM(m);
  m_n = 1 << m_m; // 2^m
  m_t = mcnoodle::minimumT(t);

  /*
  ** Some calculations.
  */

  m_d = 2 * m_t + 1;
  m_k = m_n - m_m * m_t;
  m_polynomial = NTL::BuildRandomIrred(NTL::BuildIrred_GF2X(m_m));
  prepareP();
  prepareS();
}

mcnoodle_private_key::~mcnoodle_private_key()
{
}

bool mcnoodle_private_key::prepareP(void)
{
  try
    {
      long int n = static_cast<long int> (m_n);
      std::map<long int, char> indexes;

      /*
      ** 0 ... 1 ... 0 ... 0 ...
      ** 1 ... 0 ... 0 ... 0 ...
      ** 0 ... 0 ... 1 ... 0 ...
      ** 0 ... 0 ... 0 ... 0 ...
      ** 0 ... 0 ... 0 ... 1 ...
      ** ...
      */

      m_P.SetDims(n, n);
      m_Pinv.SetDims(n, n);

      for(long int i = 0; i < m_P.NumRows(); i++)
	do
	  {
	    long int j = NTL::RandomBnd(m_P.NumCols());

	    if(indexes.find(j) == indexes.end())
	      {
		indexes[j] = 0;
		m_P[i][j] = 1;
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

      m_Pinv = NTL::transpose(m_P);
    }
  catch(...)
    {
      NTL::clear(m_P);
      NTL::clear(m_Pinv);
      return false;
    }

  return true;
}

bool mcnoodle_private_key::prepareS(void)
{
  try
    {
      int long k = static_cast<long int> (m_k);

      m_S.SetDims(k, k);

      do
	{
	  for(long int i = 0; i < k; i++)
	    m_S[i] = NTL::random_vec_GF2(k);
	}
      while(determinant(m_S) == 0);

      m_Sinv = NTL::inv(m_S);
    }
  catch(...)
    {
      NTL::clear(m_S);
      NTL::clear(m_Sinv);
      return false;
    }

  return true;
}

mcnoodle_public_key::mcnoodle_public_key(const size_t t)
{
  m_t = mcnoodle::minimumT(t);
}

mcnoodle_public_key::~mcnoodle_public_key()
{
}

mcnoodle::mcnoodle(const size_t m,
		   const size_t t)
{
  m_m = minimumM(m);
  m_n = 1 << m_m; // 2^m
  m_privateKey = new mcnoodle_private_key(m, t);
  m_publicKey = new mcnoodle_public_key(t);
  m_t = minimumM(t);

  /*
  ** Some calculations.
  */

  m_k = m_n - m_m * m_t;
}

mcnoodle::~mcnoodle()
{
  delete m_privateKey;
  delete m_publicKey;
}

bool mcnoodle::decrypt(const std::stringstream &ciphertext,
		       std::stringstream &plaintext)
{
  char *p = 0;

  try
    {
      NTL::mat_GF2 c;
      std::stringstream s;

      s << ciphertext.rdbuf();
      s >> c;

      if(c.NumCols() != static_cast<long int> (m_n) && c.NumRows() != 1)
	return false;

      NTL::mat_GF2 ccar;
      NTL::mat_GF2 m;

      ccar = c * m_privateKey->m_Pinv;
      m = ccar * m_privateKey->m_Sinv;

      size_t plaintext_size = static_cast<size_t>
	(std::ceil(m.NumCols() / CHAR_BIT)); /*
					     ** m_n is not necessarily
					     ** a multiple of CHAR_BIT.
					     */

      if(plaintext_size <= 0) // Unlikely.
	return false;

      p = new char[plaintext_size];
      memset(p, 0, plaintext_size);

      for(long int i = 0, k = 0; i < m.NumCols(); k++)
	{
	  std::bitset<CHAR_BIT> b;

	  for(long int j = 0; j < CHAR_BIT && i < m.NumCols(); i++, j++)
	    b[static_cast<size_t> (j)] = m[0][i] == 0 ? 0 : 1;

	  p[k] = static_cast<char> (b.to_ulong());
	}

      plaintext << p;
    }
  catch(...)
    {
      delete []p;
      return false;
    }

  delete []p;
  return true;
}

bool mcnoodle::encrypt(const char *plaintext, const size_t plaintext_size,
		       std::stringstream &ciphertext)
{
  if(!plaintext || plaintext_size <= 0)
    return false;

  if(CHAR_BIT * plaintext_size > static_cast<size_t> (m_k))
    return false;

  try
    {
      /*
      ** Represent the message as a binary vector of length k.
      */

      NTL::mat_GF2 m;

      m.SetDims(1, static_cast<long int> (m_k));

      for(size_t i = 0, k = 0; i < plaintext_size; i++)
	{
	  std::bitset<CHAR_BIT> b(plaintext[i]);

	  for(long int j = 0; static_cast<size_t> (j) < b.size() &&
		static_cast<long int> (k) < m.NumCols(); j++, k++)
	    m[0][k] = b[static_cast<size_t> (j)];
	}

      ciphertext << m;
    }
  catch(...)
    {
      return false;
    }

  return true;
}
