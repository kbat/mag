#include <iostream>
#include <chrono>
#include <string>

#include <TROOT.h>
#include <TH2D.h>
#include <TFile.h>

#include "Test.h"
#include "Material.h"
#include "Source.h"
#include "Solver.h"
#include "SolverArguments.h"

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

int main(int argc, const char **argv)
{
  std::unique_ptr<SolverArguments> args = std::make_unique<SolverArguments>(argc, argv);

  if (args->IsHelp())
    return 0;

  if (args->IsTest()) {
    auto t = args->GetMap()["test"].as<std::vector<size_t> >();
    std::unique_ptr<Test> test = std::make_unique<Test>(t[0], t[1]);
    return test->run();
  }

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

  if (args->IsMaterials()) {
    std::cout << "Supported materials:" << std::endl;
    for (auto m : mat)
      std::cout << m.first << std::endl;
    return 0;
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
