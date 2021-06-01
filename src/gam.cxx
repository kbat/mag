#include <iostream>

#include <TROOT.h>
#include <TH2D.h>
#include <THnSparse.h>
#include <TKey.h>
#include <TFile.h>

#include "Material.h"
#include "Source.h"
#include "Solver.h"
#include "Optimiser.h"
#include "OptArguments.h"

std::map<char, std::shared_ptr<TH2D>> fillSDEF(const std::string& fname)
/*!
  Read sdef tallies from the given file name and convert them into TH2D histograms
 */
{
  const double epsilon = 1e-3;
  std::map<char, std::shared_ptr<TH2D>> sdef;
  const std::map<size_t,char> tallyDict = {{1,'n'}, {11,'p'}, {21,'e'}, {31,'|'} };

  TFile file(fname.data());
  TIter next(file.GetListOfKeys());

  while (TKey *key = (TKey*)next()) {
    TObject *obj = file.Get(key->GetName()); // copy object to memory
    if (obj->InheritsFrom("THnSparseF")) {
      const THnSparseF *f1 = dynamic_cast<THnSparseF*>(obj);
      const std::string hname = f1->GetName();
      std::string tnum = hname;
      tnum.erase(0,1); // remove ^f
      if (f1->GetAxis(0)->GetNbins() == 2) {
	f1->GetAxis(0)->SetRange(2,2); // assume that 2nd bin is forward-going
      }
      f1->GetAxis(5)->SetRangeUser(epsilon, 1.0-epsilon); // forward bins only
      auto h = f1->Projection(5,6);
      h->SetDirectory(0);
      try {
	const char p0 = tallyDict.at(std::stoi(tnum));
	//	h->SetName(std::string(1,p0).c_str());
	h->SetName(&p0);
	//	std::cout << h->GetName() << std::endl;
	sdef.insert(std::make_pair(p0, std::make_shared<TH2D>(*h)));
      } catch (...) {
      	std::cout << "solve.cxx: error with " << hname << std::endl;
      }
      delete h; h = nullptr;
    }
    delete obj; obj = nullptr;
  }

  file.Close();

  return sdef;
}


int main(int argc, const char **argv)
{
  std::unique_ptr<OptArguments> args = std::make_unique<OptArguments>(argc, argv);

  if (args->IsHelp())
    return 0;

  std::set<std::shared_ptr<Material> > matdb;
  matdb.insert(std::make_shared<Material>("Stainless304", "Stainless304.root", 3, 7.96703));
  matdb.insert(std::make_shared<Material>("Water", "Water.root", 11, 1.0));
  matdb.insert(std::make_shared<Material>("Lead", "Lead.root", 23, 11.1837));
  matdb.insert(std::make_shared<Material>("W", "Tungsten.root", 38, 19.413));
  matdb.insert(std::make_shared<Material>("B4C", "B4C.root", 47, 2.50608));
  matdb.insert(std::make_shared<Material>("Poly", "Poly.root", 48, 0.91));
  matdb.insert(std::make_shared<Material>("Concrete", "Concrete.root", 49, 2.33578));

  std::map<char, std::shared_ptr<TH2D>> sdef;
  const auto vsdef = args->GetMap()["sdef"].as<std::vector<std::string> >();

  if (vsdef.size()==1) {
    sdef = fillSDEF(vsdef[0]);
    std::cout << "Fluxes of " << sdef.size() << " sdef particles:\t" << std::flush;
    std::for_each(sdef.begin(), sdef.end(),
		  [](const auto& s) {
		    std::cout << s.first << ": " << s.second->Integral() << "\t";
		  });
    std::cout << std::endl;
  } else if (vsdef.size()==3) {
    const char p0 = vsdef[0][0];
    const double E0 = std::stod(vsdef[1]);
    const double mu0 = std::stod(vsdef[2]);

    auto h2 = (*matdb.begin())->getSDEF();
    h2->Reset(); // just to be sure it's empty
    h2->Fill(E0, mu0);

    sdef.insert(std::make_pair(p0, h2));
  } else {
    std::cerr << "gam.cxx: wrong sdef" << std::endl;
    return 1;
  }

  auto opt = std::make_unique<Optimiser>(sdef, matdb, args->getNLayers());
  opt->setReflectionOrder(0);
  //  opt->setMinRandomPopulation(0);
  opt->setNPrint(5); // number of best solutions printed after each generation

  opt->setDoseWeight(1.0);
  opt->setMassWeight(0.0);
  opt->setComplexityWeight(0.0);

  // opt->setInheritedFraction(0.07); // 0.05
  // opt->setPMutation(0.1); // 0.3

  opt->run(args->getNGen()); //, prob);
  // std::cout << "Mass: " << opt->getMass() << std::endl;
  //  std::cout << opt->getDose() << "\t";
  // for (auto l : result)
  //   std::cout << l->getID() << " ";
  // std::cout << std::endl;

}
