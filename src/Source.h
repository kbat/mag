#ifndef Source_h
#define Source_h

#include <vector>
#include <TH2D.h>
#include <TVectorD.h>

class Source {
 private:
  const TH2D* hist;
  TVectorD *S;

  std::vector<double> buildRow();
 public:
  Source(const TH2D* h);
  std::shared_ptr<TH2D> FillHist() const;
  inline Int_t GetNrows() const { return S->GetNrows(); }

  TVectorD &operator*=(const TMatrixD &source);
};

#endif
