#include <iostream>
#include <chrono>
#include <string>

#include "SDEF.h"
#include "Test.h"
#include "Material.h"
#include "Source.h"
#include "Solver.h"
#include "SolverArguments.h"
#include "functions.h"

int main(int argc, const char **argv)
{
  std::unique_ptr<SolverArguments> args = std::make_unique<SolverArguments>(argc, argv);

  if (args->IsHelp())
    return 0;

  // get sdef
  std::map<ParticleID, std::shared_ptr<TH2D>> sdef;
  const auto vsdef = args->GetMap()["sdef"].as<std::vector<std::string> >();

  if (args->IsTest()) {
    auto t = args->GetMap()["test"].as<std::vector<size_t> >();
    std::unique_ptr<Test> test = std::make_unique<Test>(t[0], t[1]);
    if (vsdef.size()==1) {
      auto s = std::make_unique<SDEF>(vsdef[0]);
      sdef = s->getSDEF();
      test->setSDEF(sdef);
    }
    return test->run();
  }

  std::set<std::shared_ptr<Material> > matdb;
  matdb.insert(std::make_shared<Material>("Stainless304", "Stainless304.root", 3, 7.96703));
  matdb.insert(std::make_shared<Material>("Water", "Water.root", 11, 1.0));
  matdb.insert(std::make_shared<Material>("Lead", "Lead.root", 23, 11.1837));
  matdb.insert(std::make_shared<Material>("W", "Tungsten.root", 38, 19.413));
  matdb.insert(std::make_shared<Material>("B4C", "B4C.root", 47, 2.50608));
  matdb.insert(std::make_shared<Material>("Poly", "Poly.root", 48, 0.91));
  matdb.insert(std::make_shared<Material>("Concrete", "Concrete.root", 49, 2.33578));

  if (args->IsMaterials()) {
    print_materials(matdb);
    return 0;
  }

  const auto layout = args->GetMap()["layers"].as<std::vector<std::string> >();
  size_t n(1);
  std::vector<std::string> vlayers;
  for (auto l : layout)
    {
      if (is_number(l))
	  n = std::stoi(l);
      else
	for (size_t i=0; i<n; ++i)
	  vlayers.push_back(l);
    }

  std::vector<std::shared_ptr<Material>> layers;
  for (auto l : vlayers) {
    const auto m = std::find_if(matdb.begin(), matdb.end(),
				[&l](std::shared_ptr<Material> m)
				{return l == m->getName();});
    if (m != matdb.end())
      layers.push_back(*m);
    else {
      std::cerr << "Material " << l << " not found in the database. "
	" Print known materials with " << argv[0] << " -mat" << std::endl;
      return 1;
    }
  }

  // print layer configuration
  const size_t nLayers = vlayers.size();
  std::cout << nLayers << (nLayers == 1 ? " layer: " : " layers: ");
  for (auto l : vlayers)
    std::cout << l << " ";
  std::cout << std::endl;

  auto h2 = layers[0]->getSDEF();
  set_sdef(vsdef, h2, sdef);

  auto solver = std::make_shared<Solver>(sdef, layers);
  solver->run(-1,4);

  const auto fout = args->GetMap()["o"].as<std::string>();
  if (!fout.empty())
    solver->save(fout.data());

  const std::set<ParticleID> ftd {'n', 'p', 'e', '|'};
  std::cout << "\t\tDose rates:\t" << std::flush;
  for (auto p : ftd)
    std::cout << p << ": " << solver->getDose(p) << "\t";

  std::cerr << "total: " << solver->getDose() << std::endl;

  return 0;
}
