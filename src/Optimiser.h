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
  bool checkSum(const std::map<std::shared_ptr<Material>, double>&);
  bool checkMat(const std::map<std::shared_ptr<Material>, double>&);
  std::vector<std::shared_ptr<Material>>
    getLayers(const std::shared_ptr<Material>&);
  std::vector<std::shared_ptr<Material>>
    getLayers(const std::map<std::shared_ptr<Material>, double>&);
  std::vector<std::shared_ptr<Material>> getLayers();

 public:
  Optimiser(const char, std::shared_ptr<TH2D>,
	    std::set<std::shared_ptr<Material> >&,
	    const size_t);
  void run(size_t);
  inline void setReflectionOrder(size_t ro) { RO = ro; }
  inline void setGenSize(size_t n) { genSize = n; }
  double getObjectiveFunction(const std::shared_ptr<Solver>&) const;
};


#endif
