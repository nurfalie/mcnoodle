#ifndef _mcnoodle_h_
#define _mcnoodle_h_

#ifdef MCNOODLE_OS_UNIX
#include <NTL/GF2E.h>
#include <NTL/GF2EX.h>
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
  NTL::mat_GF2 m_P;
  NTL::mat_GF2 m_Pinv;
  NTL::mat_GF2 m_S;
  NTL::mat_GF2 m_Sinv;

 private:
  size_t m_d;
  size_t m_k;
  size_t m_m;
  size_t m_n;
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

 private:
  mcnoodle_private_key *m_privateKey;
  size_t m_k;
  size_t m_m;
  size_t m_n;
  size_t m_t;
};

#endif
