#ifndef Optimiser_h
#define Optimiser_h

#include <TH2D.h>
#include "Material.h"

class Optimiser {
 private:
  const char p0; //< incident particle ID
  const std::shared_ptr<TH2D> sdef; //< source definition
  std::set<std::shared_ptr<Material> > mat; //< library of materials
  const size_t nLayers; //< number of layers
  size_t RO;     //< Reflection Order
  size_t genSize;     //< number of solutions in a single generation
  double doseWeight;  //< dose weight in objective function
  double massWeight;  //< mass weight in objective function
  double compWeight;  //< complexity weight in objective function
  double inheritedFraction; //< fraction of solutions directly inherited to the next generation
  double pMutation; //< mutation probability (crossover = 1 - pMutation)
  bool checkSum(const std::map<std::shared_ptr<Material>, double>&) const;
  bool checkMat(const std::map<std::shared_ptr<Material>, double>&) const;
  std::vector<std::shared_ptr<Material>>
    getLayers(const std::shared_ptr<Material>&) const;
  std::vector<std::shared_ptr<Material>>
    getLayers(const std::map<std::shared_ptr<Material>, double>&) const;
  std::vector<std::shared_ptr<Material>> getLayers() const;

 public:
  Optimiser(const char, std::shared_ptr<TH2D>,
	    std::set<std::shared_ptr<Material> >&,
	    const size_t);
  void run(size_t);
  inline void setReflectionOrder(size_t ro) { RO = ro; }
  inline void setGenSize(size_t n) { genSize = n; }
  inline void setDoseWeight(double w) { doseWeight = w; }
  inline void setMassWeight(double w) { massWeight = w; }
  inline void setComplexityWeight(double w) { compWeight = w; }
  inline void setInheritedFraction(double f) { inheritedFraction = f; }
  inline void setPMutation(double val) { pMutation = val; }
  double getFitness(const std::shared_ptr<Solver>&) const;
  std::vector<std::shared_ptr<Material>> mutate(const std::shared_ptr<Solver>&) const;
  std::vector<std::shared_ptr<Material>>
    crossover(const std::shared_ptr<Solver>&, const std::shared_ptr<Solver>&) const;
};


#endif
