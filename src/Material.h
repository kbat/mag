#ifndef Material_h
#define Material_h

#include <set>
#include <TMatrixD.h>
#include <TH2D.h>
#include <TFile.h>

class Material
{
 private:
  std::string name;         ///< material name
  std::set<char> particles; ///< set of incident/scored particles
  ///! transmission matrices and their names
  std::map<std::string, std::shared_ptr<TMatrixD> > T;
  ///! reflection matrices and their names
  std::map<std::string, std::shared_ptr<TMatrixD> > R;
  std::shared_ptr<TH2D> sdef; ///< [empty] sdef histogram
  double den;                 ///< mass density [g/cm3]

  std::shared_ptr<TMatrixD> h2m(const TH2D *h) const;
 public:
  Material(const std::string&, const std::string&, const double&);
  std::shared_ptr<TMatrixD> get(const std::string&) const;
  std::shared_ptr<TMatrixD> getT(const char, const char) const;
  std::shared_ptr<TMatrixD> getR(const char, const char) const;
  inline std::shared_ptr<TH2D> getSDEF() const { return sdef; }
  inline std::set<char> getParticles() const { return particles; }
  void print() const;
  double getDensity() const { return den; }
};

#endif
