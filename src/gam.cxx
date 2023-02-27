#include <iostream>

#include "SDEF.h"
#include "Material.h"
#include "Source.h"
#include "Solver.h"
#include "Optimiser.h"
#include "OptArguments.h"
#include "functions.h"

int main(int argc, const char **argv)
{
  std::unique_ptr<OptArguments> args = std::make_unique<OptArguments>(argc, argv);

  if (args->IsHelp())
    return 0;

  std::set<std::shared_ptr<Material> > matdb;
  matdb.insert(std::make_shared<Material>("Stainless304", "Stainless304.root", 3, 7.96703));
  //  matdb.insert(std::make_shared<Material>("Water", "Water.root", 11, 1.0));
  matdb.insert(std::make_shared<Material>("Lead", "Lead.root", 23, 11.1837));
  matdb.insert(std::make_shared<Material>("W", "Tungsten.root", 38, 19.413));
  matdb.insert(std::make_shared<Material>("B4C", "B4C.root", 47, 2.50608));
  matdb.insert(std::make_shared<Material>("Poly", "Poly.root", 48, 0.91));
  //  matdb.insert(std::make_shared<Material>("Concrete", "Concrete.root", 49, 2.33578));

  if (args->IsMaterials()) {
    print_materials(matdb);
    return 0;
  }


  ////////////////////////////////////////////////////////
  // TODO: do not duplicate code with solve.cxx
  const auto layout = args->getTail();
  std::vector<std::shared_ptr<Material>> tail;
  if (!layout[0].empty()) // tail is defined
    {
      size_t n(1);
      std::vector<std::string> vtail;
      for (auto l : layout)
	{
	  if (is_number(l))
	    n = std::stoi(l);
	  else
	    for (size_t i=0; i<n; ++i)
	      vtail.push_back(l);
	}

      for (auto l : vtail) {
	const auto m = std::find_if(matdb.begin(), matdb.end(),
				    [&l](std::shared_ptr<Material> m)
				    {return l == m->getName();});
	if (m != matdb.end())
	  tail.push_back(*m);
	else {
	  std::cerr << "Material " << l << " not found in the database. "
	    " Print known materials with " << argv[0] << " -mat" << std::endl;
	  return 1;
	}
      }
    }
  ///////////////////////////////////////////////////////


  std::map<char, std::shared_ptr<TH2D>> sdef;
  const auto vsdef = args->GetMap()["sdef"].as<std::vector<std::string> >();

  auto h2 = (*matdb.begin())->getSDEF();
  set_sdef(vsdef, h2, sdef);

  auto opt = std::make_unique<Optimiser>(sdef, matdb, args->getNLayers());
  opt->setTail(tail);
  opt->setReflectionOrder(0);
  //  opt->setMinRandomPopulation(0);
  opt->setNPrint(args->getNPrint()); // number of best solutions printed after each generation

  opt->setDoseWeight(1.0);
  opt->setMassWeight(0.5);
  opt->setComplexityWeight(0.1);

  opt->setNInherited(2); // 0.05
  // opt->setPMutation(0.1); // 0.3

  opt->run(args->getNGen()); //, prob);
  // std::cout << "Mass: " << opt->getMass() << std::endl;
  //  std::cout << opt->getDose() << "\t";
  // for (auto l : result)
  //   std::cout << l->getID() << " ";
  // std::cout << std::endl;

}
