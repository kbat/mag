#include <iostream>
#include <numeric>
#include <random>

#include "Solver.h"
#include "Optimiser.h"

std::default_random_engine eng{std::random_device{}()};
using Distribution = std::uniform_real_distribution<>;
Distribution dist;

Optimiser::Optimiser(const char p0,
		     std::shared_ptr<TH2D> sdef,
		     std::set<std::shared_ptr<Material>>& mat,
		     const size_t nLayers) :
  p0(p0), sdef(sdef), mat(mat), nLayers(nLayers)
{
  RO = 1;
  genSize = 50;

  return;
}

bool Optimiser::checkSum(const std::map<std::shared_ptr<Material>, double>& prob)
{
  // Check if sum of probabilities add up to unity

  const double epsilon = 0.01;
  const double sum = std::accumulate(prob.begin(), prob.end(), 0.0,
  				     [](const double previous, const auto& element)
  				     { return previous+element.second; });

  return (std::abs(sum-1.0)<epsilon);
}

bool Optimiser::checkMat(const std::map<std::shared_ptr<Material>, double>& prob)
{
  // Check if materals in the 'prob' map match to the 'mat' set

  for (auto p : prob) {
    auto m = std::find_if(mat.begin(), mat.end(),
  			  [&p](std::shared_ptr<Material> mm)
  			  {return mm == p.first;});
    if (m == mat.end()) {
      std::cerr << "Material " << p.first->getName() << " not found in the 'mat' set" << std::endl;
      return false;
    }
  }

  return true;
}

std::vector<std::shared_ptr<Material>>
Optimiser::getLayers(const std::shared_ptr<Material>& m)
{
  // Return vector of layers with the same material m

  std::vector<std::shared_ptr<Material>> layers;
  layers.resize(nLayers,m);

  return layers;
}

std::vector<std::shared_ptr<Material>>
Optimiser::getLayers(const std::map<std::shared_ptr<Material>, double>& prob)
{
  // Return vector of layers given the material probabilities

  assert(mat.size() == prob.size() &&
	 "Wrong length of material probabilities vector");
  assert(checkSum(prob) && "Probabilities must sum up to 1.0");
  assert(checkMat(prob) && "Materials specified in 'prob' do not match the 'mat' set");

  // auxiliary map with cumulative distributions of probabilities
  // in order to sample probabilities
  std::map<std::shared_ptr<Material>, double> cumulative;
  double prev = 0.0;
  for (auto m : prob) {
    prev += m.second;
    cumulative.insert(std::make_pair(m.first, prev));
  }

  std::vector<std::shared_ptr<Material>> layers;

  for (size_t i=0; i<nLayers; ++i) {
    auto p = dist(eng, Distribution::param_type{0.0, 1.0});
    for (auto mp : cumulative) { // mp: material, probability
      if (p<mp.second) {
  	auto m = std::find_if(mat.begin(), mat.end(),
  			      [&mp](std::shared_ptr<Material> mm)
  			      {return mm == mp.first;});
  	// if (m == mat.end()) {
  	//   std::cerr << mp.first->getName() << " not found in the 'mat' set" << std::endl;
  	//   exit(1);
  	// }

  	layers.push_back(*m);
  	break;
      }
    }
  }

  assert(layers.size() == nLayers);

  return layers;
}

std::vector<std::shared_ptr<Material>> Optimiser::getLayers()
{
  // Return vector of layers with uniform material probabilities

  // non-normalised random uniform vector
  std::vector<double> vec(mat.size());
  std::generate(vec.begin(), vec.end(), [&](){return dist(eng, Distribution::param_type{0.0, 1.0});});
  const double sum = std::accumulate(vec.begin(), vec.end(), 0.0);

  // random vector normalised to unity
  std::vector<double> nvec;
  std::transform(vec.begin(), vec.end(), std::back_inserter(nvec), [&sum](double x){return x/sum;});

  // map of uniform probabilities
  std::map<std::shared_ptr<Material>, double > prob;
  std::transform(mat.begin(), mat.end(), nvec.begin(),
		 std::inserter(prob, prob.end()),
		 std::make_pair<std::shared_ptr<Material> const&, double const&>);

  return getLayers(prob);
}

// std::vector<std::shared_ptr<Material>>
// Optimiser::run(const std::map<std::shared_ptr<Material>, double>& prob)
// {
//   // Run calculation with the given probabilities for different materials in the layers

//   std::vector<std::shared_ptr<Material>> layers = getLayers(prob);

//   auto solver = std::make_shared<Solver>(p0, sdef, layers);
//   solver->run(RO);

//   getObjectiveFunction(solver);

//   return layers;
// }

void Optimiser::run()
{
  // Run calculation with uniform probabilities for different materials in the layers

  //  getLayers(*(++mat.begin()));

  std::vector<std::shared_ptr<Material>> layers;
  std::vector<std::shared_ptr<Solver>>   solutions;

  // First, we create (but not run yet) solutions with homogenic materials:
  for (auto m : mat) {
    layers = getLayers(m);
    solutions.emplace_back(std::make_shared<Solver>(p0, sdef, layers));
  }

  // Now we prepare the rest of the first generation
  for (size_t i=0; i<genSize; ++i) {
    layers = getLayers();
    solutions.emplace_back(std::make_shared<Solver>(p0, sdef, layers));
  }

  std::for_each(solutions.begin(), solutions.end(), [&](auto &s){ s->run(RO); });

  std::cout << solutions.size() << std::endl;

}

double Optimiser::getObjectiveFunction(const std::shared_ptr<Solver>& s) const
{
  // Return objective function

  const double dose = s->getDose();
  const double mass = s->getMass();
  const size_t comp = s->getComplexity();

  //  std::cout << dose << " " << mass << " " << comp << std::endl;

  return dose;
}
