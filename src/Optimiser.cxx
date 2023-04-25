#include <iostream>
#include <numeric>
#include <random>
#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>
#include <chrono>

#include "Solver.h"
#include "Optimiser.h"

std::default_random_engine eng{std::random_device{}()};
using Distribution = std::uniform_real_distribution<>;
Distribution dist;

Optimiser::Optimiser(const std::map<ParticleID, std::shared_ptr<TH2D>>& sdef,
		     const std::set<std::shared_ptr<Material>>& mat,
		     const size_t nLayers) :
  sdef(sdef), mat(mat), nLayers(nLayers)
{
  RO = 1;
  minRandomPopulation = 0;

  // these are default values
  // set the weights in gam.cxx
  doseWeight = 1.0;
  massWeight = 0.5;
  compWeight = 0.1;

  nInherited = 2;
  pMutation = 0.3;
  nprint = -1;

  return;
}

void Optimiser::setMinRandomPopulation(size_t n)
/*!
  Set minimal random population size. Its default value is 0 and
  it's OK to leave it to be zero since anyway a part of population will
  become random (with probabilty pMutation) due to mutation in each generation.
  Use this method in order to increase the size of random population already in the first
  generation but be aware that too large fraction of random population will lead to inefficient
  random search.
 */
{
  minRandomPopulation = n;
}

bool Optimiser::checkSum(const std::map<std::shared_ptr<Material>, double>& prob) const
{
  // Check if sum of probabilities add up to unity

  const double epsilon = 0.01;
  const double sum = std::accumulate(prob.begin(), prob.end(), 0.0,
  				     [](const double previous, const auto& element)
  				     { return previous+element.second; });

  return (std::abs(sum-1.0)<epsilon);
}

bool Optimiser::checkMat(const std::map<std::shared_ptr<Material>, double>& prob) const
{
  // Check if materals in the 'prob' map match the 'mat' set

  for (auto p : prob) {
    auto m = std::find_if(mat.begin(), mat.end(),
  			  [&p](const auto &mm)
  			  {return mm == p.first;});
    if (m == mat.end()) {
      std::cerr << "Material " << p.first->getName() << " not found in the 'mat' set" << std::endl;
      return false;
    }
  }

  return true;
}

std::vector<std::shared_ptr<Material>>
Optimiser::getLayers(const std::shared_ptr<Material>& m) const
{
  // Return vector of layers with the same material m

  std::vector<std::shared_ptr<Material>> layers;
  layers.resize(nLayers,m);

  return layers;
}

std::vector<std::shared_ptr<Material>>
Optimiser::getLayers(const std::shared_ptr<Material>& m1,
		     const std::shared_ptr<Material>& m2,
		     size_t j) const
{
  // Return vector of layers constructed by shuffling combinations the
  // two materials m1 and m2.  Only layers of complexity 2 are taken
  // into account.
  // j : number of the boundary layer between two materials

  std::vector<std::shared_ptr<Material>> layers;

  if (m1!=m2)
    for (size_t i=0; i<nLayers; ++i)
      layers.push_back(i<=j ? m1 : m2);

  return layers;
}


std::vector<std::shared_ptr<Material>>
Optimiser::getLayers(const std::map<std::shared_ptr<Material>, double>& prob) const
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
  for (const auto m : prob) {
    prev += m.second;
    cumulative.insert(std::make_pair(m.first, prev));
  }

  std::vector<std::shared_ptr<Material>> layers;

  for (size_t i=0; i<nLayers; ++i) {
    const auto p = dist(eng, Distribution::param_type{0.0, 1.0});
    for (const auto mp : cumulative) { // mp: material, probability
      if (p<mp.second) {
  	const auto m = std::find_if(mat.begin(), mat.end(),
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

std::vector<std::shared_ptr<Material>> Optimiser::getLayers() const
{
  // Return vector of layers with uniform random material probabilities

  // non-normalised random uniform vector
  std::vector<double> vec(mat.size());
  std::generate(vec.begin(), vec.end(),
		[&](){return dist(eng, Distribution::param_type{0.0, 1.0});});
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

void Optimiser::run(size_t ngen)
/*!
  Run calculation with uniform probabilities for different materials in the layers
  ngen : number of generations to run
*/
{
  std::vector<std::shared_ptr<Solver>> solutions;

  //  Solutions with homogenic materials:
  std::for_each(mat.begin(), mat.end(),
  		[&](const auto &m){
  		  std::vector<std::shared_ptr<Material>> layers = getLayers(m);
		  layers.insert(layers.end(), tail.begin(), tail.end()); // add tail
  		  solutions.emplace_back(std::make_shared<Solver>(sdef, layers, -1));
  		});

  // We shuffle combinations of two materials of complexity 2 in different proportions
  for (auto m1 : mat)
    for (auto m2 : mat)
      for (size_t j=0; j<nLayers-1; ++j) {
	std::vector<std::shared_ptr<Material>> layers = getLayers(m1, m2, j);
	if (layers.size()) {
	  layers.insert(layers.end(), tail.begin(), tail.end()); // add tail
	  solutions.emplace_back(std::make_shared<Solver>(sdef, layers, -1));
	}
      }

  const size_t ncores = tbb::task_scheduler_init::default_num_threads();
  //  std::cout << "Number of cores: " << ncores << std::endl;

  const size_t Nconst = solutions.size();
  std::cout << "Pre-defined generation size: " << Nconst << std::endl;
  // std::cout << "Min random population: " << minRandomPopulation << std::endl;
  const size_t Ntot = Nconst + minRandomPopulation;
  // std::cout << "Total pre-defined generation size: " << N << std::endl;
  const size_t n = (ncores-Ntot%ncores)%ncores;
  std::cout << "Additional random population in order to fill all " << ncores << " cores: " << n << std::endl;
  // std::cout << (Ntot+n)%ncores << std::endl;

  std::cout << nInherited << " best layouts are inherited into the next generation,\n  the rest " << Ntot-nInherited <<
    " are either mutated (with probability of " << pMutation <<")\n  or otherwise crossbreed in each generation." << std::endl;


  // Now we fill the rest of the first generation with solutions made from random materials
  const size_t nRandom = minRandomPopulation + n;
  for (size_t i=0; i<nRandom; ++i) {
    std::vector<std::shared_ptr<Material>> layers = getLayers();
    layers.insert(layers.end(), tail.begin(), tail.end()); // add tail
    solutions.emplace_back(std::make_shared<Solver>(sdef, layers, -1));
  }

  nprint == -1 ? solutions.size() : nprint;

  for (size_t gen=1; gen<=ngen; ++gen) {
    std::cout << "\nGeneration: " << gen << std::endl;

    auto start = std::chrono::system_clock::now();

    tbb::task_scheduler_init init(ncores);
    tbb::parallel_for(tbb::blocked_range<size_t>(0,solutions.size()),
		      [&](const tbb::blocked_range<size_t>& r) {
			for (size_t i=r.begin(); i<r.end(); ++i)
			  solutions[i]->run(RO);
		      });
    auto stop  = std::chrono::system_clock::now();
    std::chrono::duration<double> dur = stop-start;
    std::cout << dur.count() << " sec" << std::endl;

    // sort solutions
    std::sort(solutions.begin(), solutions.end(),
	      [&](const auto &l, const auto &r){
		return (getFitness(l)<getFitness(r));
	      });

    // remove duplicates
    purge(solutions);


    std::cout << "Fitness  Dose     Mass   Complexity Materials" << std::endl;

    // print
    const size_t n = std::min<size_t>(nprint, solutions.size());
    std::for_each(solutions.begin(), std::next(solutions.begin(), n),
    		  [&](const auto &s){
    		    std::cout << getFitness(s) << " " << s->getDose() << " "
    			      << s->getMass() << " " << s->getComplexity() << "         ";
    		    const auto mat = s->getLayers();
    		    std::for_each(mat.begin(), mat.end(),
    				  [](const auto &m){std::cout << m->getID() << " ";});
    		    std::cout << std::endl;
    		  });

    // leave in the solutions vector only directly inherited solutions
    // the rest will be mutated or crossbreed
    if (gen!=ngen) {
      // failed = not directly inherited -> crossover or mutation
      std::vector<std::shared_ptr<Solver>> failed;
      const auto it = std::next(solutions.begin(), nInherited);
      std::move(it, solutions.end(), std::back_inserter(failed));
      solutions.erase(it, solutions.end());

      for (std::vector<std::shared_ptr<Solver>>::const_iterator
	     it = failed.begin(); it!=failed.end(); it++)
	{
	  const auto p = dist(eng, Distribution::param_type{0.0, 1.0});
	  std::vector<std::shared_ptr<Material>> layers;
	  if (p<pMutation)
	    layers = mutate(*it);
	  else
	    layers = (std::next(it) != failed.end()) ?
	      crossover(*it, *std::next(it)) : crossover(*it, *std::prev(it));
	  // layers = (it == failed.begin()) ?
	  //   crossover(*solutions.begin(), *it) : crossover(*it, *std::prev(it));

	  if (layers.size()!=0)
	    solutions.emplace_back(std::make_shared<Solver>(sdef, layers, -1));
	  else {
	    std::cerr << "Error: empty layers" << std::endl;
	    exit(1);
	  }
	}
    }
  }
}

double Optimiser::getFitness(const std::shared_ptr<Solver>& s) const
{
  // Return fitness function

  const double dose = s->getDose();
  const double mass = s->getMass();
  const size_t comp = s->getComplexity();

  return dose*doseWeight + mass*massWeight + comp*compWeight;
}

std::vector<std::shared_ptr<Material>> Optimiser::mutate(const std::shared_ptr<Solver>&) const
{
  // Implements uniform mutation in genetic algorithm in order to introduce
  // diversity into the sampled population

  // just return new random layers
  auto layers = getLayers();
  layers.insert(layers.end(), tail.begin(), tail.end()); // add tail
  return layers;
}

std::vector<std::shared_ptr<Material>> Optimiser::crossover(const std::shared_ptr<Solver>& s1,
							    const std::shared_ptr<Solver>& s2) const
{
  // Implements uniform crossover operator

  const std::vector<std::shared_ptr<Material>> l1 = s1->getLayers();
  const std::vector<std::shared_ptr<Material>> l2 = s2->getLayers();
  const size_t n = nLayers;

  std::vector<std::shared_ptr<Material>> layers; // TODO reserve (ant not push_back) - faster?

  for (size_t i=0; i<n; ++i) {
    const auto p = dist(eng, Distribution::param_type{0.0, 1.0}); // TODO: use int between 0 and 1
    if (p<0.5)
      layers.push_back(l1[i]);
    else
      layers.push_back(l2[i]);
  }

  layers.insert(layers.end(), tail.begin(), tail.end()); // add tail

  return layers;
}

void Optimiser::purge(std::vector<std::shared_ptr<Solver>>& solutions)
/*!
  Remove duplicate layers in the *sorted* vector of solutions
 */
{
  for (std::vector<std::shared_ptr<Solver>>::const_iterator
	 it = solutions.begin(); it!=solutions.end(); it++)
      {
	if (std::next(it) != solutions.end())
	  if (*(*it) == *(*std::next(it)))
	    solutions.erase(it);
      }
}
