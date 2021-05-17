#include <iostream>
#include <chrono>

#include <TROOT.h>
#include <TH2D.h>
#include <TFile.h>

#include "Material.h"
#include "Source.h"
#include "Solver.h"

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
  auto poly = std::make_shared<Material>("Polyethylene", "Poly.root", 0.91);
  auto concrete = std::make_shared<Material>("Concrete", "Concrete.root", 2.33578);

  size_t nLayers = argc == 2 ? atoi(argv[1]) : 10;
  //  std::cout << "nLayers: " << nLayers << std::endl;
  const char p0 = 'n'; // incident particle
  const double E0 = 2e3;
  const double mu0 = 0.99;

  std::vector<std::shared_ptr<Material>> mat;

  // tests
  if (argc==3) { // test nLayers
    //    std::cout << "test" << std::endl;
    auto mTest1    = std::make_shared<Material>("Test", "test/solver/test1.root", 1);
    auto mTest2    = std::make_shared<Material>("Test", "test/solver/test2.root", 1);
    auto mTest3    = std::make_shared<Material>("Test", "test/solver/test3.root", 1);
    auto mTest5    = std::make_shared<Material>("Test", "test/solver/test5.root", 1);
    auto mTest6    = std::make_shared<Material>("Test", "test/solver/test6.root", 1);
    size_t ro=0; // reflection order
    if (!strcmp(argv[1], "test1")) {
      nLayers = atoi(argv[2]);
      for (size_t i=0; i<nLayers; ++i)
	mat.push_back(mTest1);
    } else if (!strcmp(argv[1], "test2")) {
      nLayers = atoi(argv[2]);
      for (size_t i=0; i<nLayers; ++i)
	mat.push_back(mTest2);
    } else if (!strcmp(argv[1], "test3")) {
      nLayers = atoi(argv[2]);
      for (size_t i=0; i<nLayers; ++i)
	mat.push_back(mTest3);
    } else if (!strcmp(argv[1], "test4")) {
      nLayers = atoi(argv[2]);
      ro = 1;
      for (size_t i=0; i<nLayers; ++i)
	mat.push_back(mTest1);
    } else if (!strcmp(argv[1], "test5")) {
      nLayers = atoi(argv[2]);
      ro = 1;
      for (size_t i=0; i<nLayers; ++i)
	mat.push_back(mTest5);
    } else if (!strcmp(argv[1], "test6")) {
      nLayers = atoi(argv[2]);
      ro = 1;
      for (size_t i=0; i<nLayers; ++i)
	mat.push_back(mTest6);
    } else {
      std::cerr << "usage: " << argv[0] << " test[123456] nLayers" << std::endl;
      return 1;
    }
    auto solver = std::make_shared<Solver>('n', mat[0]->getSDEF(), mat);
    solver->run(ro);
    solver->save("res.root");
    return 0;
  }

  for (size_t i=0; i<nLayers; ++i)
    mat.push_back(poly);

  auto sdef = mat[0]->getSDEF();
  sdef->Fill(E0, mu0);


  auto solver = std::make_shared<Solver>(p0, sdef, mat);
  solver->run(1);
  solver->save("res.root");

  static std::set<char> ftd {'n', 'p', 'e', '|'};
  for (auto p : ftd)
    std::cout << p << " " << solver->getDose(p) << std::endl;

  std::cerr << "total: " << solver->getDose() << std::endl;

  return 0;
}
