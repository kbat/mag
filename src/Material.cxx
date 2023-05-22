#include <iostream>

#include <Material.h>
#include <TKey.h>

Material::Material(const std::string& name,
		   const std::string& fname,
		   const size_t id,
		   const double& den) :
  name(name), id(id), den(den), empty(nullptr)
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
	  T.insert(m);
	else if (hname[1] == 'R')
	  R.insert(m);
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

  auto M = (name[1] == 'T') ? T : R;

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

std::shared_ptr<TMatrixD> Material::getEmpty()
{
  // Return empty matrix (for the Markov process)
  if (empty)
    return empty;

  const auto p = particles.begin(); // just any existing particle

  empty = std::make_shared<TMatrixD>(TMatrixD::kZero,*getT(*p,*p));

  return empty;
}

void Material::print() const
{
  std::cout << "Material: " << name << std::endl;
}
