#include <iostream>
#include <chrono>

#include <TROOT.h>
#include <TH2D.h>
#include <TFile.h>

#include "Source.h"

std::shared_ptr<TMatrixD> GetReflectionsT(std::shared_ptr<TMatrixD> T,std::shared_ptr<TMatrixD> Tn,
					  std::shared_ptr<TMatrixD> R,std::shared_ptr<TMatrixD> Rn,
					  const size_t order=1)
{
  assert(order==1 && "Orders > 1 not supported yet");

  std::shared_ptr<TMatrixD> m = std::make_shared<TMatrixD>(*Tn); // todo: check if not cloned

  *m *= *R;
  *m *= *Rn;
  *m *= *T;

  return m;
}

std::shared_ptr<TMatrixD> GetReflectionsR(std::shared_ptr<TMatrixD> T,std::shared_ptr<TMatrixD> Tn,
					  std::shared_ptr<TMatrixD> R,std::shared_ptr<TMatrixD> Rn,
					  const size_t order=1)
{
  assert(order==1 && "Orders > 1 not supported yet");

  std::shared_ptr<TMatrixD> m = std::make_shared<TMatrixD>(*T);

  *m *= *Rn;
  *m *= *T;

  return m;
}

std::shared_ptr<TMatrixD> GetNextT(std::shared_ptr<TMatrixD> T,std::shared_ptr<TMatrixD> Tn,
				   std::shared_ptr<TMatrixD> R,std::shared_ptr<TMatrixD> Rn,
				   const size_t order=1)
{
  // Calculate next iteration T matrix

  std::shared_ptr<TMatrixD> m = std::make_shared<TMatrixD>(*Tn);

  *m *= *T;
  *m += *GetReflectionsT(T, Tn, R, Rn, order);

  return m;
}

std::shared_ptr<TMatrixD> GetNextR(std::shared_ptr<TMatrixD> T,std::shared_ptr<TMatrixD> Tn,
				   std::shared_ptr<TMatrixD> R,std::shared_ptr<TMatrixD> Rn,
				   const size_t order=1)
{
  // Calculate next iteration R matrix

  std::shared_ptr<TMatrixD> m = std::make_shared<TMatrixD>(*R);

  *m += *GetReflectionsR(T, Tn, R, Rn, order);

  return m;
}

std::shared_ptr<TMatrixD> h2m(const TH2D *h)
{
  // Convert TH2 to TMatrix

  const Int_t nx = h->GetNbinsX();
  const Int_t ny = h->GetNbinsY();

  std::shared_ptr<TMatrixD> m = std::make_shared<TMatrixD>(nx, ny);
  for (Int_t i=0; i<nx; ++i)
    for (Int_t j=0; j<ny; ++j)
      (*m)[i][j] = h->GetBinContent(i+1, j+1);

  return m;
}

void SaveMatrices(std::shared_ptr<TMatrixD> T, std::shared_ptr<TMatrixD> R,
		  const char* fname="RT-cxx.root")
{
  TFile fout(fname, "recreate");

  std::shared_ptr<TH2D> hT = std::make_shared<TH2D>(*T.get());
  hT->SetName("T");
  std::shared_ptr<TH2D> hR = std::make_shared<TH2D>(*R.get());
  hR->SetName("R");

  hT->Write();
  hR->Write();
  fout.Close();
}

int main(int argc, const char **argv)
{
  TFile f("gam.root");
  const TH2D *hT = f.Get<const TH2D>("T");
  const TH2D *hR = f.Get<const TH2D>("R");
  TH2D *h0 = f.Get<TH2D>("h0");
  h0->SetDirectory(0);

  // todo: nbins[xy]+2 ???
  const std::shared_ptr<TMatrixD> T = h2m(hT);
  const std::shared_ptr<TMatrixD> R = h2m(hR);

  f.Close();

  std::shared_ptr<TMatrixD> T1 = std::make_shared<TMatrixD>(*T);
  std::shared_ptr<TMatrixD> R1 = std::make_shared<TMatrixD>(*R);

  // const double E0 = 53.2785;
  // const double mu0 = 0.95;
  const double E0 = 707.732;
  const double mu0 = 0.05;

  h0->Fill(E0, mu0);

  std::unique_ptr<Source> s = std::make_unique<Source>(h0);
  std::unique_ptr<Source> res = std::make_unique<Source>(h0); // todo: use copy constructor here

  for (size_t i=0; i<50-1; ++i) {
    T1 = GetNextT(T, T1, R, R1);
    R1 = GetNextR(T, T1, R, R1);
  }

  std::cout << "Result rows: " << res->GetNrows() << std::endl;
  std::cout << "Matrix: " << T1->GetNrows() << " x " << T1->GetNcols() << std::endl;

  SaveMatrices(T1, R1);

  *res *= *T1;

  std::shared_ptr<TH2D> h = res->Histogram();

  h->SaveAs("res.root");

  return 0;
}
