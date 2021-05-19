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
  auto W1 = std::make_shared<Material>("Tungsten1", "Tungsten.root", 38, 19.413);

  const size_t nLayers = 10;//150;
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

  auto opt = std::make_shared<Optimiser>(p0, sdef, mat, nLayers);

  std::map<std::shared_ptr<Material>, double > prob;
  prob.insert(std::make_pair(poly, 0.11));
  prob.insert(std::make_pair(b4c, 0.2));
  prob.insert(std::make_pair(steel, 0.09));
  prob.insert(std::make_pair(W, 0.6));
  //prob.insert(std::make_pair(W1, 0.6));

  auto result = opt->run(1); //, prob);
  // std::cout << "Mass: " << opt->getMass() << std::endl;
  for (auto l : result)
    std::cout << l->getID() << " ";
  std::cout << opt->getDose() << std::endl;

}
