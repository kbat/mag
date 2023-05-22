#ifndef Markov_h
#define Markov_h

#include "Source.h"
#include "Material.h"
#include "Solver.h"

class Markov {
 private:
  std::vector<std::shared_ptr<Material>> layers;
  std::vector<std::vector<std::shared_ptr<TMatrixD>>> createMOP() const;
 public:
  Markov(const std::vector<std::shared_ptr<Material>>&);

};

#endif
