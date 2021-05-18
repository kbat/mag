#include <iostream>
#include <numeric>

#include "Optimiser.h"

Optimiser::Optimiser(const char p0,
		     std::shared_ptr<TH2D> sdef,
		     std::vector<std::shared_ptr<Material>>& mat,
		     const size_t nLayers) :
  nLayers(nLayers)
{
  result = mat; // tmp
}

void Optimiser::run()
{
  // Run optimisation
}

double Optimiser::getMass() const
{
  // Return sum of densities of all materials in the vector

  double sum =
    std::accumulate(result.begin(), result.end(), 0.0,
		    [](double s, const std::shared_ptr<Material> m){
		      return m->getDensity()+s; });

  return sum;
}
