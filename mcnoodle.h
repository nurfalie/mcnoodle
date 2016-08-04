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
  bool prepareP(void);
  bool prepareS(void);
  NTL::GF2E m_A;
  NTL::GF2EX m_g;
  NTL::GF2X m_polynomial;
  NTL::mat_GF2 m_P;
  NTL::mat_GF2 m_Pinv;
  NTL::mat_GF2 m_S;
  NTL::mat_GF2 m_Sinv;
  NTL::vec_GF2E m_L;

 private:
  size_t m_d;
  size_t m_k;
  size_t m_m;
  size_t m_n;
  size_t m_t;

  void prepareIrreducibleGenerator(void)
  {
    long int t = static_cast<long int> (m_t);

    m_g = NTL::BuildRandomIrred(NTL::BuildIrred_GF2EX(t));
  }

  void prepareIrreduciblePolynomial(void)
  {
    long int m = static_cast<long int> (m_m);

    m_polynomial = NTL::BuildRandomIrred(NTL::BuildIrred_GF2X(m));
    NTL::GF2E::init(m_polynomial);
  }
};

class mcnoodle_public_key
{
 public:
  mcnoodle_public_key(const size_t t);
  ~mcnoodle_public_key();
  NTL::mat_GF2 m_Gcar;

 private:
  size_t m_t;
};

class mcnoodle
{
 public:
  mcnoodle(const size_t m, const size_t t);
  ~mcnoodle();
  bool decrypt(const std::stringstream &ciphertext,
	       std::stringstream &plaintext);
  bool encrypt(const char *plaintext, const size_t plaintext_size,
	       std::stringstream &ciphertext);

  static size_t minimumM(const size_t m)
  {
#ifdef MCNOODLE_ASSUME_SAFE_PARAMETERS
    return m;
#else
    return std::max(static_cast<size_t> (1), m);
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

  void generateKeyPair(void);

 private:
  NTL::GF2X m_mX;
  mcnoodle_private_key *m_privateKey;
  mcnoodle_public_key *m_publicKey;
  size_t m_k;
  size_t m_m;
  size_t m_n;
  size_t m_t;
};

#endif
