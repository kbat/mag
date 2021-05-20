#ifndef Optimiser_h
#define Optimiser_h

#include <TH2D.h>
#include "Material.h"

class Optimiser {
 private:
  const char p0; //< incident particle
  const std::shared_ptr<TH2D> sdef; //< source definition
  std::set<std::shared_ptr<Material> > mat; //< library of materials
  const size_t nLayers; //< number of layers
  bool checkSum(const std::map<std::shared_ptr<Material>, double>&);
  bool checkMat(const std::map<std::shared_ptr<Material>, double>&);
 public:
  Optimiser(const char, std::shared_ptr<TH2D>,
	    std::set<std::shared_ptr<Material> >&,
	    const size_t);
  std::vector<std::shared_ptr<Material>>
    getLayers(const std::map<std::shared_ptr<Material>, double>&);
  std::vector<std::shared_ptr<Material>>
    run(const size_t, const std::map<std::shared_ptr<Material>, double>&);
  std::vector<std::shared_ptr<Material>> run(const size_t);
  double getObjectiveFunction(const std::shared_ptr<Solver>&) const;
};


#endif
