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

mcnoodle::mcnoodle(const size_t k,
		   const size_t n,
		   const size_t t)
{
  NTL::ZZ p(2);

  NTL::ZZ_p::init(p);
  m_k = minimumK(k);
  m_n = minimumN(n);
  m_t = minimumT(t);

#ifdef MCNOODLE_ARTIFICIAL_GENERATOR
  m_n = m_k;
#endif

  try
    {
      m_G.SetDims(static_cast<long int> (m_n),
		  static_cast<long int> (m_k));
      m_Gcar.SetDims(static_cast<long int> (m_n),
		     static_cast<long int> (m_k));
      m_Ginv.SetDims(static_cast<long int> (m_n),
		     static_cast<long int> (m_k));
      m_P.SetDims(static_cast<long int> (m_n),
		  static_cast<long int> (m_n));
      m_Pinv.SetDims(static_cast<long int> (m_n),
		     static_cast<long int> (m_n));
      m_S.SetDims(static_cast<long int> (m_k),
		  static_cast<long int> (m_k));
      m_Sinv.SetDims(static_cast<long int> (m_k),
		     static_cast<long int> (m_k));
    }
  catch(...)
    {
    }
}

mcnoodle::~mcnoodle()
{
}

bool mcnoodle::decrypt(const std::stringstream &ciphertext,
		       std::stringstream &plaintext)
{
  char *p = 0;

  try
    {
      NTL::mat_ZZ_p c;
      std::stringstream s;

      s << ciphertext.rdbuf();
      s >> c;

      NTL::mat_ZZ_p ccar;
      NTL::mat_ZZ_p m;

      ccar = c * m_Pinv;
#ifdef MCNOODLE_ARTIFICIAL_GENERATOR
      m = ccar * m_Ginv * m_Sinv;
#endif

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

      NTL::mat_ZZ_p m;

      m.SetDims(1, static_cast<long int> (m_k));

      for(size_t i = 0, k = 0; i < plaintext_size; i++)
	{
	  std::bitset<CHAR_BIT> b(plaintext[i]);

	  for(long int j = 0; static_cast<size_t> (j) < b.size() &&
		static_cast<long int> (k) < m.NumCols(); j++, k++)
	    m[0][k] = b[static_cast<size_t> (j)];
	}

#ifdef MCNOODLE_ARTIFICIAL_GENERATOR
      /*
      ** This will allow us to prove that decryption and encryption
      ** are correct without a generator G matrix.
      */

      ciphertext << m * m_Gcar;
#else
#endif
    }
  catch(...)
    {
      return false;
    }

  return true;
}

bool mcnoodle::prepareG(void)
{
  try
    {
#ifdef MCNOODLE_ARTIFICIAL_GENERATOR
      NTL::ident(m_G, m_k);
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
      std::map<long int, char> indexes;

      /*
      ** 0 ... 1 ... 0 ... 0 ...
      ** 1 ... 0 ... 0 ... 0 ...
      ** 0 ... 0 ... 1 ... 0 ...
      ** 0 ... 0 ... 0 ... 0 ...
      ** 0 ... 0 ... 0 ... 1 ...
      ** ...
      */

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

      NTL::transpose(m_Pinv, m_P);
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
	    m_S[i][j] = NTL::RandomBnd
	      (static_cast<int> (std::numeric_limits<uint64_t>::max() % 2));

      NTL::inv(determinant, m_Sinv, m_S);

      if(determinant == 0)
	{
	  std::cerr << "Zero determinant!\n";
	  goto restart_label;
	}

      std::string a;
    }
  catch(...)
    {
      return false;
    }

  return true;
}
