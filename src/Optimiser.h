#ifndef Optimiser_h
#define Optimiser_h

#include <TH2D.h>
#include "Material.h"

class Optimiser {
 private:
  const size_t nLayers; //< number of layers
 public:
  Optimiser(const char, std::shared_ptr<TH2D>,
	    std::vector<std::shared_ptr<Material>> &,
	    const size_t);
};


#endif
