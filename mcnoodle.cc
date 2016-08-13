extern "C"
{
#include <inttypes.h>
#include <limits.h>
#include <string.h>
}

#include <bitset>
#include <limits>
#include <map>
#include <vector>

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
  prepareMX(); /*
	       ** Must be performed prior to
	       ** prepareIrreducibleGenerator() as it
	       ** also initializes some NTL containers.
	       */
  prepareIrreducibleGenerator();
  prepareG();
  prepareP();
  prepareS();

  long int n = static_cast<long int> (m_n);
  std::vector<long int> dividers;

  for(long int i = 2; i < (n - 1) / 2 + 1; i++)
    if((n - 1) % i == 0)
      dividers.push_back(i);

  for(long int i = 2; i < n; i++)
    {
      NTL::GF2E gf2e;
      NTL::GF2X gf2x;
      bool p = true;

      gf2x.SetLength(static_cast<long int> (m));
      gf2x = NTL::GF2X::zero();

      for(long int j = 0; j < static_cast<long int> (m); j++)
	/*
	** 0 or 1, selected randomly.
	*/

	NTL::SetCoeff(gf2x, j, NTL::RandomBnd(2));

      gf2e = NTL::to_GF2E(gf2x);

      for(int long j = 0; j < static_cast<long int> (dividers.size()); j++)
	if(NTL::power(gf2e, dividers[j]) == NTL::to_GF2E(1))
	  {
	    p = false;
	    break;
	  }

      if(p)
	{
	  m_A = gf2e;
	  break;
	}
    }

  m_L.SetLength(n);

  for(long int i = 0; i < m_L.length(); i++)
    if(i == 0)
      m_L[i] = NTL::GF2E::zero();
    else if(i == 1)
      m_L[i] = m_A;
    else
      m_L[i] = m_A * m_L[i - 1];
}

mcnoodle_private_key::~mcnoodle_private_key()
{
}

bool mcnoodle_private_key::prepareG(void)
{
  try
    {
      long int k = static_cast<long int> (m_k);
      long int n = static_cast<long int> (m_n);

      m_G.SetDims(k, n);
    }
  catch(...)
    {
      return false;
    }

  return true;
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

mcnoodle_public_key::mcnoodle_public_key(const size_t m,
					 const size_t t)
{
  m_t = mcnoodle::minimumT(t);

  /*
  ** Some calculations.
  */

  long int k = 0;
  long int n = 1 << static_cast<long int> (m);

  k = n - static_cast<long int> (m) * static_cast<long int> (m_t);
  m_Gcar.SetDims(k, n);
}

mcnoodle_public_key::~mcnoodle_public_key()
{
}

mcnoodle::mcnoodle(const size_t m,
		   const size_t t)
{
  m_privateKey = 0;
  m_publicKey = 0;

  try
    {
      initializeSystemParameters(m, t);
    }
  catch(...)
    {
    }
}

mcnoodle::mcnoodle(const size_t m,
		   const size_t t,
		   const std::stringstream &G,
		   const std::stringstream &P,
		   const std::stringstream &S)
{
  m_privateKey = 0;
  m_publicKey = 0;

  try
    {
      initializeSystemParameters(m, t);
    }
  catch(...)
    {
    }
}

mcnoodle::~mcnoodle()
{
  delete m_privateKey;
  delete m_publicKey;
}

bool mcnoodle::decrypt(const std::stringstream &ciphertext,
		       std::stringstream &plaintext)
{
  if(!m_privateKey)
    return false;

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

      ccar = c * m_privateKey->Pinv();
      m = ccar * m_privateKey->Sinv();

      size_t plaintext_size = static_cast<size_t>
	(std::ceil(m.NumCols() / CHAR_BIT)); /*
					     ** m_n is not necessarily
					     ** a multiple of CHAR_BIT.
					     ** It may be, however.
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

bool mcnoodle::encrypt(const char *plaintext,
		       const size_t plaintext_size,
		       std::stringstream &ciphertext)
{
  if(!m_publicKey || !plaintext || plaintext_size <= 0)
    return false;

  if(CHAR_BIT * plaintext_size > static_cast<size_t> (m_k))
    return false;

  try
    {
      /*
      ** Represent the message as a binary vector of length k.
      */

      NTL::vec_GF2 m;

      m.SetLength(static_cast<long int> (m_k));

      for(size_t i = 0, k = 0; i < plaintext_size; i++)
	{
	  std::bitset<CHAR_BIT> b(plaintext[i]);

	  for(long int j = 0; static_cast<size_t> (j) < b.size() &&
		static_cast<long int> (k) < m.length(); j++, k++)
	    m[k] = b[static_cast<size_t> (j)];
	}

      /*
      ** Create the random vector e. It will contain at most t ones.
      */

      NTL::vec_GF2 e;
      long int t = static_cast<long int> (m_t);
      long int ts = 0;

      e.SetLength(static_cast<long int> (m_n));

      do
	{
	  long int i = NTL::RandomBnd(e.length());

	  if(e(i) == 0)
	    {
	      e[i] = 1;
	      ts += 1;
	    }
	}
      while(t > ts);

      NTL::vec_GF2 c = m * m_publicKey->Gcar() + e;

      ciphertext << c;
    }
  catch(...)
    {
      return false;
    }

  return true;
}

bool mcnoodle::generatePrivatePublicKeys(void)
{
  try
    {
      m_privateKey = new mcnoodle_private_key(m_m, m_t);
      m_publicKey = new mcnoodle_public_key(m_m, m_t);

      NTL::mat_GF2 H;
      long int m = static_cast<long int> (m_m);
      long int n = static_cast<long int> (m_n);
      long int t = static_cast<long int> (m_t);

      H.SetDims(m * t, n);

      for(long int i = 0; i < t; i++)
	for(long int j = 0; j < n; j++)
	  {
	    NTL::GF2E gf2e = NTL::inv(NTL::eval(m_privateKey->g(),
						m_privateKey->L()[j])) *
	      NTL::power(m_privateKey->L()[j], i);
	    NTL::vec_GF2 v = NTL::to_vec_GF2(gf2e._GF2E__rep);

	    for(long int k = 0; k < v.length(); k++)
	      H[i * m + k][j] = v[k];
      }

      NTL::gauss(H);

      /*
      ** Reduced row echelon form.
      */

      long int lead = 0;

      for(long int r = 0; r < H.NumRows(); r++)
	{
	  if(H.NumCols() <= lead)
	    break;

	  long int i = r;

	  while(H[i][lead] == 0)
	    {
	      i += 1;

	      if(H.NumRows() == i)
		{
		  i = r;
		  lead += 1;

		  if(H.NumCols() == lead)
		    goto done_label;
		}
	    }

	  NTL::swap(H[i], H[r]);

	  if(H[r][lead] != 0)
	    for(long int j = 0; j < H.NumCols(); j++)
	      H[r][j] /= H[r][lead];

	  for(long int j = 0; j < H.NumRows(); j++)
	    if(j != r)
	      H[j] = H[j] - H[j][lead] * H[r];

	  lead += 1;
	}

      /*
      ** H = [I|R].
      */

      for(long int i = 0; i < H.NumRows(); i++)
	if(H[i][i] == 0)
	  {
	    bool pivot = true;

	    for(long int j = i + 1; j < H.NumCols(); j++)
	      {
		if(H[i][j] == 1)
		  {
		    for(long int k = i + 1; k < H.NumRows(); k++)
		      {
			if(H[k][j] == 1)
			  {
			    pivot = false;
			    break;
			  }
		      }

		    if(!pivot)
		      break;

		    for(long int k = i - 1; k >= 0; k--)
		      if(H[k][j] == 1)
			{
			  pivot = false;
			  break;
			}
		  }
		else
		  continue;

		if(pivot)
		  break;
	      }
	  }

      m_publicKey->prepareGcar
	(m_privateKey->G(), m_privateKey->P(), m_privateKey->S());
    }
  catch(...)
    {
      delete m_privateKey;
      m_privateKey = 0;
      delete m_publicKey;
      m_publicKey = 0;
      return false;
    }

 done_label:
  return true;
}

void mcnoodle::privateKeyParameters(std::stringstream &G,
				    std::stringstream &P,
				    std::stringstream &S)
{
  G << m_privateKey->G();
  P << m_privateKey->P();
  S << m_privateKey->S();
}

void mcnoodle::publicKeyParameters(size_t &t, std::stringstream &Gcar)
{
  if(m_publicKey)
    {
      Gcar << m_publicKey->Gcar();
      t = m_t;
    }
}
