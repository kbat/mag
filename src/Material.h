#ifndef Material_h
#define Material_h

#include <set>
#include <TMatrixD.h>
#include <TH2D.h>
#include <TFile.h>

class Material
{
 private:
  const std::string name;         ///< material name
  const size_t id;                ///< material number
  const double den;               ///< mass density [g/cm3]
  std::set<char> particles; ///< set of incident/scored particles
  ///! transmission matrices and their names
  std::map<std::string, std::shared_ptr<TMatrixD> > mapT;
  ///! reflection matrices and their names
  std::map<std::string, std::shared_ptr<TMatrixD> > mapR;
  std::shared_ptr<TH2D> sdef; ///< [empty] sdef histogram
  std::shared_ptr<TMatrixD> empty; ///< empty matrix (for the Markov process)
  std::shared_ptr<TMatrixD> unit; ///< identity matrix (for the Markov process)

  // TODO: use sparse matrices for T and R (test if this is more efficient)
  std::shared_ptr<TMatrixD> T; ///< transmission matrix for all particles (for the Markov process)
  std::shared_ptr<TMatrixD> R; ///< reflection matrix for all particles (for the Markov process)

  std::shared_ptr<TMatrixD> h2m(const TH2D *h) const;
  std::shared_ptr<TMatrixD> get(const std::string&) const;
 public:
  Material(const std::string&, const std::string&, const size_t, const double&);
  std::shared_ptr<TMatrixD> getT(const char, const char) const;
  std::shared_ptr<TMatrixD> getR(const char, const char) const;
  std::shared_ptr<TMatrixD> getT();
  std::shared_ptr<TMatrixD> getR();
  std::shared_ptr<TMatrixD> getEmpty();
  std::shared_ptr<TMatrixD> getUnit();
  // to prevent data loss conversion if size_t passed instead of char
  std::shared_ptr<TMatrixD> getT(const size_t, const size_t) const = delete;
  std::shared_ptr<TMatrixD> getR(const size_t, const size_t) const = delete;
  inline std::shared_ptr<TH2D> getSDEF() const { return sdef; }
  inline std::set<char> getParticles() const { return particles; }
  void print() const;
  inline std::string  getName() const { return name; }
  inline size_t getID() const { return id; }
  inline double getDensity() const { return den; }
};

#endif

/* TODO
 * implement the checkMatrices function to be sure all the matrices are of the same shape
 * if try to avoid returning matrices. If do it, return TMatrixTLazy instead
 */
