#include <iostream>

#include "Solver.h"

Solver::Solver(const char p0,
	       std::shared_ptr<TH2D> sdef,
	       std::vector<std::shared_ptr<Material>>& mat) :
  p0(p0), sdef(sdef), mat(mat), particles(mat[0]->getParticles())
{
  std::cout << "Solver: constructor" << std::endl;
}

std::map<char, std::shared_ptr<Source> > Solver::run(const size_t nLayers)
{
  size_t layer=0;

  // LAYER 0
  // incident particle is e, but we still need individual sources for each particle,
  // they will be used at sources for the next layer
  for (auto p : particles) {
    result.insert(std::make_pair(p, std::make_shared<Source>(sdef.get())));
    *result[p] *= *mat[layer]->getT(p0, p);
  }

  // Now result contains spectra of individual particles leaving the first layer
  std::map<char, std::map<char, std::shared_ptr<Source> > > spectra2;

  for (size_t layer=1; layer<nLayers; ++layer) {
    std::cout << layer << std::endl;
    spectra2.clear();

    // define all combinations of spectra after the 2nd layer
    // but before we do transport, we just copy data from result
    // because they will be the corresponding sdefs
    for (auto i : particles)   // incident
      for (auto j : particles)  // scored
	spectra2[i].insert(std::make_pair(j,
					  std::make_shared<Source>(*result[i])));

    // transport through the 2nd layer (combine both series of loops in the future)
    for (auto i : particles)   // incident
      for (auto j : particles)  // scored
	*spectra2[i][j] *= *mat[layer]->getT(i,j);

    result.clear();

    // add up spectra of each secondary particle produced by different incidents
    for (auto i : particles) {
      result[i] = std::make_shared<Source>(*spectra2[i][i]);
      for (auto j : particles)
	if (i!=j)
	  *result[i] += *spectra2[j][i];
    }
  }

  return result;
}

void Solver::save(const std::string& fname)
{
  TFile fout(fname.data(), "recreate");

  for (auto p : particles)
    result[p]->Histogram(std::string(1, p))->Write();

  fout.Close();

}
