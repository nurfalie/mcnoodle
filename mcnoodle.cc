/*
** Copyright (c) Alexis Megas.
** All rights reserved.
**
** Software based on specifications provided by Antoon Bosselaers,
** René Govaerts, Robert McEliece, Bart Preneel, Marek Repka,
** Christopher Roering, Joos Vandewalle.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from skein without specific prior written permission.
**
** MCNOODLE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** MCNOODLE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <bitset>
#include <map>

#include "mcnoodle.h"

mcnoodle_private_key::mcnoodle_private_key(const size_t m, const size_t t)
{
  m_k = 0;
  m_m = mcnoodle::minimumM(m);
  m_n = 1 << m_m; // 2^m
  m_ok = true;
  m_t = mcnoodle::minimumT(t);

  /*
  ** Some calculations.
  */

  m_k = m_n - m_m * m_t;

  /*
  ** Prepare important containers.
  */

  prepare_gZ();
  prepareP();
  prepareS();
  prepareSwappingColumns();

  long int n = static_cast<long int> (m_n);
  std::vector<long int> dividers;

  for(long int i = 2; i < (n - 1) / 2 + 1; i++)
    if((n - 1) % i == 0)
      dividers.push_back(i);

  NTL::GF2E A = NTL::GF2E::zero();

  for(long int i = 2; i < n; i++)
    {
      NTL::GF2E gf2e;
      NTL::GF2X gf2x;
      bool found = true;

      gf2x.SetLength(static_cast<long int> (m));
      gf2x = NTL::GF2X::zero();

      for(long int j = 0; j < static_cast<long int> (m); j++)
	/*
	** 0 or 1, selected randomly. This has the potential
	** of introducing divisions by zero. Only a test library!
	*/

	NTL::SetCoeff(gf2x, j, NTL::RandomBnd(2));

      A = gf2e = NTL::to_GF2E(gf2x);

      for(int long j = 0; j < static_cast<long int> (dividers.size()); j++)
	if(NTL::power(gf2e, dividers[j]) == NTL::to_GF2E(1))
	  {
	    found = false;
	    break;
	  }

      if(found)
	{
	  A = gf2e;
	  break;
	}
    }

  m_L.SetLength(n);

  for(long int i = 0; i < n; i++)
    if(i == 0)
      m_L[i] = NTL::GF2E::zero(); // Lambda-0 is always zero.
    else if(i == 1)
      m_L[i] = A; // Discovered generator.
    else
      m_L[i] = A * m_L[i - 1];

  preparePreSynTab();
}

mcnoodle_private_key::~mcnoodle_private_key()
{
}

bool mcnoodle_private_key::prepareG(const NTL::mat_GF2 &R)
{
  try
    {
      if(m_swappingColumns.size() != static_cast<size_t> (m_n))
	{
	  m_ok = false;
	  return false;
	}

      long int k = static_cast<long int> (m_k);
      long int n = static_cast<long int> (m_n);

      m_G.SetDims(k, n);

      for(long int i = 0; i < k; i++)
	{
	  for(long int j = 0; j < n - k; j++)
	    m_G[i][j] = R[i][j];

	  m_G[i][n - k + i] = NTL::to_GF2(1);
	}

      NTL::mat_GF2 mat_GF2;

      mat_GF2.SetDims(k, n);

      for(long int i = 0; i < n; i++)
	for(long int j = 0; j < k; j++)
	  mat_GF2[j][m_swappingColumns[i]] = m_G[j][i];

      m_G = mat_GF2;
    }
  catch(...)
    {
      NTL::clear(m_G);
      m_ok = false;
      return false;
    }

  m_ok &= true;
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
      m_ok = false;
      return false;
    }

  m_ok &= true;
  return true;
}

bool mcnoodle_private_key::preparePreSynTab(void)
{
  try
    {
      if(!NTL::deg(m_gZ))
	{
	  m_ok = false;
	  return false;
	}

      long int n = static_cast<long int> (m_n);

      if(m_L.length() != n)
	{
	  m_ok = false;
	  return false;
	}

      m_X.SetLength(2);
      NTL::SetCoeff(m_X, 0, 0);
      NTL::SetCoeff(m_X, 1, 1);
      m_preSynTab.clear();

      for(long int i = 0; i < n; i++)
	/*
	** Division by zero will occur if m_X - m_L[i] equals zero.
	**/

	if(!NTL::IsZero(m_X - m_L[i]))
	  m_preSynTab.push_back(NTL::InvMod(m_X - m_L[i], m_gZ));
	else
	  m_preSynTab.push_back(m_X);
    }
  catch(...)
    {
      m_ok = false;
      m_preSynTab.clear();
      return false;
    }

  m_ok &= true;
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
      while(NTL::determinant(m_S) == 0);

      m_Sinv = NTL::inv(m_S);
    }
  catch(...)
    {
      NTL::clear(m_S);
      NTL::clear(m_Sinv);
      m_ok = false;
      return false;
    }

  m_ok &= true;
  return true;
}

bool mcnoodle_private_key::prepare_gZ(void)
{
  try
    {
      NTL::GF2E::init
	(NTL::BuildIrred_GF2X(static_cast<long int> (m_m))); /*
							     ** Initialize
							     ** some NTL
							     ** internal
							     ** object(s).
							     */
      m_gZ = NTL::BuildRandomIrred
	(NTL::BuildIrred_GF2EX(static_cast<long int> (m_t)));
    }
  catch(...)
    {
      NTL::clear(m_gZ);
      m_ok = false;
      return false;
    }

  m_ok &= true;
  return true;
}

void mcnoodle_private_key::prepareSwappingColumns(void)
{
  m_swappingColumns.clear();

  long int n = static_cast<long int> (m_n);

  for(long int i = 0; i < n; i++)
    m_swappingColumns.push_back(i);
}

void mcnoodle_private_key::swapSwappingColumns(const long int i,
					       const long int j)
{
  if(static_cast<size_t> (i) >= m_swappingColumns.size() ||
     static_cast<size_t> (j) >= m_swappingColumns.size())
    {
      m_ok = false;
      return;
    }

  long int t = m_swappingColumns[i];

  m_swappingColumns[i] = m_swappingColumns[j];
  m_swappingColumns[j] = t;
}

mcnoodle_public_key::mcnoodle_public_key(const size_t m,
					 const size_t t)
{
  m_ok = true;
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

bool mcnoodle_public_key::prepareGcar(const NTL::mat_GF2 &G,
				      const NTL::mat_GF2 &P,
				      const NTL::mat_GF2 &S)
{
  try
    {
      m_Gcar = S * G * P;
    }
  catch(...)
    {
      NTL::clear(m_Gcar);
      m_ok = false;
      return false;
    }

  m_ok &= true;
  return true;
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

mcnoodle::~mcnoodle()
{
  delete m_privateKey;
  delete m_publicKey;
}

bool mcnoodle::decrypt(const std::stringstream &ciphertext,
		       std::stringstream &plaintext) const
{
  if(!m_privateKey || !m_privateKey->ok())
    return false;

  size_t plaintext_size = static_cast<size_t>
    (std::ceil(m_k / CHAR_BIT)); /*
				 ** m_k is not necessarily
				 ** a multiple of CHAR_BIT.
				 ** It may be, however.
				 */

  if(plaintext_size <= 0) // Unlikely.
    return false;

  char *p = new (std::nothrow) char[plaintext_size];

  if(!p)
    return false;

  try
    {
      NTL::vec_GF2 c;
      std::stringstream s;

      s << ciphertext.rdbuf();
      s >> c;

      if(c.length() != static_cast<long int> (m_n))
	{
	  delete []p;
	  return false;
	}

      NTL::vec_GF2 ccar = c * m_privateKey->Pinv();

      if(ccar.length() != static_cast<long int> (m_n) ||
	 m_n != m_privateKey->preSynTab().size())
	{
	  delete []p;
	  return false;
	}

      /*
      ** Patterson.
      */

      NTL::GF2EX syndrome = NTL::GF2EX::zero();
      long int n = static_cast<long int> (m_n);
      std::vector<NTL::GF2EX> v(m_privateKey->preSynTab());

      for(long int i = 0; i < n; i++)
	if(ccar[i] != 0)
	  syndrome += v[i];

      NTL::GF2EX sigma = NTL::GF2EX::zero();

      if(!NTL::IsZero(syndrome))
	{
	  NTL::GF2EX T = NTL::InvMod(syndrome, m_privateKey->gZ()) +
	    m_privateKey->X();
	  NTL::GF2EX alpha = NTL::GF2EX::zero();
	  NTL::GF2EX beta = NTL::GF2EX::zero();
	  NTL::GF2EX gamma = NTL::GF2EX::zero();
	  NTL::GF2EX tau = NTL::GF2EX::zero();
	  NTL::ZZ exponent = NTL::power
	    (NTL::power2_ZZ(static_cast<long int> (m_t)),
	     static_cast<long int> (m_m)) / 2;

	  if(NTL::IsZero(T))
	    sigma = m_privateKey->X();
	  else
	    {
	      tau = NTL::PowerMod(T, exponent, m_privateKey->gZ());

	      NTL::GF2E c1;
	      NTL::GF2E c2;
	      NTL::GF2E c3;
	      NTL::GF2E c4;
	      NTL::GF2EX gf2ex = NTL::GF2EX::zero();
	      NTL::GF2EX r0 = m_privateKey->gZ();
	      NTL::GF2EX r1 = tau;
	      NTL::GF2EX u0 = NTL::GF2EX::zero();
	      NTL::GF2EX u1;
	      long int dr = NTL::deg(r1);
	      long int dt = NTL::deg(r0) - dr;
	      long int du = 0;
	      long int t = static_cast<long int> (m_t / 2);

	      u1.SetLength(1);
	      NTL::SetCoeff(u1, 0, 1);

	      while(dr >= t + 1)
		{
		  for(long int j = dt; j >= 0; j--)
		    {
		      NTL::GetCoeff(c1, r0, dr + j);
		      NTL::GetCoeff(c2, r1, dr);
		      c3 = c1 * NTL::inv(c2);
		      c1 = c3;

		      if(!NTL::IsZero(c1))
			{
			  for(long int i = 0; i <= du; i++)
			    {
			      NTL::GetCoeff(c3, u0, i + j);
			      NTL::GetCoeff(c4, u1, i);
			      c3 = c3 + c1 * c4;
			      NTL::SetCoeff(u0, i + j, c3);
			    }

			  for(long int i = 0; i <= dr; i++)
			    {
			      NTL::GetCoeff(c3, r0, i + j);
			      NTL::GetCoeff(c4, r1, i);
			      c3 = c3 + c1 * c4;
			      NTL::SetCoeff(r0, i + j, c3);
			    }
			}
		    }

		  gf2ex = r0;
		  r0 = r1;
		  r1 = gf2ex;
		  gf2ex = u0;
		  u0 = u1;
		  u1 = gf2ex;
		  du = du + dt;
		  dt = 1;
		  NTL::GetCoeff(c3, r1, dr - dt);

		  while(NTL::IsZero(c3))
		    {
		      dt++;
		      NTL::GetCoeff(c3, r1, dr - dt);
		    }

		  dr -= dt;
		}

	      gamma = u1;
	      beta = r1;
	      NTL::rem(alpha, beta, m_privateKey->gZ());
	      sigma = NTL::power(alpha, 2) +
		NTL::power(gamma, 2) * m_privateKey->X();
	    }
	}

      NTL::vec_GF2 e;
      NTL::vec_GF2E L = m_privateKey->L();

      e.SetLength(n);

      for(long int i = 0; i < n; i++)
	if(NTL::IsZero(NTL::eval(sigma, L[i])))
	  e[i] = 1;

      ccar += e;

      NTL::vec_GF2 m;
      NTL::vec_GF2 mcar;
      NTL::vec_GF2 vec_GF2;
      std::vector<long int> swappingColumns(m_privateKey->swappingColumns());

      vec_GF2.SetLength(n);

      for(long int i = 0; i < n; i++)
	vec_GF2[i] = ccar[swappingColumns[i]];

      long int k = static_cast<long int> (m_k);

      mcar.SetLength(k);

      for(long int i = 0; i < k; i++)
	mcar[i] = vec_GF2[i + n - k];

      m = mcar * m_privateKey->Sinv();
      memset(p, 0, plaintext_size);

      for(long int i = 0, k = 0; i < static_cast<long int> (plaintext_size);
	  i++)
	{
	  std::bitset<CHAR_BIT> b;

	  for(long int j = 0; j < CHAR_BIT && k < m.length(); j++, k++)
	    b[static_cast<size_t> (j)] = m[k] == 0 ? 0 : 1;

	  p[i] = static_cast<char> (b.to_ulong());
	}

      plaintext << p;
    }
  catch(...)
    {
      delete []p;
      plaintext.clear();
      return false;
    }

  delete []p;
  return true;
}

bool mcnoodle::encrypt(const char *plaintext,
		       const size_t plaintext_size,
		       std::stringstream &ciphertext) const
{
  if(!m_publicKey || !m_publicKey->ok() || !plaintext || plaintext_size <= 0)
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

	  if(e[i] == 0)
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
      ciphertext.clear();
      return false;
    }

  return true;
}

bool mcnoodle::generatePrivatePublicKeys(void)
{
  delete m_privateKey;
  m_privateKey = 0;
  delete m_publicKey;
  m_publicKey = 0;
  m_privateKey = new (std::nothrow) mcnoodle_private_key(m_m, m_t);

  if(!m_privateKey)
    return false;

  m_publicKey = new (std::nothrow) mcnoodle_public_key(m_m, m_t);

  if(!m_publicKey)
    {
      delete m_privateKey;
      m_privateKey = 0;
      return false;
    }

  try
    {
      if(!m_privateKey->ok() || !m_publicKey->ok())
	throw std::exception();

      /*
      ** Create the parity-check matrix H.
      */

      NTL::GF2EX gZ = m_privateKey->gZ();
      NTL::mat_GF2 H;
      NTL::vec_GF2E L = m_privateKey->L();
      long int m = static_cast<long int> (m_m);
      long int n = static_cast<long int> (m_n);
      long int t = static_cast<long int> (m_t);

      H.SetDims(m * t, n);

      for(long int i = 0; i < t; i++)
	for(long int j = 0; j < n; j++)
	  {
	    NTL::GF2E gf2e = NTL::inv(NTL::eval(gZ, L[j])) *
	      NTL::power(L[j], i);
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

    done_label:

      /*
      ** H = [I|R], systematic form.
      ** More information at https://en.wikipedia.org/wiki/Generator_matrix.
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
		      if(H[k][j] == 1)
			{
			  pivot = false;
			  break;
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
		  {
		    m_privateKey->swapSwappingColumns(i, j);
		    break;
		  }
	      }
	  }

      NTL::mat_GF2 mat_GF2;
      std::vector<long int> swappingColumns(m_privateKey->swappingColumns());

      mat_GF2.SetDims(H.NumRows(), H.NumCols());

      for(long int i = 0; i < n; i++)
	for(long int j = 0; j < m * t; j++)
	  mat_GF2[j][i] = H[j][swappingColumns[i]];

      H = mat_GF2;

      NTL::mat_GF2 R;

      R.SetDims(m * t, n - m * t); // R^T has n - mt rows and mt columns.

      for(long int i = 0; i < R.NumRows(); i++)
	for(long int j = 0; j < R.NumCols(); j++)
	  R[i][j] = H[i][j + m * t];

      R = NTL::transpose(R);
      m_privateKey->prepareG(R);
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

  return true;
}
