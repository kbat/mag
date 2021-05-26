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
  std::vector<double> buildRow();
 public:
  Source(const TH2D* h);
  Source(const Source&);
  //  virtual ~Source() { std::cout << "Source destructor" << std::endl; }
  std::shared_ptr<TH2D> Histogram(const std::string&) const;
  inline Int_t GetNrows() const { return vec->GetNrows(); }
  inline std::shared_ptr<TVectorD> GetVector() const { return vec; }

  Source   &operator=(const Source&);
  TVectorD &operator*=(const TMatrixD&);
  TVectorD &operator+=(const TVectorD&);
  TVectorD &operator+=(const Source&);
  friend Source operator+(Source lhs, const Source& rhs) { lhs += rhs; return rhs; }
  friend std::ostream &operator<<(std::ostream &os, Source const &m) {
    const Int_t n = m.GetVector()->GetNrows();
    for (Int_t i=0; i<n; i++)
      os << " " << (*m.GetVector())[i];
    return os;
  };
};


#endif
