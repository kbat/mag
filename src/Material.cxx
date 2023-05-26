#include <iostream>

#include <Material.h>
#include <TKey.h>

Material::Material(const std::string& name,
		   const std::string& fname,
		   const size_t id,
		   const double& den) :
  name(name), id(id), den(den), empty(nullptr), unit(nullptr), T(nullptr), R(nullptr)
{
  // Constructor

  TFile file(fname.data());
  TIter next(file.GetListOfKeys());

  while (TKey *key = (TKey*)next()) {
    TObject *obj = file.Get(key->GetName()); // copy object to memory
    if (obj->InheritsFrom("TH2D")) {
      const TH2D *h = dynamic_cast<TH2D*>(obj);
      const std::string hname = h->GetName();
      if (hname == "sdef")
	sdef = std::make_shared<TH2D>(*h);
      else {
	particles.insert(hname[0]);

	auto m = std::make_pair(hname, h2m(h));
	if (hname[1] == 'T')
	  mapT.insert(m);
	else if (hname[1] == 'R')
	  mapR.insert(m);
	else
	  std::cerr << "Unknown histogram: " << hname << std::endl;
      }
    }
    delete obj;
  }

  file.Close();
}

std::shared_ptr<TMatrixD> Material::h2m(const TH2D *h) const
{
  // Convert TH2 to TMatrix

  const Int_t nx = h->GetNbinsX();
  const Int_t ny = h->GetNbinsY();

  assert(nx==ny && "nx != ny");

  auto m = std::make_shared<TMatrixD>(nx, ny);

  for (Int_t i=0; i<nx; ++i)
    for (Int_t j=0; j<ny; ++j)
      (*m)[i][j] = h->GetBinContent(j+1, i+1);

  return m;
}

std::shared_ptr<TMatrixD> Material::get(const std::string& name) const
{
  // Return matrix with the given name

  auto M = (name[1] == 'T') ? mapT : mapR;

  return M[name];
}

std::shared_ptr<TMatrixD> Material::getT(const char pIn,
					 const char pOut) const
{
  // Return transmission matrix given incident and scored particles

  std::string name;
  name += pIn;
  name += 'T';
  name += pOut;

  return get(name);
}

std::shared_ptr<TMatrixD> Material::getR(const char pIn,
					 const char pOut) const
{
  // Return reflection matrix given incident and scored particles

  std::string name;
  name += pIn;
  name += 'R';
  name += pOut;

  return get(name);
}

std::shared_ptr<TMatrixD> Material::getT()
{
  // Return transmission matrix for all paticles

  if (T) return T;

  const Int_t nParticles = particles.size();
  const Int_t ny = mapT.begin()->second->GetNrows();
  const Int_t nx = mapT.begin()->second->GetNcols();
  const Int_t nrows = ny*nParticles;
  const Int_t ncols = nx*nParticles;

  assert(ncols==nrows);

  T = std::make_shared<TMatrixD>(nrows,ncols);

  // Filling this matrix is similar to Markov::createMatrix
  Int_t i(0), j(0);
  for (const auto p1 : particles) { // mopx
    for (Int_t x=0; x<nx; ++x) {
      for (const auto p2 : particles) { // mopy
	const auto m = getT(p1,p2);
	for (Int_t y=0; y<ny; ++y) {
	  const Double_t val = (*m)[x][y];
	  (*T)[j][i] = val;
	  i++;
	}
      }
      i=0;
      j++;
    }
  }

  return T;
}

std::shared_ptr<TMatrixD> Material::getR()
{
  // Return reflection matrix for all paticles
  // TODO: this method is the same as Material::getT() apart from the
  // getR/getT methods. Reimplement to avoid the same code.

  if (R) return R;

  const Int_t nParticles = particles.size();
  const Int_t ny = mapT.begin()->second->GetNrows();
  const Int_t nx = mapT.begin()->second->GetNcols();
  const Int_t nrows = ny*nParticles;
  const Int_t ncols = nx*nParticles;

  assert(ncols==nrows);

  R = std::make_shared<TMatrixD>(nrows,ncols);

  // Filling this matrix is similar to Markov::createMatrix
  Int_t i(0), j(0);
  for (const auto p1 : particles) { // mopx
    for (Int_t x=0; x<nx; ++x) {
      for (const auto p2 : particles) { // mopy
	const auto m = getR(p1,p2);
	for (Int_t y=0; y<ny; ++y) {
	  const Double_t val = (*m)[x][y];
	  (*R)[j][i] = val;
	  i++;
	}
      }
      i=0;
      j++;
    }
  }

  return R;
}


std::shared_ptr<TMatrixD> Material::getEmpty()
{
  // Return empty matrix (for the Markov process)

  if (empty)
    return empty;

  empty = std::make_shared<TMatrixD>(TMatrixD::kZero,*getT());

  return empty;
}

std::shared_ptr<TMatrixD> Material::getUnit()
{
  // Return unit (identity) matrix (for the Markov process)

  if (unit)
    return unit;

  unit = std::make_shared<TMatrixD>(TMatrixD::kUnit,*getT());

  return unit;
}

void Material::print() const
{
  std::cout << "Material: " << name << std::endl;
}
