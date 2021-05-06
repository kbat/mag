#include <Source.h>
#include <iostream>

Source::Source(const TH2D* h) :
  hist(h)
{
  std::vector<double> s = buildRow();
  vec = std::make_shared<TVectorD>(s.size(), s.data());
}

std::vector<double> Source::buildRow()
{
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

TVectorD &Source::operator*=(const TMatrixD &source)
{
  *vec *= source;
  return *vec;
}
