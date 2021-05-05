#include <Source.h>
#include <iostream>

Source::Source(const TH2D* h) :
  hist(h)
{
  std::vector<double> s = buildRow();
  S = new TVectorD(s.size(), s.data());
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

  return vec; // todo: how to avoid creating a new vector?
}

TH2D* Source::FillHist()
{
  TH2D *h = dynamic_cast<TH2D*>(hist->Clone("h"));
  h->Reset();

  const Int_t nx = h->GetNbinsX();
  const Int_t ny = h->GetNbinsY();

  for (Int_t j=0; j<ny; ++j)
    for (Int_t i=0; i<nx; ++i)
      h->SetBinContent(i,j, (*S)[i+j*nx]);

  return h;
}
