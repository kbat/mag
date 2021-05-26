#include <iostream>
#include <chrono>
#include <string>

#include <TROOT.h>
#include <TH2D.h>
#include <TFile.h>

#include "Material.h"
#include "Source.h"
#include "Solver.h"
#include "SolverArguments.h"

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

bool test(size_t n, size_t nLayers)
/*!
  Run some tests
 */
{
  bool val(false);

  //  std::shared_ptr<Material> m;
  std::vector<std::shared_ptr<Material>> mat;

  auto mTest1 = std::make_shared<Material>("Test", "test/solver/test1.root", 1, 1);
  auto mTest2 = std::make_shared<Material>("Test", "test/solver/test2.root", 2, 1);
  auto mTest3 = std::make_shared<Material>("Test", "test/solver/test3.root", 3, 1);
  auto mTest5 = std::make_shared<Material>("Test", "test/solver/test5.root", 4, 1);
  auto mTest6 = std::make_shared<Material>("Test", "test/solver/test6.root", 5, 1);

  size_t ro=0; // reflection order
  if (n==1) {
    for (size_t i=0; i<nLayers; ++i)
      mat.push_back(mTest1);
  } else if (n==2) {
    for (size_t i=0; i<nLayers; ++i)
      mat.push_back(mTest2);
  } else if (n==3) {
    for (size_t i=0; i<nLayers; ++i)
      mat.push_back(mTest3);
  } else if (n==4) {
    ro = 1;
    for (size_t i=0; i<nLayers; ++i)
      mat.push_back(mTest1);
  } else if (n==5) {
    ro = 1;
    for (size_t i=0; i<nLayers; ++i)
      mat.push_back(mTest5);
  } else if (n==6) {
    ro = 1;
    for (size_t i=0; i<nLayers; ++i)
      mat.push_back(mTest6);
  } else {
    std::cerr << "gam-solver: test error" << std::endl;
    return false;
  }
  auto solver = std::make_shared<Solver>('n', mat[0]->getSDEF(), mat);
  solver->run(ro);
  solver->save("res.root");

  return val;
}

int main(int argc, const char **argv)
{
  std::unique_ptr<SolverArguments> args = std::make_unique<SolverArguments>(argc, argv);

  if (args->IsHelp())
    return 0;

  if (args->IsTest()) {
    auto t = args->GetMap()["test"].as<std::vector<size_t> >();
    return test(t[0], t[1]);
  }

  auto config = args->GetMap()["layers"].as<std::vector<std::string> >();

  size_t n(1);
  std::vector<std::string> vlayers;
  for (auto l : config)
    {
      if (is_number(l))
	  n = std::stoi(l);
      else
	for (size_t i=0; i<n; ++i)
	  vlayers.push_back(l);
    }

  const size_t nLayers = vlayers.size();
  std::cout << nLayers << (nLayers == 1 ? " layer: " : " layers: ");
  for (auto l : vlayers)
    std::cout << l << " ";
  std::cout << std::endl;

  std::map<std::string, std::shared_ptr<Material> > mat;
  mat.insert(std::make_pair("Poly",
				  std::make_shared<Material>("Poly", "Poly.root", 48, 0.91)));
  mat.insert(std::make_pair("W",
				  std::make_shared<Material>("Tungsten", "Tungsten.root", 38, 19.413)));
  // mat.insert(std::make_pair("Concrete",
  // 			    std::make_shared<Material>("Concrete", "Concrete.root", 49, 2.33578)));
  mat.insert(std::make_pair("B4C",
			    std::make_shared<Material>("B4C", "B4C.root", 47, 2.50608)));
  mat.insert(std::make_pair("Stainless304",
			    std::make_shared<Material>("Stainless304",
						       "Stainless304.root", 3, 7.96703)));

  const char p0 = 'e'; // incident particle
  const double E0 = 3e3;
  const double mu0 = 0.999;

  std::vector<std::shared_ptr<Material>> layers;

  for (auto l : vlayers)
    layers.push_back(mat[l]);

  auto sdef = layers[0]->getSDEF();
  sdef->Fill(E0, mu0);

  auto solver = std::make_shared<Solver>(p0, sdef, layers);
  solver->run(1);
  solver->save("res.root");

  static std::set<char> ftd {'n', 'p', 'e', '|'};
  for (auto p : ftd)
    std::cout << p << " " << solver->getDose(p) << "\t";

  std::cerr << "total: " << solver->getDose() << std::endl;

  return 0;
}
