#include <iostream>
#include <numeric>
#include<random>

#include "Optimiser.h"

std::default_random_engine eng{std::random_device{}()};
using Distribution = std::uniform_real_distribution<>;
Distribution dist;

Optimiser::Optimiser(const char p0,
		     std::shared_ptr<TH2D> sdef,
		     std::set<std::shared_ptr<Material>>& mat,
		     const size_t nLayers) :
  mat(mat), nLayers(nLayers)
{
  //  result = mat; // tmp
  return;
}

bool Optimiser::checkSum(const std::map<std::string, double>& prob)
{
  // Check if sum of probabilities add up to unity

  const double epsilon = 0.01;
  const double sum = std::accumulate(prob.begin(), prob.end(), 0.0,
  				     [](const double previous, const auto& element)
  				     { return previous+element.second; });

  return std::abs(sum-1.0<epsilon);
}


std::vector<std::shared_ptr<Material>>
Optimiser::run(const std::map<std::string, double>& prob)
{
  // Run optimisation given the probabilities of materials in the
  // material vector

  assert(mat.size() == prob.size() &&
	 "Wrong length of material probabilities vector: ");
  assert(checkSum(prob) && "Probabilities must sum up to 1.0");

  // // auxiliary map with cumulative distributions of probabilities
  // // in order to sample probabilities
  // std::map<std::string, double> cumulative;
  // double prev = 0.0;
  // for (auto m : prob) {
  //   prev += m.second;
  //   cumulative.insert(std::make_pair(m.first, prev));
  // }

  std::vector<std::shared_ptr<Material>> vec;
  // std::uniform_real_distribution<float> ugen(0.0f, 1.0f);

  // for (size_t i=0; i<nLayers; ++i) {
  //   auto p = dist(eng, Distribution::param_type{0.0, 1.0});
  //   for (auto m : cumulative) {
  //     if (p<m.second) {
  // 	vec.push_back(mat[m.first]);
  // 	break;
  //     }
  //   }
  // }
  // std::cout << vec.size() << std::endl;

  return vec;
}

double Optimiser::getMass() const
{
  // Return sum of densities of all materials in the vector

  return std::accumulate(result.begin(), result.end(), 0.0,
			 [](double prev, const auto m)
			 { return prev+m->getDensity(); });
}
