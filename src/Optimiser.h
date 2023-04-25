#ifndef Optimiser_h
#define Optimiser_h

#include <TH2D.h>
#include "Material.h"

class Optimiser {
 private:
  std::map<char, std::shared_ptr<TH2D>> sdef;   ///< source definition
  std::set<std::shared_ptr<Material> > mat; //< library of materials to optimise
  std::vector<std::shared_ptr<Material> > tail; //< fixed configuration of tail (not optimised)
  const size_t nLayers; //< number of layers
  size_t RO;     //< Reflection Order
  size_t minRandomPopulation;     //< number of solutions in a single generation
  double doseWeight;  //< dose weight in objective function
  double massWeight;  //< mass weight in objective function
  double compWeight;  //< complexity weight in objective function
  size_t nInherited; //< number of solutions directly inherited into the next generation
  double pMutation; //< mutation probability (crossover = 1 - pMutation)
  int    nprint;    //< number of best solutions printed at each generation

  bool checkSum(const std::map<std::shared_ptr<Material>, double>&) const;
  bool checkMat(const std::map<std::shared_ptr<Material>, double>&) const;
  std::vector<std::shared_ptr<Material>>
    getLayers(const std::shared_ptr<Material>&) const;
  std::vector<std::shared_ptr<Material>>
    getLayers(const std::shared_ptr<Material>&,const std::shared_ptr<Material>&, size_t) const;
  std::vector<std::shared_ptr<Material>>
    getLayers(const std::map<std::shared_ptr<Material>, double>&) const;
  std::vector<std::shared_ptr<Material>> getLayers() const;

 public:
  Optimiser(const std::map<char, std::shared_ptr<TH2D>>&,
	    const std::set<std::shared_ptr<Material> >&,
	    const size_t);
  void run(size_t);
  inline void setReflectionOrder(size_t ro) { RO = ro; }
  void setMinRandomPopulation(size_t);
  inline void setDoseWeight(double w) { doseWeight = w; }
  inline void setMassWeight(double w) { massWeight = w; }
  inline void setComplexityWeight(double w) { compWeight = w; }
  inline void setNInherited(size_t n) { nInherited = n; }
  inline void setPMutation(double val) { pMutation = val; }
  double getFitness(const std::shared_ptr<Solver>&) const;
  std::vector<std::shared_ptr<Material>> mutate(const std::shared_ptr<Solver>&) const;
  std::vector<std::shared_ptr<Material>>
    crossover(const std::shared_ptr<Solver>&, const std::shared_ptr<Solver>&) const;
  inline void setNPrint(int n) { nprint =n ;}
  inline void setTail(const std::vector<std::shared_ptr<Material>>& t) {tail = t;}
  void purge(std::vector<std::shared_ptr<Solver>>&);
};


#endif
