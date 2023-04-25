#include <iostream>
#include <numeric>

#include "Solver.h"

Solver::Solver(const std::map<ParticleID, std::shared_ptr<TH2D>>&  sdef,
	       const std::vector<std::shared_ptr<Material>>& layers,
	       const int nr) :
  sdef(sdef), layers(layers), nLayers(layers.size()), nReflectionLayers(0),
  particles(layers[0]->getParticles())
{
  if (!checkParticles())
    exit(1);

  assert(checkSDEF());

  // An empty source histogram for transported particles not present in the 'sdef' map
  emptySDEF = std::make_shared<TH2D>(*(*sdef.begin()).second.get());
  emptySDEF->Reset();

  // Set the number of last layers where reflection should be taken
  // into account. If negative, reflection in all layers is calculated
  // (this is slow). Experiment with this number to set it to the
  // minimum value which does not yet significantly biases results.
  if (nr<0)
    nReflectionLayers = nLayers;
  else
    nReflectionLayers = std::min(nr,(int)nLayers);

  done = false;
}

bool Solver::checkSDEF() const
/*!
  Return false if any of the SDEF histograms is nullptr
 */
{
  bool val = true;
  for_each(sdef.begin(),sdef.end(),
	   [&val](const auto& s) {
	     if (s.second == nullptr) {
	       std::cerr << "Error: nullptr passed as SDEF histogram for " << s.first << std::endl;
	       val = false;
	     }
	   });

  return val;
}

bool Solver::checkParticles() const
/*!
  Return false if SDEF contains non-transported particles
*/
{
  bool val = true;
  for_each(sdef.begin(), sdef.end(),
	   [&](const auto &s){
	     if (!particles.count(s.first)) {
	       std::cerr << "Error: SDEF contains non-transported particle: " << s.first << std::endl;
	       val = false;
	     }
	   });
  return val;
}

void Solver::fillSDEF()
/*!
 Fill the 'result' map with sdef histograms. For the particles present in 'sdef'
 their corresponding histograms are used. For the other transported particles
 a copy of an empty histogram (emptySDEF) is used.
*/
{
  for_each(particles.begin(), particles.end(),
	   [&](const auto &p){
	     auto s = std::find_if(sdef.begin(), sdef.end(),
				   [&p](const auto &s){ return p == s.first; });

	     const std::shared_ptr<TH2D> h = (s == sdef.end()) ? emptySDEF : s->second;

	     result.insert(std::make_pair(p, std::make_shared<Source>(h.get())));
	   });
}

data_t Solver::reflect(const size_t layer)
/*!
  Implements reflections (firt and multpila orders).
  First order: reflect backwards from the current layer into the previous one;
  then reflect forward towards the current layer and transmit
  through the current layer.
  Second order: the same but two layers backward.
  etc
*/
{
  std::map<ParticleID, data_t > R;
  data_t tmp1, tmp2, tmp3, tmp4;

  // sum up contributions to i from different incident particles j
  auto sum = [&](data_t  &tmp) {
	       //tmp.clear(); // really needed?
	       for (auto i : particles) {
		 tmp[i] = std::make_shared<Source>(*R[i][i]);
		 for (auto j : particles) {
		   if (i!=j)
		     *tmp[i] += *R[j][i];
		 }
	       }
	     };

  enum direction {kR, kT};

  auto propagate = [&](data_t &src,
		       const std::shared_ptr<Material> &bb,
		       const direction dir) {
		     for (auto i : particles)   // incident
		       for (auto j : particles) { // scored
			 R[i][j] = std::make_shared<Source>(*src.at(i));
			 *R[i][j] *= (dir == kR) ? *bb->getR(i,j) : *bb->getT(i,j);
		       }
		     sum((src==result) ? tmp1 : src);
		     //		     R.clear(); // TODO: if called and the historam at one point is empty then no corresponding histogram in the output ROOT file.
		   };

  std::cout << "TODO: this can be ran in parallel:" << std::endl;

  // first order reflection
  if (layer>=1) {
    propagate(result, layers[layer], kR); // reflected back by the current layer
    tmp2 = tmp1;
    propagate(tmp1, layers[layer-1], kR); // reflecting from the previous layer to the current one
    propagate(tmp1, layers[layer], kT); // transmitting through the current layer
  }

  // // second order reflection
  // if (layer >=2) {
  //   propagate(tmp2, layers[layer-1], kT);
  //   tmp3 = tmp2;
  //   propagate(tmp2, layers[layer-2], kR);
  //   propagate(tmp2, layers[layer-1], kT);
  //   propagate(tmp2, layers[layer],   kT);

  //   for (auto i : particles)
  //     *tmp1[i] += *tmp2[i];
  // }

  // // third order reflection
  // if (layer >= 3) {
  //   propagate(tmp3, layers[layer-2], kT);
  //   tmp4 = tmp3;
  //   propagate(tmp3, layers[layer-3], kR);
  //   propagate(tmp3, layers[layer-2], kT);
  //   propagate(tmp3, layers[layer-1], kT);
  //   propagate(tmp3, layers[layer],   kT);

  //   for (auto i : particles)
  //     *tmp1[i] += *tmp3[i];
  // }

  // // fourth order reflection
  // if (layer >= 4) {
  //   propagate(tmp4, layers[layer-3], kT);
  //   propagate(tmp4, layers[layer-4], kR);
  //   propagate(tmp4, layers[layer-3], kT);
  //   propagate(tmp4, layers[layer-2], kT);
  //   propagate(tmp4, layers[layer-1], kT);
  //   propagate(tmp4, layers[layer],   kT);

  //   for (auto i : particles)
  //     *tmp1[i] += *tmp4[i];
  // }

  return tmp1;
}

data_t Solver::run(const size_t ro)
{
  // ro : reflection order to take into account [only ro<=1 implemented]

  if (done)
    return result;

  fillSDEF();


  for (size_t layer=0; layer<nLayers; ++layer) {
    // reflected spectra (if needed)
    data_t reflected;
    // transmitted[i][j]: transmitted spectra from incident particle i to j
    std::map<ParticleID, data_t> transmitted;

    // define all combinations of spectra after the 2nd layer
    // but before we do transport, we just copy data from result
    // because they will be the corresponding sdefs
    for (auto i : particles)     // incident
      for (auto j : particles) { // scored
	transmitted[i].insert(std::make_pair(j,
					     std::make_shared<Source>(*result[i])));
	*transmitted[i][j] *= *layers[layer]->getT(i,j); // transmitted through the current layer
      }

    // reflections
    const bool doReflect = (layer>0) && (layer>=nLayers-nReflectionLayers) && (ro>=1);
    //    std::cout << layer << " " << nLayers-nReflectionLayers << " " << std::flush;
    if (doReflect)// {
      //      std::cout << "reflect" << std::endl;
      reflected = reflect(layer);
    // } else
    //   std::cout << std::endl;

    // add up spectra of each secondary particle produced by different incidents
    result.clear();
    for (auto i : particles) {
      result[i] = std::make_shared<Source>(*transmitted[i][i]);
      for (auto j : particles)
	if (i!=j)
	  *result[i] += *transmitted[j][i];
    }

    if (doReflect)
      for (auto i : particles)
	*result[i] += *reflected[i];
  }

  done = true;

  return result;
}

void Solver::save(const std::string& fname) const
{
  TFile fout(fname.data(), "recreate");

  for_each(particles.begin(), particles.end(),
	   [&](const auto &p){
	     auto h = result.at(p)->Histogram(std::string(1, p));
	     h->Write();
	   });

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
  // This is a link to draft, the document is not released yet
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
  // This is a link to draft, the document is not released yet
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

double Solver::getFTD(const ParticleID p, const double E) const
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

  //  return getDose('n');

  // particles with flux-to-dose conversion factors available:
  static std::set<ParticleID> ftd {'n', 'p', 'e', '|', 'h'};

  return std::accumulate(particles.begin(), particles.end(), 0.0,
  		  [&](double prev, const auto p){
  		    return ftd.count(p) ? prev+getDose(p) : prev;
  		  });
}

double Solver::getDose(const ParticleID p) const
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

  return std::accumulate(layers.begin(), layers.end(), 0.0,
			 [](double prev, const auto m)
			 { return prev+m->getDensity(); });
}

size_t Solver::getComplexity() const
{
  // Return number of boundaries with different materials
  // TODO: use stl

  size_t sum = 1; // to avoid division by 0 in homogenic layers
  size_t n = layers.size();
  for (size_t i=1; i<n; ++i) {
    if (layers[i-1] != layers[i])
      sum++;
  }

  return sum;
}

bool Solver::operator==(const Solver& rhs)
{
  //  std::cout << "comparing" << std::endl;
  const auto rlayers = rhs.getLayers();
  const size_t n = rlayers.size();

  const auto llayers = getLayers();

  if (llayers.size() != n)
    return false;

  for (size_t i=0; i<n; ++i) {
    if (llayers[i]->getID() != rlayers[i]->getID())
      return false;
  }

  return true;
}
