#ifndef Solver_h
#define Solver_h

#include <iostream>

#include "Source.h"
#include "Material.h"

using ParticleID = char;
using TransportMatrix = Source;
using data_t = std::map<ParticleID, std::shared_ptr<TransportMatrix>>;

class Solver {
 private:
  std::map<ParticleID, std::shared_ptr<TH2D>>  sdef;   ///< source definition
  std::vector<std::shared_ptr<Material>> layers; ///< vector of materials
  const size_t nLayers;           ///< number of material layers
  size_t nReflectionLayers; // number of last layers where reflection should be taken into account
  size_t ReflectionOrder; // order of reflection to calculate from each of the above layers
  const std::set<ParticleID> particles; ///< set of transported particles
  data_t result;
  bool done; // true the run method was called
  std::shared_ptr<TH2D> emptySDEF; ///< TODO: delete
  size_t getFTDbin(const double, const std::vector<float>& ebins) const;
  data_t reflectOLD(const size_t layer);
  data_t reflect(const size_t layer);
  bool checkParticles() const;
  bool checkSDEF() const;
  void fillSDEF();
 public:
  Solver(const std::map<ParticleID, std::shared_ptr<TH2D>>&,
	 const std::vector<std::shared_ptr<Material>>&,
	 const int nr);
  //  virtual ~Solver() { std::cout << "Solver destructor" << std::endl; }
  data_t run(const size_t ro=1);
  void   save(const std::string&) const;
  double getNeutronFTD(const double) const;
  double getPhotonFTD(const double) const;
  double getProtonFTD(const double) const;
  double getElectronFTD(const double) const;
  double getMuonFTD(const double) const;
  double getFTD(const ParticleID, const double) const;
  double getDose() const;
  double getDose(const ParticleID) const;
  double getMass() const;
  size_t getComplexity() const;
  inline std::vector<std::shared_ptr<Material>> getLayers() const {return layers;}
  bool operator==(const Solver& rhs);
};

#endif
