#include "Markov.h"

Markov::Markov(const std::vector<std::shared_ptr<Material>>& layers) :
  layers(layers)
{
  const auto mop = createMOP();
}

std::vector<std::vector<std::shared_ptr<TMatrixD>>> Markov::createMOP() const
{
  // Create the block matrix of pointers

  std::vector<std::vector<std::shared_ptr<TMatrixD>>> mop;

  const size_t N = layers.size();
  // An additional layers: N=1th - transmission forward from the last layer:
  const size_t nx = N+1;
  // An additonal layer to account for the reflection matrix
  const size_t ny = N+1;

  const auto empty = layers[0]->getEmpty();
  std::shared_ptr<TMatrixD> m = nullptr;
  std::string title; // tmp

  for (size_t j=0; j<ny; ++j) {
    std::vector<std::shared_ptr<TMatrixD>> row;
    for (size_t i=0; i<nx; ++i) {
      if ((i==j) && (i!=nx-1)) {
	m = empty;
	title = "A" + std::to_string(i);
      } else if (i==j+1) { // last layer - only reflection
	m = layers[i-1]->getT('n','n');
	title = "T" + std::to_string(i-1);
      } else if (j==i+1) {
	m = layers[i]->getR('n','n');
	title = "R"  + std::to_string(i);
      } else {
	m = empty;
	title = " 0";
      }
      row.push_back(m);
      std::cout << title << " ";
    }
    std::cout << std::endl;
    mop.push_back(row);
  }

  return mop;
}
