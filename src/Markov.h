#ifndef Markov_h
#define Markov_h

#include "Source.h"
#include "Material.h"
#include "Solver.h"

class Markov {
 private:
  data_t sdef; //< initially: sdef (corresponds to result in Solver.cxx)
  std::vector<std::shared_ptr<Material>> layers; ///< vector of materials
  std::shared_ptr<TMatrixDSparse> M;
  data_t result;
  std::vector<std::vector<std::shared_ptr<TMatrixD>>> createMOP() const;
  void createMatrix();
 public:
  Markov(const data_t&,const std::vector<std::shared_ptr<Material>>&);
  data_t run(const Double_t);
};

#endif
