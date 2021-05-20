#ifndef Solver_h
#define Solver_h

#include <iostream>

#include "Source.h"
#include "Material.h"

class Solver {
 private:
  const char p0;                  ///< incident particle
  std::shared_ptr<TH2D> sdef;     ///< source definition
  std::vector<std::shared_ptr<Material>> mat; ///< vector of materials
  const size_t nLayers;           ///< number of material layers
  const std::set<char> particles; ///< set of transported particles
  std::map<char, std::shared_ptr<Source> > result;
  size_t getFTDbin(const double, const std::vector<float>& ebins) const;
  std::map<char, std::shared_ptr<Source> > reflect(const size_t layer);
 public:
  Solver(const char, std::shared_ptr<TH2D>, std::vector<std::shared_ptr<Material>> &);
  //  virtual ~Solver() { std::cout << "Solver destructor" << std::endl; }
  std::map<char, std::shared_ptr<Source> > run(const size_t ro=1);
  void save(const std::string&) const;
  double getNeutronFTD(const double) const;
  double getPhotonFTD(const double) const;
  double getProtonFTD(const double) const;
  double getElectronFTD(const double) const;
  double getMuonFTD(const double) const;
  double getFTD(const char, const double) const;
  double getDose() const;
  double getDose(const char) const;
  double getMass() const;
};


#endif
