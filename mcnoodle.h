#ifndef _mcnoodle_h_
#define _mcnoodle_h_

#ifdef MCNOODLE_OS_UNIX
#include <NTL/mat_ZZ_p.h>
#endif

class mcnoodle
{
 public:
  mcnoodle(const size_t k, const size_t n, const size_t t);
  ~mcnoodle();

  /*
  ** The contents of plaintext must be deallocated via delete [].
  */

  bool decrypt(const char *ciphertext, const size_t ciphertext_size,
	       char *&plaintext, size_t *plaintext_size);

  /*
  ** The contents of ciphertext must be deallocated via delete [].
  */

  bool encrypt(const char *plaintext, const size_t plaintext_size,
	       char *&ciphertext, size_t *ciphertext_size);

  bool prepareG(void);
  bool prepareGcar(void);
  bool prepareP(void);
  bool prepareS(void);

 private:
  NTL::mat_ZZ_p m_S;
  NTL::mat_ZZ_p m_Sinv;
  size_t m_k;
  size_t m_n;
  size_t m_t;

  size_t minimumK(const size_t k) const
  {
#ifdef MCNOODLE_ASSUME_SAFE_PARAMETERS
    return k;
#else
    return std::max(static_cast<size_t> (644), k);
#endif
  }

  size_t minimumN(const size_t n) const
  {
#ifdef MCNOODLE_ASSUME_SAFE_PARAMETERS
    return n;
#else
    return std::max(static_cast<size_t> (1024), n);
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
