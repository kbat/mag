/*
  Common functions
 */

#include <set>
#include <memory>
#include <iostream>

#include "SDEF.h"
#include "Material.h"
#include "functions.h"

bool is_number(const std::string& s)
/*!
  Check if a string is numeric.
 */
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

void print_materials(const std::set<std::shared_ptr<Material> >& matdb)
/*!
  Print material database.
 */
{
  std::cout << "Supported materials:" << std::endl;
    std::for_each(matdb.begin(), matdb.end(),
	     [](const auto &m) {
	       // TODO: use C++-20 and std::format
	       std::cout << m->getID() << " " << m->getName() << " " << m->getDensity() << std::endl;;
	     });
}

void set_sdef(const std::vector<std::string>& vsdef,
	      std::shared_ptr<TH2D>& h2,
	      std::map<char, std::shared_ptr<TH2D>>& sdef)
/*!
  SDEF setter
 */
{
  if (vsdef.size()==1) {
    auto s = std::make_unique<SDEF>(vsdef[0]);
    sdef = s->getSDEF();
    std::cout << "Fluxes of " << sdef.size() << " sdef particles:\t" << std::flush;
    std::for_each(sdef.begin(), sdef.end(),
		  [](const auto& s) {
		    std::cout << s.first << ": " << s.second->Integral() << "\t";
		  });
    std::cout << std::endl;
  } else if (vsdef.size()==3) {

    const char p0 = vsdef[0][0];
    const double E0 = std::stod(vsdef[1]);

    if (E0<=0.0) {
      std::cerr << "ERROR: E0 is negative: " << E0 << std::endl;
      exit(1);
    }

    double mu0 = std::stod(vsdef[2]);

    if (mu0>=1.0) {
      if (mu0<1.0+std::numeric_limits<double>::epsilon()) {
	// substracting epsilon so that mu0 falls in the correct histogram bin
	mu0 -= std::numeric_limits<double>::epsilon();
      }  else {
	std::cerr << "ERROR: mu0 must be below 1.0: " << mu0 << std::endl;
	exit(1);
      }
    }

    h2->Reset(); // just to be sure it's empty
    h2->Fill(E0, mu0);

    sdef.insert(std::make_pair(p0, h2));
  } else {
    std::cerr << "ERROR: set_sdef: wrong sdef" << std::endl;
    exit(1) ;
  }

  return;
}

bool check_layers(std::vector<std::shared_ptr<Material>>& layers)
{
  // Check if matrices of all layers have the same shape. This
  // function assumes that all matrices of the same material have the
  // same shape (so that it's enough to compare with the other layers
  // only one of them) - TODO

  const auto l0 = *layers.begin();
  const auto p = *l0->getParticles().begin(); // just any existing particle
  const auto m0 = l0->getT(p,p);

  bool val = true;

  for_each(layers.begin()+1, layers.end(),
	   [&](const auto& l)  {
	     const auto ml = l->getT(p,p);
	     if (m0->GetNrows()!=ml->GetNrows()) {
	       std::cerr << "check_layers: different number of rows between "
			 << l0->getName() << " and " << l->getName() << std::endl;
	       val = false;
	     } else if (m0->GetNcols()!=ml->GetNcols()) {
	       std::cerr << "check_layers: different number of columns between "
			 << l0->getName() << " and " << l->getName() << std::endl;
	       val = false;
	     }
	   });

  return val;
}
