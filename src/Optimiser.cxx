#include <iostream>

#include "Optimiser.h"

Optimiser::Optimiser(const char p0,
		     std::shared_ptr<TH2D> sdef,
		     std::vector<std::shared_ptr<Material>>& mat,
		     const size_t nLayers) :
  nLayers(nLayers)
{
}
