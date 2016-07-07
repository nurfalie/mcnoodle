#ifndef _mcnoodle_h_
#define _mcnoodle_h_

#include <boost/numeric/ublas/matrix.hpp>

class mcnoodle
{
 public:
  mcnoodle(const size_t k, const size_t n, const size_t t);
  ~mcnoodle();

 private:
  boost::numeric::ublas::matrix<int> S;
  size_t m_k;
  size_t m_n;
  size_t m_t;
  void prepareS(void);
};

#endif
