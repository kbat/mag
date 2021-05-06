#include <iostream>

#include <Material.h>
#include <TKey.h>

Material::Material(const std::string& name,
		   const std::string& fname) :
  name(name)
{
  setParticles(fname);
  size_t n = particles.size();
  n *= n;
  T.reserve(n);
  R.reserve(n);

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
	std::pair<std::string, std::shared_ptr<TMatrixD> > m;
	m.first = hname;
	m.second = h2m(h);
	if (hname[1] == 'T')
	  T.push_back(m);
	else if (hname[1] == 'R')
	  R.push_back(m);
	else
	  std::cerr << "Unknown histogram: " << hname << std::endl;
      }
    }
    delete obj;
  }
  // sdef->Print();
  // std::cout << n << " " << T.size() << " " << R.size() << std::endl;
}


void Material::setParticles(const std::string& fname)
{
  TFile file(fname.data());
  TKey *key;
  TIter next(file.GetListOfKeys());
  while ((key = (TKey*)next()))
    if (strcmp(key->GetName(), "sdef") != 0)
      particles.insert(key->GetName()[0]);

  // for (std::set<char>::iterator it=particles.begin();
  //      it!=particles.end(); ++it)
  //   std::cout << ' ' << *it;
  // std::cout<<std::endl;
  file.Close();
}

std::shared_ptr<TMatrixD> Material::h2m(const TH2D *h) const
{
  // Convert TH2 to TMatrix

  const Int_t nx = h->GetNbinsX();
  const Int_t ny = h->GetNbinsY();

  std::shared_ptr<TMatrixD> m = std::make_shared<TMatrixD>(nx, ny);
  for (Int_t i=0; i<nx; ++i)
    for (Int_t j=0; j<ny; ++j)
      (*m)[i][j] = h->GetBinContent(i+1, j+1);

  return m;
}

std::shared_ptr<TMatrixD> Material::get(const std::string& name) const
{
  // Return matrix with the given name

  std::vector<std::pair<std::string, std::shared_ptr<TMatrixD> > > m =
    (name[1] == 'T') ? T : R;

  for (std::vector<std::pair<std::string,
	 std::shared_ptr<TMatrixD> > >::const_iterator it=m.begin();
       it!=m.end(); ++it)
    if (it->first == name)
      return it->second;

  std::cerr << "Matrix " << name << " not found" << std::endl;

  return nullptr;
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

void Material::print() const
{
  std::cout << "Material: " << name << std::endl;
}
