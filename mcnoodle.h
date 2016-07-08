#ifndef _mcnoodle_h_
#define _mcnoodle_h_

#include <boost/numeric/ublas/matrix.hpp>

typedef int mcnoodble_matrix_element_type_t;

class mcnoodle
{
 public:
  mcnoodle(const std::size_t k, const std::size_t n, const std::size_t t);
  ~mcnoodle();
  void prepareP(void);
  void prepareS(void);

 private:
  boost::numeric::ublas::matrix<mcnoodble_matrix_element_type_t> m_P;
  boost::numeric::ublas::matrix<mcnoodble_matrix_element_type_t> m_Pinv;
  boost::numeric::ublas::matrix<mcnoodble_matrix_element_type_t> m_S;
  boost::numeric::ublas::matrix<mcnoodble_matrix_element_type_t> m_Sinv;
  std::size_t m_k;
  std::size_t m_n;
  std::size_t m_t;
};

#endif
