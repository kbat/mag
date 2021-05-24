#include <iostream>

#include <TROOT.h>
#include <TH2D.h>

#include "Material.h"
#include "Source.h"
#include "Solver.h"
#include "Optimiser.h"


int main(int argc, const char **argv)
{
  auto poly = std::make_shared<Material>("Poly", "Poly.root", 48, 0.91);
  auto concrete = std::make_shared<Material>("Concrete", "Concrete.root", 49, 2.33578);
  auto b4c = std::make_shared<Material>("B4C", "B4C.root", 47, 2.50608);
  auto steel = std::make_shared<Material>("Stainless304", "Stainless304.root", 3, 7.96703);
  auto W = std::make_shared<Material>("Tungsten", "Tungsten.root", 38, 19.413);

  const size_t nLayers = 15;
  const char p0 = 'e';
  const double E0 = 3e3;
  const double mu0 = 0.999; // must be < 1

  std::set<std::shared_ptr<Material> > mat;
  mat.insert(poly);
  //  mat.insert(concrete);
  mat.insert(b4c);
  mat.insert(steel);
  mat.insert(W);

  auto sdef = (*mat.begin())->getSDEF();
  sdef->Fill(E0, mu0);

  auto opt = std::make_unique<Optimiser>(p0, sdef, mat, nLayers);
  opt->setReflectionOrder(0);
  opt->setGenSize(48); // 8*6 = 48

  opt->setDoseWeight(1.0);
  opt->setMassWeight(0.0);
  opt->setComplexityWeight(0.0);

  opt->setInheritedFraction(0.07); // 0.05
  opt->setPMutation(0.1); // 0.3, but Wiki says it must be low. If it is set too high, the search will turn into a primitive random search.

  // std::map<std::shared_ptr<Material>, double > prob;
  // prob.insert(std::make_pair(poly, 0.11));
  // prob.insert(std::make_pair(b4c, 0.2));
  // prob.insert(std::make_pair(steel, 0.09));
  // prob.insert(std::make_pair(W, 0.6));
  // //prob.insert(std::make_pair(W1, 0.6));

  opt->run(300); //, prob);
  // std::cout << "Mass: " << opt->getMass() << std::endl;
  //  std::cout << opt->getDose() << "\t";
  // for (auto l : result)
  //   std::cout << l->getID() << " ";
  // std::cout << std::endl;

}
