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
  auto concrete = std::make_shared<Material>("Concrete", "Concrete.root");

  auto mTest1    = std::make_shared<Material>("Test", "test/solver/test1.root");
  auto mTest2    = std::make_shared<Material>("Test", "test/solver/test2.root");
  auto mTest3    = std::make_shared<Material>("Test", "test/solver/test3.root");

  size_t nLayers = 10;
  std::vector<std::shared_ptr<Material>> mat;

  // tests
  size_t test = 0;
  if (argc==3) { // test nLayers
    if (!strcmp(argv[1], "test1")) {
      test = 1;
      nLayers = atoi(argv[2]);
      for (size_t i=0; i<nLayers; ++i)
	mat.push_back(mTest1);
    } else if (!strcmp(argv[1], "test2")) {
      test = 2;
      nLayers = atoi(argv[2]);
      for (size_t i=0; i<nLayers; ++i)
	mat.push_back(mTest2);
    } else if (!strcmp(argv[1], "test3")) {
      test = 3;
      nLayers = atoi(argv[2]);
      for (size_t i=0; i<nLayers; ++i)
	mat.push_back(mTest3);
    } else {
      std::cerr << "usage: " << argv[0] << " test[12] nLayers" << std::endl;
      return 1;
    }
  }
  else {
    for (size_t i=0; i<nLayers; ++i)
      mat.push_back(poly);
  }

  // const double E0 = 53.2785;
  // const double mu0 = 0.95;
  const double E0 = 2e3;
  const double mu0 = 0.99;

  auto sdef = mat[0]->getSDEF();
  if (!test)
    sdef->Fill(E0, mu0);

  const char p0 = 'n'; // incident particle
  auto particles = mat[0]->getParticles();

  size_t layer=0;
  // LAYER 0
  // incident particle is e, but we still need individual sources for each particle,
  // they will be used at sources for the next layer
  std::map<char, std::shared_ptr<Source> > spectra1;

  for (auto p : particles) {
    spectra1.insert(std::make_pair(p, std::make_shared<Source>(sdef.get())));
    *spectra1[p] *= *mat[layer]->getT(p0, p);
  }

  // Now spectra1 contains spectra of individual particles leaving the first layer
  std::map<char, std::map<char, std::shared_ptr<Source> > > spectra2;

  for (size_t layer=1; layer<nLayers; ++layer) {
    spectra2.clear();

    // define all combinations of spectra after the 2nd layer
    // but before we do transport, we just copy data from spectra1
    // because they will be the corresponding sdefs
    for (auto i : particles)   // incident
      for (auto j : particles)  // scored
	spectra2[i].insert(std::make_pair(j,
					  std::make_shared<Source>(*spectra1[i])));

    // transport through the 2nd layer (combine both series of loops in the future)
    for (auto i : particles)   // incident
      for (auto j : particles)  // scored
	*spectra2[i][j] *= *mat[layer]->getT(i,j);

    spectra1.clear();

    // add up spectra of each secondary particle produced by different incidents
    for (auto i : particles) {
      spectra1[i] = std::make_shared<Source>(*spectra2[i][i]);
      for (auto j : particles)
	if (i!=j)
	  *spectra1[i] += *spectra2[j][i];
    }
  }


  TFile fout("res.root", "recreate");

  for (auto p : particles)
    spectra1[p]->Histogram(std::string(1, p))->Write();

  fout.Close();

  return 0;
}
