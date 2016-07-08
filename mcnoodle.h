#ifndef _mcnoodle_h_
#define _mcnoodle_h_

#include <boost/numeric/ublas/matrix.hpp>

class mcnoodle
{
 public:
  mcnoodle(const size_t k, const size_t n, const size_t t);
  ~mcnoodle();
  void prepareP(void);
  void prepareS(void);

 private:
  boost::numeric::ublas::matrix<float> m_P;
  boost::numeric::ublas::matrix<float> m_S; /*
					    ** BOOST type-checking will
					    ** raise an exception unless
					    ** we're using real numbers.
					    */
  size_t m_k;
  size_t m_n;
  size_t m_t;
};

#endif
