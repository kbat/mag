#ifndef Markov_h
#define Markov_h

#include "Source.h"
#include "Material.h"
#include "Solver.h"

class Markov {
 private:
  std::vector<std::shared_ptr<Material>> layers;
  std::shared_ptr<TMatrixD> M;
  std::vector<std::vector<std::shared_ptr<TMatrixD>>> createMOP() const;
  void createMatrix();
 public:
  Markov(const std::vector<std::shared_ptr<Material>>&);

};

#endif
