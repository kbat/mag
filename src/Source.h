#ifndef Source_h
#define Source_h

#include <vector>
#include <TH2D.h>
#include <TVectorD.h>

class Source {
 private:
  const TH2D* hist;
  std::shared_ptr<TVectorD> vec;

  std::vector<double> buildRow();
 public:
  Source(const TH2D* h);
  std::shared_ptr<TH2D> Histogram() const;
  inline Int_t GetNrows() const { return vec->GetNrows(); }

  TVectorD &operator*=(const TMatrixD &source);
  TVectorD &operator+=(const TVectorD &source);
};

#endif
