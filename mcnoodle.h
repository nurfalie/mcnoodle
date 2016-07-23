#ifndef _mcnoodle_h_
#define _mcnoodle_h_

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/LU>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/nondet_random.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/random.hpp>

class mcnoodle
{
 public:
  mcnoodle(const long int k, const long int n, const long int t);
  mcnoodle(const Eigen::MatrixXd &Gcar,
	   const long int k,
	   const long int n,
	   const long int t);
  ~mcnoodle();

  Eigen::MatrixXd P(void) const
  {
    return m_P;
  }

  Eigen::MatrixXd S(void) const
  {
    return m_S;
  }

  /*
  ** The contents of plaintext must be deallocated via delete [].
  */

  bool decrypt(const char *ciphertext, const size_t ciphertext_size,
	       char *&plaintext, size_t *plaintext_size);
  bool deserialize
    (const char *buffer, const size_t buffer_size,
     boost::numeric::ublas::matrix<double> &m);

  /*
  ** The contents of ciphertext must be deallocated via delete [].
  */

  bool encrypt(const char *plaintext, const size_t plaintext_size,
	       char *&ciphertext, size_t *ciphertext_size);

  /*
  ** The contents of buffer must be deallocated via delete [].
  */

  bool serialize(char *&buffer,
		 size_t *buffer_size,
		 const Eigen::MatrixXd &m);
  bool serialize(char *&buffer,
		 size_t *buffer_size,
		 const boost::numeric::ublas::matrix<double> &m);

  long int pSize(void) const
  {
    return m_P.rows(); // Square matrix.
  }

  long int sSize(void) const
  {
    return m_S.rows(); // Square matrix.
  }

  bool prepareG(void);
  bool prepareGcar(void);
  bool prepareP(void);
  bool prepareS(void);
  static bool equal(const Eigen::MatrixXd &m1, const Eigen::MatrixXd &m2);
  static bool equal(const Eigen::MatrixXd &m1,
		    const boost::numeric::ublas::matrix<double> &m2);

 private:
  Eigen::MatrixXd m_G;
  Eigen::MatrixXd m_Gcar;
#if MCNOODLE_ARTIFICIAL_GENERATOR
  Eigen::MatrixXd m_Ginv;
#endif
  Eigen::MatrixXd m_P;
  Eigen::MatrixXd m_Pinv;
  Eigen::MatrixXd m_S;
  Eigen::MatrixXd m_Sinv;
  long int m_k;
  long int m_n;
  long int m_t;

  long int minimumK(const long int k) const
  {
#ifdef MCNOODLE_ASSUME_SAFE_PARAMETERS
    return k;
#else
    return std::max(static_cast<long int> (644), k);
#endif
  }

  long int minimumN(const long int n) const
  {
#ifdef MCNOODLE_ASSUME_SAFE_PARAMETERS
    return n;
#else
    return std::max(static_cast<long int> (1024), n);
#endif
  }

  long int minimumT(const long int t) const
  {
#ifdef MCNOODLE_ASSUME_SAFE_PARAMETERS
    return t;
#else
    return std::max(static_cast<long int> (38), t);
#endif
  }
};

#endif
