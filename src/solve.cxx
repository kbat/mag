#include <iostream>
#include <chrono>
#include <string>

#include <TROOT.h>
#include <TH2D.h>
#include <THnSparse.h>
#include <TKey.h>
#include <TFile.h>

#include "Test.h"
#include "Material.h"
#include "Source.h"
#include "Solver.h"
#include "SolverArguments.h"

bool is_number(const std::string& s)
/*!
  Check if a string is numeric.
 */
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

void print_materials(std::set<std::shared_ptr<Material> >& matdb)
/*!
  Print material database.
 */
{
    std::cout << "Supported materials: ";
    for (auto m : matdb)
      std::cout << m->getName() << " ";
    std::cout << std::endl;
}

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
  std::unique_ptr<SolverArguments> args = std::make_unique<SolverArguments>(argc, argv);

  if (args->IsHelp())
    return 0;

  // get sdef
  std::map<char, std::shared_ptr<TH2D>> sdef;
  const auto vsdef = args->GetMap()["sdef"].as<std::vector<std::string> >();

  if (args->IsTest()) {
    auto t = args->GetMap()["test"].as<std::vector<size_t> >();
    std::unique_ptr<Test> test = std::make_unique<Test>(t[0], t[1]);
    if (vsdef.size()==1) {
      sdef = fillSDEF(vsdef[0]);
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

    auto h2 = layers[0]->getSDEF();
    h2->Reset(); // just to be sure it's empty
    h2->Fill(E0, mu0);

    sdef.insert(std::make_pair(p0, h2));
  } else {
    std::cerr << "solve.cxx: wrong sdef" << std::endl;
    return 1;
  }

  auto solver = std::make_shared<Solver>(sdef, layers);
  solver->run(1);
  solver->save("res.root");

  static std::set<char> ftd {'n', 'p', 'e', '|'};
  std::cout << "\t\tDose rates:\t" << std::flush;
  for (auto p : ftd)
    std::cout << p << ": " << solver->getDose(p) << "\t";

  std::cerr << "total: " << solver->getDose() << std::endl;

  return 0;
}
