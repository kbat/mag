#include <iostream>
#include <numeric>

#include "Optimiser.h"

Optimiser::Optimiser(const char p0,
		     std::shared_ptr<TH2D> sdef,
		     std::map<std::string, std::shared_ptr<Material> >& mat,
		     const size_t nLayers) :
  mat(mat), nLayers(nLayers)
{
  //  result = mat; // tmp
}

void Optimiser::run(std::map<std::string, double>& prob)
{
  // Run optimisation given the probabilities of materials in the material vector
  assert(mat.size() == prob.size()+1 && "Wrong length of material probabilities vector: "); //  <<   prob.size() << "+1 != " << mat.size() << std::endl;

  assert(std::accumulate(prob.begin(), prob.end(), 0.0,
			 [](const double previous, const auto& element)
			 { return previous + element.second; })<1.0 && "Sum of probabilities must be below 1.0");

}

double Optimiser::getMass() const
{
  // Return sum of densities of all materials in the vector

  return std::accumulate(result.begin(), result.end(), 0.0,
			 [](double prev, const auto m)
			 { return prev+m->getDensity(); });
}
