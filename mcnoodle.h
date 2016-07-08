#ifndef _mcnoodle_h_
#define _mcnoodle_h_

#include <boost/numeric/ublas/matrix.hpp>

typedef int mcnoodble_matrix_element_type_t;

class mcnoodle
{
 public:
  mcnoodle(const std::size_t k, const std::size_t n, const std::size_t t);
  ~mcnoodle();

  size_t pSize(void) const
  {
    return m_P.size1(); // Square matrix.
  }

  size_t sSize(void) const
  {
    return m_S.size1(); // Square matrix.
  }

  void prepareP(void);
  void prepareS(void);
  void serializeP(char *buffer, const size_t buffer_size);
  void serializePinv(char *buffer, const size_t buffer_size);
  void serializeS(char *buffer, const size_t buffer_size);
  void serializeSinv(char *buffer, const size_t buffer_size);

 private:
  boost::numeric::ublas::matrix<mcnoodble_matrix_element_type_t> m_P;
  boost::numeric::ublas::matrix<mcnoodble_matrix_element_type_t> m_Pinv;
  boost::numeric::ublas::matrix<mcnoodble_matrix_element_type_t> m_S;
  boost::numeric::ublas::matrix<mcnoodble_matrix_element_type_t> m_Sinv;
  std::size_t m_k;
  std::size_t m_n;
  std::size_t m_t;
  void serialize
    (char *buffer,
     const size_t buffer_size,
     const boost::numeric::ublas::matrix<mcnoodble_matrix_element_type_t> &m);
};

#endif
