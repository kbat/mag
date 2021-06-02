#include <THnSparse.h>
#include <TKey.h>
#include <TFile.h>

#include "SDEF.h"

SDEF::SDEF(const std::string& fname) :
  fname(fname)
  /*!
    Constructor.
    Read sdef tallies from the given file name and convert them into TH2D histograms
  */

{
  const double epsilon = 1e-3;
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
      try {
	const char p0 = tallyDict.at(std::stoi(tnum));
	//	h->SetName(std::string(1,p0).c_str());
	if (f1->GetAxis(0)->GetNbins() == 2) {
	  f1->GetAxis(0)->SetRange(2,2); // assume that 2nd bin is forward-going
	}
	f1->GetAxis(5)->SetRangeUser(epsilon, 1.0-epsilon); // forward bins only
	auto h = f1->Projection(5,6);
	h->SetDirectory(0);
	h->SetName(&p0);
	//	std::cout << h->GetName() << std::endl;
	sdef.insert(std::make_pair(p0, std::make_shared<TH2D>(*h)));
	delete h; h = nullptr;
      } catch (...) {
      	std::cout << "SDEF::SDEF: skipping tally " << hname << std::endl;
      }
    }
    delete obj; obj = nullptr;
  }

  file.Close();
}
