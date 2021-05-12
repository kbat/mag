#include <iostream>

#include "Solver.h"

Solver::Solver(const char p0,
	       std::shared_ptr<TH2D> sdef,
	       std::vector<std::shared_ptr<Material>>& mat) :
  p0(p0), sdef(sdef), mat(mat),
  particles(mat[0]->getParticles())
{
  return;
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

double Solver::getNeutronFTD(const double E) const
{
  // Return flux-to-dose conversion factor for neutrons of the given energy
  // E : energy [MeV]

  static std::vector<float> ebins {1e-09, 1e-08, 2.5e-08, 1e-07, 2e-07, 5e-07, 1e-06, 2e-06, 5e-06, 1e-05, 2e-05, 5e-05, 0.0001, 0.0002, 0.0005, 0.001, 0.002, 0.005, 0.01, 0.02, 0.03, 0.05, 0.07, 0.1, 0.15, 0.2, 0.3, 0.5, 0.7, 0.9, 1, 1.2, 1.5, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 15, 16, 18, 20, 21, 30, 50, 75, 100, 130, 150, 180, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 2000, 5000, 10000};
  const double nbins = ebins.size();

  // Neutrons
  // ICRP-116 Flux to dose conversion factors
  // [#muSv/h per n/cm^{2}/s]
  static std::vector<float> data {0.0111, 0.0128, 0.0144, 0.0187, 0.0211, 0.0237, 0.0253, 0.0266, 0.0278, 0.0282, 0.0282, 0.0282, 0.028, 0.0278, 0.0271, 0.0271, 0.0274, 0.0287, 0.0328, 0.0439, 0.0565, 0.0828, 0.11, 0.151, 0.218, 0.284, 0.41, 0.637, 0.835, 1, 1.08, 1.19, 1.31, 1.47, 1.65, 1.74, 1.78, 1.79, 1.8, 1.8, 1.8, 1.8, 1.8, 1.78, 1.77, 1.76, 1.74, 1.72, 1.71, 1.63, 1.56, 1.58, 1.6, 1.61, 1.61, 1.61, 1.61, 1.67, 1.79, 1.92, 2.05, 2.16, 2.24, 2.3, 2.35, 2.76, 3.64, 4.75};

  size_t j(nbins-1); // last data bin by default
  for (size_t i=0; i<nbins; ++i)
    if (E<ebins[i]) {
      j=i;
      break;
    }

  return data[j];
}

double Solver::getPhotonFTD(const double E) const
{
  // Return flux-to-dose conversion factor for photons of the given energy
  // E : energy [MeV]

  static std::vector<float> ebins {0.01, 0.015, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.1,
				   0.15, 0.2, 0.3, 0.4, 0.5, 0.511, 0.6, 0.662, 0.8, 1, 1.12,
				   1.33, 1.5, 2, 3, 4, 5, 6, 6.13, 8, 10, 15, 20, 30, 40, 50,
				   60, 80, 100, 150, 200, 300, 400, 500, 600, 800, 1000, 1500,
				   2000, 3000, 4000, 5000, 6000, 8000, 10000};
  const double nbins = ebins.size();

  // Photons
  // ICRP-116 H*(10) values which are higher than the flux to dose conversion factors!
  // [#muSv/h per n/cm^{2}/s]
  static std::vector<float> data {0.000247, 0.000562, 0.00081, 0.00113, 0.00126, 0.00133,
				  0.0014, 0.00149, 0.0016, 0.00187, 0.00269, 0.0036, 0.00544,
				  0.0072, 0.00889, 0.00907, 0.0105, 0.0114, 0.0134, 0.0162,
				  0.0176, 0.0201, 0.022, 0.0269, 0.0351, 0.0421, 0.0482, 0.054,
				  0.0547, 0.067, 0.0792, 0.109, 0.138, 0.185, 0.223, 0.26,
				  0.295, 0.352, 0.396, 0.468, 0.515, 0.58, 0.619, 0.648, 0.67,
				  0.702, 0.724, 0.763, 0.792, 0.835, 0.875, 0.904, 0.929, 0.965,
				  0.994};

  size_t j(nbins-1); // last data bin by default
  for (size_t i=0; i<nbins; ++i)
    if (E<ebins[i]) {
      j=i;
      break;
    }

  return data[j];
}
