#ifndef Markov_h
#define Markov_h

#include "Source.h"
#include "Material.h"
#include "Solver.h"

class Markov {
 private:
  data_t result; //< initially: sdef, after running: result
  std::vector<std::shared_ptr<Material>> layers; ///< vector of materials
  std::shared_ptr<TMatrixD> M;
  std::vector<std::vector<std::shared_ptr<TMatrixD>>> createMOP() const;
  void createMatrix();
 public:
  Markov(data_t&,const std::vector<std::shared_ptr<Material>>&);

};

#endif
