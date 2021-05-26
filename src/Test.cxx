#include <iostream>

#include "Test.h"
#include "Material.h"
#include "Solver.h"

Test::Test(size_t n, size_t nLayers) :
  n(n), nLayers(nLayers)
{
}

bool Test::run()
/*!
  Run some tests
 */
{
  bool val(true);

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
