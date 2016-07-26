#ifndef _mcnoodle_h_
#define _mcnoodle_h_

#ifdef MCNOODLE_OS_UNIX
#include <NTL/mat_GF2.h>
#include <NTL/mat_ZZ_p.h>
#endif

#include <sstream>

class mcnoodle
{
 public:
  mcnoodle(const size_t m, const size_t t);
  ~mcnoodle();
  bool decrypt(const std::stringstream &ciphertext,
	       std::stringstream &plaintext);
  bool encrypt(const char *plaintext, const size_t plaintext_size,
	       std::stringstream &ciphertext);
  bool prepareG(void);
  bool prepareGcar(void);
  bool prepareP(void);
  bool prepareS(void);

 private:
  NTL::mat_GF2 m_G;
  NTL::mat_GF2 m_Gcar;
  NTL::mat_GF2 m_Ginv;
  NTL::mat_GF2 m_P;
  NTL::mat_GF2 m_Pinv;
  NTL::mat_GF2 m_S;
  NTL::mat_GF2 m_Sinv;
  size_t m_d;
  size_t m_k;
  size_t m_m;
  size_t m_n;
  size_t m_t;

  size_t minimumM(const size_t m) const
  {
#ifdef MCNOODLE_ASSUME_SAFE_PARAMETERS
    return m;
#else
    return std::max(static_cast<size_t> (1), m);
#endif
  }

  size_t minimumT(const size_t t) const
  {
#ifdef MCNOODLE_ASSUME_SAFE_PARAMETERS
    return t;
#else
    return std::max(static_cast<size_t> (38), t);
#endif
  }
};

#endif
