#include <iostream>
#include <chrono>

#include <TROOT.h>
#include <TH2D.h>
#include <TFile.h>

#include "Material.h"
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
  auto poly = std::make_shared<Material>("Polyethylene", "Poly.root");

  std::vector<std::shared_ptr<Material>> mat;
  for (size_t i=0; i<50; ++i)
    mat.push_back(poly);

  // const double E0 = 53.2785;
  // const double mu0 = 0.95;
  const double E0 = 707.732;
  const double mu0 = 0.05;

  auto sdef = mat[0]->getSDEF();
  sdef->Fill(E0, mu0);

  const char p0 = 'e'; // incident particle
  auto particles = mat[0]->getParticles();

  auto res = std::make_shared<Source>(sdef.get()); // result

  // first layer
  std::map<char, std::shared_ptr<Source> > source1;

  for (auto p : particles)
    source1.insert(std::make_pair(p, std::make_shared<Source>(sdef.get())));

  for (size_t i=0; i<50; ++i) {
    *res *= *mat[i]->get("eTe");
  }

  std::shared_ptr<TH2D> h = res->Histogram();

  h->SaveAs("res.root");

  return 0;
}
