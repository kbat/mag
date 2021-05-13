#ifndef Solver_h
#define Solver_h

#include "Source.h"
#include "Material.h"

class Solver {
 private:
  const char p0;                  ///< incident particle
  std::shared_ptr<TH2D> sdef;     ///< source definition
  std::vector<std::shared_ptr<Material>> mat; ///< vector of materials
  const std::set<char> particles; ///< set of transported particles
  std::map<char, std::shared_ptr<Source> > result;
  size_t getFTDbin(const double, const std::vector<float>& ebins) const;
 public:
  Solver(const char, std::shared_ptr<TH2D>, std::vector<std::shared_ptr<Material>> &);
  std::map<char, std::shared_ptr<Source> > run(const size_t);
  void save(const std::string&);
  double getNeutronFTD(const double) const;
  double getPhotonFTD(const double) const;
  double getProtonFTD(const double) const;
  double getFTD(const char, const double) const;
  double getDose();
};


#endif
