#ifndef _mcnoodle_h_
#define _mcnoodle_h_

#ifdef MCNOODLE_OS_UNIX
#include <NTL/GF2E.h>
#include <NTL/GF2EX.h>
#include <NTL/GF2EXFactoring.h>
#include <NTL/GF2X.h>
#include <NTL/GF2XFactoring.h>
#include <NTL/mat_GF2.h>
#include <NTL/mat_ZZ_p.h>
#include <NTL/vec_GF2E.h>
#endif

#include <sstream>

class mcnoodle_private_key
{
 public:
  mcnoodle_private_key(const size_t m, const size_t t);
  ~mcnoodle_private_key();

  NTL::GF2EX g(void) const
  {
    return m_g;
  }

  NTL::mat_GF2 G(void) const
  {
    return m_G;
  }

  NTL::mat_GF2 P(void) const
  {
    return m_P;
  }

  NTL::mat_GF2 Pinv(void) const
  {
    return m_Pinv;
  }

  NTL::mat_GF2 S(void) const
  {
    return m_S;
  }

  NTL::mat_GF2 Sinv(void) const
  {
    return m_Sinv;
  }

  NTL::vec_GF2E L(void) const
  {
    return m_L;
  }

  bool prepareG(void);
  bool prepareP(void);
  bool prepareS(void);

 private:
  NTL::GF2E m_A;
  NTL::GF2EX m_g;
  NTL::GF2X m_mX;
  NTL::mat_GF2 m_G;
  NTL::mat_GF2 m_P;
  NTL::mat_GF2 m_Pinv;
  NTL::mat_GF2 m_S;
  NTL::mat_GF2 m_Sinv;
  NTL::vec_GF2E m_L;
  size_t m_k;
  size_t m_m;
  size_t m_n;
  size_t m_t;

  bool prepareIrreducibleGenerator(void)
  {
    try
      {
	long int t = static_cast<long int> (m_t);

	m_g = NTL::BuildRandomIrred(NTL::BuildIrred_GF2EX(t));
      }
    catch(...)
      {
	return false;
      }

    return true;
  }

  bool prepareMX(void)
  {
    try
      {
	long int m = static_cast<long int> (m_m);

	m_mX = NTL::BuildRandomIrred(NTL::BuildIrred_GF2X(m));
	NTL::GF2E::init(m_mX);
      }
    catch(...)
      {
	return false;
      }

    return true;
  }
};

class mcnoodle_public_key
{
 public:
  mcnoodle_public_key(const size_t m, const size_t t);
  ~mcnoodle_public_key();

  NTL::mat_GF2 Gcar(void) const
  {
    return m_Gcar;
  }

  bool prepareGcar(const NTL::mat_GF2 &G,
		   const NTL::mat_GF2 &P,
		   const NTL::mat_GF2 &S)
  {
    try
      {
	m_Gcar = S * G * P;
      }
    catch(...)
      {
	return false;
      }

    return true;
  }

 private:
  NTL::mat_GF2 m_Gcar;
  size_t m_t;
};

class mcnoodle
{
 public:
  mcnoodle(const size_t m, const size_t t);
  mcnoodle(const size_t m,
	   const size_t t,
	   const std::stringstream &G,
	   const std::stringstream &P,
	   const std::stringstream &S);
  ~mcnoodle();
  bool decrypt(const std::stringstream &ciphertext,
	       std::stringstream &plaintext);
  bool encrypt(const char *plaintext, const size_t plaintext_size,
	       std::stringstream &ciphertext);
  bool generatePrivatePublicKeys(void);

  static size_t minimumM(const size_t m)
  {
#ifdef MCNOODLE_ASSUME_SAFE_PARAMETERS
    return m;
#else
    return std::max(static_cast<size_t> (10), m);
#endif
  }

  static size_t minimumT(const size_t t)
  {
#ifdef MCNOODLE_ASSUME_SAFE_PARAMETERS
    return t;
#else
    return std::max(static_cast<size_t> (38), t);
#endif
  }

  void initializeSystemParameters(const size_t m, const size_t t)
  {
    m_m = minimumM(m);
    m_n = 1 << m_m; // 2^m
    m_t = minimumM(t);

    /*
    ** Some calculations.
    */

    m_k = m_n - m_m * m_t;
  }

  void privateKeyParameters(std::stringstream &G,
			    std::stringstream &P,
			    std::stringstream &S);
  void publicKeyParameters(size_t &t, std::stringstream &Gcar);

 private:
  mcnoodle_private_key *m_privateKey;
  mcnoodle_public_key *m_publicKey;
  size_t m_k;
  size_t m_m;
  size_t m_n;
  size_t m_t;
};

#endif
