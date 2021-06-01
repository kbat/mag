#include <iostream>

#include <TROOT.h>
#include <TH2D.h>

#include "Material.h"
#include "Source.h"
#include "Solver.h"
#include "Optimiser.h"


int main(int argc, const char **argv)
{
  std::set<std::shared_ptr<Material> > matdb;
  matdb.insert(std::make_shared<Material>("Stainless304", "Stainless304.root", 3, 7.96703));
  matdb.insert(std::make_shared<Material>("Water", "Water.root", 11, 1.0));
  matdb.insert(std::make_shared<Material>("Lead", "Lead.root", 23, 11.1837));
  matdb.insert(std::make_shared<Material>("W", "Tungsten.root", 38, 19.413));
  matdb.insert(std::make_shared<Material>("B4C", "B4C.root", 47, 2.50608));
  matdb.insert(std::make_shared<Material>("Poly", "Poly.root", 48, 0.91));
  matdb.insert(std::make_shared<Material>("Concrete", "Concrete.root", 49, 2.33578));

  std::map<char, std::shared_ptr<TH2D>> sdef;
  const size_t nLayers = 20;
  const char p0 = 'e';
  const double E0 = 3e3;
  const double mu0 = 0.999; // 0 < mu0 < 1 (not <=)

  auto h2 = (*matdb.begin())->getSDEF();
  h2->Reset();
  h2->Fill(E0, mu0);
  sdef.insert(std::make_pair(p0, h2));

  auto opt = std::make_unique<Optimiser>(sdef, matdb, nLayers);
  opt->setReflectionOrder(0);
  //  opt->setMinRandomPopulation(0);
  opt->setNPrint(5); // number of best solutions printed after each generation

  opt->setDoseWeight(1.0);
  opt->setMassWeight(0.0);
  opt->setComplexityWeight(0.0);

  // opt->setInheritedFraction(0.07); // 0.05
  // opt->setPMutation(0.1); // 0.3

  opt->run(3); //, prob);
  // std::cout << "Mass: " << opt->getMass() << std::endl;
  //  std::cout << opt->getDose() << "\t";
  // for (auto l : result)
  //   std::cout << l->getID() << " ";
  // std::cout << std::endl;

}
