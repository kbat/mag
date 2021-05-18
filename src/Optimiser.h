#ifndef Optimiser_h
#define Optimiser_h

#include <TH2D.h>
#include "Material.h"

class Optimiser {
 private:
  const size_t nLayers; //< number of layers
  std::vector<std::shared_ptr<Material>> result; //< optimised vector of materials
 public:
  Optimiser(const char, std::shared_ptr<TH2D>,
	    std::vector<std::shared_ptr<Material>> &,
	    const size_t);
  void run();
  double getMass() const;
};


#endif
