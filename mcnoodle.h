#ifndef _mcnoodle_h_
#define _mcnoodle_h_

#include <algorithm>
#include <boost/numeric/ublas/matrix.hpp>

typedef char mcnoodle_matrix_element_type_t;

class mcnoodle
{
 public:
  mcnoodle(const size_t k, const size_t n, const size_t t);
  mcnoodle
    (const boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> &Gcar,
     const size_t k,
     const size_t n,
     const size_t t);
  ~mcnoodle();

  /*
  ** The contents of plaintext must be deallocated via delete [].
  */

  bool decrypt(const char *ciphertext, const size_t ciphertext_size,
	       char *&plaintext, size_t *plaintext_size);
  bool deserialize
    (const char *buffer, const size_t buffer_size,
     boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> &m);

  /*
  ** The contents of ciphertext must be deallocated via delete [].
  */

  bool encrypt(const char *plaintext, const size_t plaintext_size,
	       char *&ciphertext, size_t *ciphertext_size);

  /*
  ** The contents of buffer must be deallocated via delete [].
  */

  bool serialize
    (char *&buffer,
     size_t *buffer_size,
     const boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> &m);

  boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> P(void) const
  {
    return m_P;
  }

  size_t pSize(void) const
  {
    return m_P.size1(); // Square matrix.
  }

  size_t sSize(void) const
  {
    return m_S.size1(); // Square matrix.
  }

  static bool equal
    (const boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> &m1,
     const boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> &m2);
  void prepareP(void);
  void prepareS(void);

 private:
  boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> m_Gcar;
  boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> m_P;
  boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> m_Pinv;
  boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> m_S;
  boost::numeric::ublas::matrix<mcnoodle_matrix_element_type_t> m_Sinv;
  size_t m_k;
  size_t m_n;
  size_t m_t;

  size_t minimumK(const size_t k) const
  {
    return std::max(static_cast<size_t> (644), k);
  }

  size_t minimumN(const size_t n) const
  {
    return std::max(static_cast<size_t> (1024), n);
  }

  size_t minimumT(const size_t t) const
  {
    return std::max(static_cast<size_t> (38), t);
  }
};

#endif
