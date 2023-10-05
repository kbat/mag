#include <iostream>

#ifdef USE_CUDA1
#include "kernel.h"
#endif

#include "Source.h"

Source::Source(const TH2D* h) :
  hist(h)
{
  // Constructor

  std::vector<double> s = buildRow();
  vec = std::make_shared<TVectorD>(s.size(), s.data());
}

Source::Source(const Source& A) :
  hist(A.hist),
  vec(A.vec)
{
  // Copy constructor
  //  std::cout << "Source: Copy constructor" << std::endl;
}

std::vector<double> Source::buildRow()
{
  // Build transport matrix row based on the source histogram

  const Int_t nx = hist->GetNbinsX();
  const Int_t ny = hist->GetNbinsY();
  std::vector<double> vec;
  vec.reserve(nx*ny);

  for (Int_t j=1; j<=ny; ++j)
    for (Int_t i=1; i<=nx; ++i)
      vec.push_back(hist->GetBinContent(i,j));

  return vec;
}

std::shared_ptr<TH2D> Source::Histogram(const std::string& name) const
{
  // Fills TH2 based on TVectorD

  std::shared_ptr<TH2D> h = std::make_shared<TH2D>(*hist);
  h->Reset();
  h->SetName(name.data());

  const Int_t nx = h->GetNbinsX();
  const Int_t ny = h->GetNbinsY();

  for (Int_t j=0; j<ny; ++j)
    for (Int_t i=0; i<nx; ++i)
      h->SetBinContent(i+1,j+1, (*vec)[i+j*nx]);

  return h;
}

Source& Source::operator=(const Source& A)
{
  // Assignment operator
  std::cout << "Source: Assignment operator" << std::endl;
  if (this != &A)
    {
      hist = A.hist;
      vec  = A.vec;
    }
  return *this; // return result by reference
}

TVectorD &Source::operator*=(const TMatrixD &m)
{
  // Multiplication assignment operator
  // reimplemented since in ROOT a *= b return b*a but not a*b
  // it's slow, will replace it with Eigen matrixes OR see efficient ROOT matrix techniques TODO

#ifdef USE_CUDA1
  std::cout << "cuda1" << std::endl;
  matrixMultiplication();
  return *vec;
#else
  std::cout << "not cuda1" << std::endl;
#endif

  const size_t n = vec->GetNrows();
  std::shared_ptr<TVectorD> vec1 = std::make_shared<TVectorD>(n);

  for (size_t i=0; i<n; ++i) {
    double sum=0.0;
    for (size_t j=0; j<n; ++j) {
      sum += (*vec)[j]*m[j][i];
      //      std::cout << (*vec)[j] << " * " << m[j][i] << std::endl;
    }
    (*vec1)[i] = sum;
  }
  vec = vec1;

  return *vec;
}

TVectorD &Source::operator+=(const TVectorD &source)
{
  // Addition assignment operator

  *vec += source;
  return *vec;
}

TVectorD &Source::operator+=(const Source &s)
{
  // Addition assignment operator

  *vec += *s.vec;
  return *vec;
}
