#include <iostream>
#include <numeric>

#include "Solver.h"

Solver::Solver(const char p0,
	       std::shared_ptr<TH2D> sdef,
	       std::vector<std::shared_ptr<Material>>& mat) :
  p0(p0), sdef(sdef), mat(mat), nLayers(mat.size()),
  particles(mat[0]->getParticles())
{
  return;
}

std::map<char, std::shared_ptr<Source> > Solver::reflect(const size_t layer)
{
  // Implements the first-order reflection:
  // reflect backwards from the current layer into the previous one;
  // then reflect forward towards the current layer and transmit
  // through the current layer

  std::map<char, std::map<char, std::shared_ptr<Source> > > R;
  std::map<char, std::shared_ptr<Source> >  tmp;

  // sum up contributions to i from different incident particles j
  auto sum = [&] {
	       tmp.clear(); // really needed?
	       for (auto i : particles) {
		 tmp[i] = std::make_shared<Source>(*R[i][i]);
		 for (auto j : particles) {
		   if (i!=j)
		     *tmp[i] += *R[j][i];
		 }
	       }
	     };

  // reflecting back
  for (auto i : particles)   // incident
    for (auto j : particles) { // scored
      R[i][j] = std::make_shared<Source>(*result[i]);
      *R[i][j] *= *mat[layer]->getR(i,j); // reflected back by the current layer
    }

  sum();

  // reflecting forward
  R.clear();
  for (auto i : particles)
    for (auto j : particles) {
      R[i][j] = std::make_shared<Source>(*tmp[i]);
      *R[i][j] *= *mat[layer-1]->getR(i,j);
    }

  sum();

  // transmitting through the current layer
  R.clear();
  for (auto i : particles)
    for (auto j : particles) {
      R[i][j] = std::make_shared<Source>(*tmp[i]);
      *R[i][j] *= *mat[layer]->getT(i,j);
    }

  sum();

  return tmp; // std::move?
}

std::map<char, std::shared_ptr<Source> > Solver::run(const size_t ro)
{
  // ro : reflection order to take into account [only ro<=1 implemented]

  size_t layer=0;

  // LAYER 0
  // incident particle is e, but we still need individual sources for each particle,
  // they will be used at sources for the next layer

  for (auto p : particles) {
    result.insert(std::make_pair(p, std::make_shared<Source>(sdef.get())));
    *result[p] *= *mat[layer]->getT(p0, p);
  }

  // Now result contains spectra of individual particles leaving the first layer
  // transmitted[i][j]: transmitted spectra from incident particle i to j
  std::map<char, std::shared_ptr<Source> > reflected;

  for (size_t layer=1; layer<nLayers; ++layer) {
    std::map<char, std::map<char, std::shared_ptr<Source> > > transmitted;

    // define all combinations of spectra after the 2nd layer
    // but before we do transport, we just copy data from result
    // because they will be the corresponding sdefs
    for (auto i : particles)   // incident
      for (auto j : particles) {// scored
	transmitted[i].insert(std::make_pair(j,
					     std::make_shared<Source>(*result[i])));
	*transmitted[i][j] *= *mat[layer]->getT(i,j); // transmitted through the current layer
      }

    // reflections
    if (ro>=1)
      reflected = reflect(layer);

    // add up spectra of each secondary particle produced by different incidents
    result.clear();
    for (auto i : particles) {
      result[i] = std::make_shared<Source>(*transmitted[i][i]);
      for (auto j : particles)
	if (i!=j)
	  *result[i] += *transmitted[j][i];
    }

    if (ro>=1)
      for (auto i : particles)
	*result[i] += *reflected[i];
  }

  return result;
}

void Solver::save(const std::string& fname) const
{
  TFile fout(fname.data(), "recreate");

  for (auto p : particles)
    result.at(p)->Histogram(std::string(1, p))->Write();

  fout.Close();

}

size_t Solver::getFTDbin(const double E, const std::vector<float>& ebins) const
{
  // Return bin number in the FTD data vector
  // E : energy [MeV]
  // ebins : energy bins vector

  const size_t nbins = ebins.size();
  size_t j(nbins-1); // last data bin by default
  for (size_t i=0; i<nbins; ++i)
    if (E<ebins[i]) {
      j=i;
      break;
    }

  return j;
}

double Solver::getNeutronFTD(const double E) const
{
  // Return the flux-to-dose proton ICRP-116 conversion factors in
  // uSv/hour per neutron/cm2/sec Reference: ESS-0019931, Table 1.
  // E : energy [MeV]

  static std::vector<float> ebins {1e-09, 1e-08, 2.5e-08, 1e-07, 2e-07, 5e-07, 1e-06, 2e-06,
				   5e-06, 1e-05, 2e-05, 5e-05, 0.0001, 0.0002, 0.0005, 0.001,
				   0.002, 0.005, 0.01, 0.02, 0.03, 0.05, 0.07, 0.1, 0.15, 0.2,
				   0.3, 0.5, 0.7, 0.9, 1, 1.2, 1.5, 2, 3, 4, 5, 6, 7, 8, 9, 10,
				   12, 14, 15, 16, 18, 20, 21, 30, 50, 75, 100, 130, 150, 180,
				   200, 300, 400, 500, 600, 700, 800, 900, 1000, 2000, 5000, 10000};

  static std::vector<float> data {0.0111, 0.0128, 0.0144, 0.0187, 0.0211, 0.0237, 0.0253, 0.0266,
				  0.0278, 0.0282, 0.0282, 0.0282, 0.028, 0.0278, 0.0271, 0.0271,
				  0.0274, 0.0287, 0.0328, 0.0439, 0.0565, 0.0828, 0.11, 0.151,
				  0.218, 0.284, 0.41, 0.637, 0.835, 1, 1.08, 1.19, 1.31, 1.47, 1.65,
				  1.74, 1.78, 1.79, 1.8, 1.8, 1.8, 1.8, 1.8, 1.78, 1.77, 1.76, 1.74,
				  1.72, 1.71, 1.63, 1.56, 1.58, 1.6, 1.61, 1.61, 1.61, 1.61, 1.67,
				  1.79, 1.92, 2.05, 2.16, 2.24, 2.3, 2.35, 2.76, 3.64, 4.75};

  const size_t j = getFTDbin(E, ebins);

  return data[j];
}

double Solver::getPhotonFTD(const double E) const
{
  // Return the flux-to-dose proton ICRP-116 conversion factors in
  // uSv/hour per photon/cm2/sec Reference: ESS-0019931, Table 2.
  // E : energy [MeV]

  static std::vector<float> ebins {0.01, 0.015, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.1,
				   0.15, 0.2, 0.3, 0.4, 0.5, 0.511, 0.6, 0.662, 0.8, 1, 1.12,
				   1.33, 1.5, 2, 3, 4, 5, 6, 6.13, 8, 10, 15, 20, 30, 40, 50,
				   60, 80, 100, 150, 200, 300, 400, 500, 600, 800, 1000, 1500,
				   2000, 3000, 4000, 5000, 6000, 8000, 10000};

  static std::vector<float> data {0.000247, 0.000562, 0.00081, 0.00113, 0.00126, 0.00133,
				  0.0014, 0.00149, 0.0016, 0.00187, 0.00269, 0.0036, 0.00544,
				  0.0072, 0.00889, 0.00907, 0.0105, 0.0114, 0.0134, 0.0162,
				  0.0176, 0.0201, 0.022, 0.0269, 0.0351, 0.0421, 0.0482, 0.054,
				  0.0547, 0.067, 0.0792, 0.109, 0.138, 0.185, 0.223, 0.26,
				  0.295, 0.352, 0.396, 0.468, 0.515, 0.58, 0.619, 0.648, 0.67,
				  0.702, 0.724, 0.763, 0.792, 0.835, 0.875, 0.904, 0.929, 0.965,
				  0.994};

  const size_t j = getFTDbin(E, ebins);

  return data[j];
}

double Solver::getProtonFTD(const double E) const
{
  // Return the flux-to-dose proton ICRP-116 conversion factors in
  // uSv/hour per proton/cm2/sec Reference: ESS-0019931, Table 3.
  // E : energy [MeV]

  static std::vector<float> ebins{1, 1.5, 2, 3, 4, 5, 6, 8, 10, 15, 20, 30, 40, 50, 60, 80, 100,
				  150, 200, 300, 400, 500, 600, 800, 1000, 1500, 2000, 3000, 4000,
				  5000, 6000, 8000, 10000};

  static std::vector<float> data{0.0197, 0.0296, 0.0392, 0.059, 0.0788, 0.0983, 0.118, 0.157,
				 0.198, 0.68, 1.54, 2.7, 3.67, 4.25, 5.33, 7.78, 9.04, 10.2,
				 7.85, 5.22, 4.68, 4.46, 4.43, 4.43, 4.43, 4.5, 4.61, 4.82,
				 5.04, 5.22, 5.65, 6.16, 6.41};

  const size_t j = getFTDbin(E, ebins);

  return data[j];
}

double Solver::getElectronFTD(const double E) const
{
  // Return the flux-to-dose proton ICRP 2010 conversion factors in
  // uSv/hour per electron/cm2/sec Reference: Table A1.3 page 63 in
  // ICRP report OPERATIONAL QUANTITIES FOR EXTERNAL RADIATION EXPOSURE
  // This is a link to draft, the document was not released yet
  // https://www.icrp.org/docs/ICRU%20and%20ICRP%20Draft%20Joint%20Report%20Operational%20Quantities%20for%20External%20Radiation%20Exposure.pdf
  // The values from the table were divided by 277 in order to
  // convert from electron fluence per ambient dose to uSv/h per electron/cm2/sec
  // E : energy [MeV]

  static std::vector<float> ebins{0.01, 0.015, 0.02, 0.03, 0.04, 0.05, 0.06, 0.08, 0.1, 0.15,
				  0.2, 0.3, 0.4, 0.5, 0.6, 0.8, 1, 1.5, 2, 3, 4, 5, 6, 8, 10,
				  15, 20, 30, 40, 50, 60, 80, 100, 150, 200, 300, 400, 500, 600,
				  800, 1000, 1500, 2000, 3000, 4000, 5000, 6000, 8000, 10000};
  static std::vector<float> data{0.00010, 0.00015, 0.00019, 0.00029, 0.00039, 0.00049, 0.00059,
				 0.00079, 0.00099, 0.00151, 0.00205, 0.00321, 0.00448, 0.00588,
				 0.00740, 0.01458, 0.02563, 0.05415, 0.08087, 0.13032, 0.17401,
				 0.21408, 0.25487, 0.35343, 0.45126, 0.67870, 0.85199, 1.09025,
				 1.18773, 1.21661, 1.24188, 1.29242, 1.32130, 1.36823, 1.40072,
				 1.48375, 1.57040, 1.62094, 1.67509, 1.76173, 1.83394, 1.89531,
				 2.05054, 2.19495, 2.30325, 2.38628, 2.46570, 2.58484, 2.67870};

  const size_t j = getFTDbin(E, ebins);

  return data[j];
}

double Solver::getMuonFTD(const double E) const
{
  // Return the flux-to-dose proton ICRP 2010 conversion factors in
  // uSv/hour per mu-/cm2/sec Reference: Table A1.6 on page 69 in
  // ICRP report OPERATIONAL QUANTITIES FOR EXTERNAL RADIATION EXPOSURE
  // This is a link to draft, the document was not released yet
  // https://www.icrp.org/docs/ICRU%20and%20ICRP%20Draft%20Joint%20Report%20Operational%20Quantities%20for%20External%20Radiation%20Exposure.pdf
  // The values from the table were divided by 277 in order to
  // convert from mu- fluence per ambient dose to uSv/h per mu-/cm2/sec
  // mu+ factors are slightly different (table A.1.7), but here we assume them to be the same as mu-
  // E : energy [MeV]

  static std::vector<float> ebins{1, 1.5, 2, 3, 4, 5, 6, 8, 10, 15, 20, 30, 40, 50, 60, 80, 100,
				  150, 200, 300, 400, 500, 600, 800, 1000, 1500, 2000, 3000, 4000,
				  5000, 6000, 8000, 10000};


  static std::vector<float> data{0.64982, 0.64982, 0.66426, 0.67870, 0.69675, 0.74007, 0.87365,
				 1.05776, 1.19856, 1.49458, 1.67870, 2.37184, 2.65343, 2.72563,
				 2.79783, 1.82310, 1.57040, 1.28159, 1.20217, 1.16245, 1.16245,
				 1.16968, 1.18412, 1.20217, 1.23466, 1.22022, 1.23105, 1.24188,
				 1.25271, 1.25632, 1.25271, 1.25993, 1.25993};

  const size_t j = getFTDbin(E, ebins);

  return data[j];
}

double Solver::getFTD(const char p, const double E) const
{
  // Return flux to dose conversion factor for the particle of the given energy
  // p : MCNP particle ID
  // E : energy [MeV]
  if (p == 'n')
    return getNeutronFTD(E);
  else if (p == 'p')
    return getPhotonFTD(E);
  else if (p == 'e')
    return getElectronFTD(E);
  else if (p == '|')
    return getMuonFTD(E);
  else if (p == 'h')
    return getProtonFTD(E);
  else {
    std::cerr << "getFTD: particle " << p << " not supported" << std::endl;
  }
  return 0.0;
}

double Solver::getDose() const
{
  // Return dose rate [uSv/h per primary particle normalisation]
  // due to all supported particles

  double D = 0.0;
  // particles with flux-to-dose conversion factors available:
  static std::set<char> ftd {'n', 'p', 'e', '|', 'h'};

  for (auto i : particles)
    if (ftd.count(i)) {
      double d = getDose(i);
      D += d;
    }

  return D;
}

double Solver::getDose(const char p) const
{
  // Return dose rate contribution from the specified particle p
  // [uSv/h per primary particle normalisation]
  // p : MCNP particle ID

  //  return result.at(p)->Histogram(std::string(1, p))->Integral();

  double D = 0.0;

  TH1D *h = result.at(p)->Histogram(std::string(1, p))->ProjectionX();
  const Int_t nbins = h->GetNbinsX();

  for (Int_t bin=1; bin<=nbins; ++bin)
    D += getFTD(p,h->GetXaxis()->GetBinUpEdge(bin))*h->GetBinContent(bin);

  delete h;

  return D;
}

double Solver::getMass() const
{
  // Return sum of densities of all materials in the vector

  return std::accumulate(mat.begin(), mat.end(), 0.0,
			 [](double prev, const auto m)
			 { return prev+m->getDensity(); });
}

size_t Solver::getComplexity() const
{
  // Return number of boundaries with different materials
  // TODO: use stl

  size_t sum = 0;
  size_t n = mat.size();
  for (size_t i=1; i<n; ++i) {
    if (mat[i-1] != mat[i])
      sum++;
  }

  return sum;
}
