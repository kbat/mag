#include <iostream>

#include "Test.h"
#include "Material.h"
#include "Solver.h"

#include <TH1D.h>

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

  const bool isMarkov = n<10 ? false : true;

  // Transmission matrix tests:
  auto mTest1 = std::make_shared<Material>("Test", "test/solver/test1.root", 1, 1);
  auto mTest2 = std::make_shared<Material>("Test", "test/solver/test2.root", 2, 1);
  auto mTest3 = std::make_shared<Material>("Test", "test/solver/test3.root", 3, 1);
  auto mTest5 = std::make_shared<Material>("Test", "test/solver/test5.root", 4, 1);
  auto mTest6 = std::make_shared<Material>("Test", "test/solver/test6.root", 5, 1);
  auto mTest7 = std::make_shared<Material>("Test", "test/solver/test7.root", 5, 1);

  // Markov chain tests
  auto mTest10 = std::make_shared<Material>("Test10", "test/solver/test10.root", 10, 1);
  auto mTest11 = std::make_shared<Material>("Test11", "test/solver/test1.root",  11, 1);

  size_t ro=0; // reflection order

  // TODO: simplify these IFs (put mTest in array)
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
  } else if (n==7) {
    ro = 1;
    for (size_t i=0; i<nLayers; ++i)
      mat.push_back(mTest7);
  } else if (n==10) {
    for (size_t i=0; i<nLayers; ++i)
      mat.push_back(mTest10);
  } else if (n==11) {
    for (size_t i=0; i<nLayers; ++i)
      mat.push_back(mTest11);
  } else {
    std::cerr << "Test::run error: test " << n << " is not defined" << std::endl;
    return false;
  }

  std::map<char, std::shared_ptr<TH2D>> sdef;
  if (getSDEF().size())
    sdef = getSDEF();
  else
    sdef.insert(std::make_pair('n', mat[0]->getSDEF()));

  auto solver = std::make_shared<Solver>(sdef, mat);
  if (isMarkov)
    solver->runMarkov(2);
  else
    solver->run(-1, ro);
  solver->save("res.root");

  return val;
}
