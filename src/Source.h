#ifndef Source_h
#define Source_h

#include <vector>
#include <TH2D.h>
#include <TVectorD.h>
//#include <Eigen/Dense>

//using Eigen::MatrixXd;

class Source {
 private:
  const TH2D* hist;
  std::shared_ptr<TVectorD> vec;
  //  std::shared_ptr<TVectorD> vec1;

  std::vector<double> buildRow();
 public:
  Source(const TH2D* h);
  Source(const Source&);
  std::shared_ptr<TH2D> Histogram() const;
  inline Int_t GetNrows() const { return vec->GetNrows(); }
  inline std::shared_ptr<TVectorD> GetVector() const { return vec; }

  Source   &operator=(const Source&);
  TVectorD &operator*=(const TMatrixD&);
  TVectorD &operator+=(const TVectorD&);
  TVectorD &operator+=(const Source&);
  friend Source operator+(Source lhs, const Source& rhs) { lhs += rhs; return rhs; }
};

/* std::ostream &operator<<(std::ostream &os, Source const &m) { */
/*   return os << "source"; */
/* }; */

#endif
