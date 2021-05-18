#ifndef Optimiser_h
#define Optimiser_h

#include <TH2D.h>
#include "Material.h"

class Optimiser {
 private:
  std::map<std::string, std::shared_ptr<Material> > mat; //< library of materials
  const size_t nLayers; //< number of layers
  std::vector<std::shared_ptr<Material>> result; //< optimised vector of materials
 public:
  Optimiser(const char, std::shared_ptr<TH2D>,
	    std::map<std::string, std::shared_ptr<Material> >&,
	    const size_t);
  std::vector<std::shared_ptr<Material>> run(const std::map<std::string, double>&);
  double getMass() const;
};


#endif
