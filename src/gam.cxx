#include <iostream>

#include <TROOT.h>
#include <TH2D.h>

#include "Material.h"
#include "Source.h"
#include "Solver.h"
#include "Optimiser.h"


int main(int argc, const char **argv)
{
  auto poly = std::make_shared<Material>("Polyethylene", "Poly.root", 48, 0.91);
  auto concrete = std::make_shared<Material>("Concrete", "Concrete.root", 49, 2.33578);
  auto b4c = std::make_shared<Material>("B4C", "B4C.root", 47, 2.50608);
  auto steel = std::make_shared<Material>("Stainless304", "Stainless304.root", 3, 7.96703);
  auto W = std::make_shared<Material>("Tungsten", "Tungsten.root", 38, 19.413);

  const size_t nLayers = 150;
  const char p0 = 'e';
  const double E0 = 3e3;
  const double mu0 = 1.0;

  std::vector<std::shared_ptr<Material>> mat;
  mat.push_back(poly);
  //  mat.push_back(concrete);
  mat.push_back(b4c);
  mat.push_back(steel);
  mat.push_back(W);

  auto sdef = mat[0]->getSDEF();
  sdef->Fill(E0, mu0);

  auto opt = std::make_shared<Optimiser>(p0, sdef, mat, nLayers);
}
