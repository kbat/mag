#include <iostream>
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
  std::cout << "Source: Copy constructor" << std::endl;
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

std::shared_ptr<TH2D> Source::Histogram() const
{
  // Fills TH2 based on TVectorD

  std::shared_ptr<TH2D> h = std::make_shared<TH2D>(*hist);
  h->Reset();
  h->SetName("h");

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

  *vec *= m;
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
