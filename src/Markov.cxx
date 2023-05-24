#include "Markov.h"

Markov::Markov(const data_t& sdef,
	       const std::vector<std::shared_ptr<Material>>& layers) :
  sdef(sdef), layers(layers), M(nullptr), result({})
{
  createMatrix();
}

std::vector<std::vector<std::shared_ptr<TMatrixD>>> Markov::createMOP() const
{
  // Create the block matrix of pointers

  std::vector<std::vector<std::shared_ptr<TMatrixD>>> mop;

  const size_t N = layers.size();
  // An additional layers: N=1th - transmission forward from the last layer:
  const size_t nx = N+2;
  // An additonal layer to account for the reflection matrix
  const size_t ny = N+2;

  const auto empty = layers[0]->getEmpty();
  const auto unit = layers[0]->getUnit();
  std::shared_ptr<TMatrixD> m = nullptr;
  std::string title; // tmp

  for (size_t j=0; j<ny; ++j) {
    std::vector<std::shared_ptr<TMatrixD>> row;
    for (size_t i=0; i<nx; ++i) {
      if (j==ny-1) {
	m = empty;
	title = "L0";
      } else if (i==j) {
	if (i==0) {
	  m = empty;
	  title = " 0";
	} else {
	  m = empty;
	  title = "A" + std::to_string(i-1);
	}
      } else if (i==j+1) { // last layer - only reflection
	if (j==0) {
	  m = unit;
	  title = " E";
	} else {
	  m = layers[i-2]->getT('n','n');
	  title = "T" + std::to_string(i-2);
	}
      } else if (j==i+1) {
	m = layers[i]->getR('n','n');
	title = "R"  + std::to_string(i);
      } else {
       	m = empty;
       	title = " 0";
      }
      row.push_back(m);
      std::cout << title << " ";
    }
    std::cout << std::endl;
    mop.push_back(row);
  }

  return mop;
}

void Markov::createMatrix()
{
  const auto mop = createMOP();

  //  const size_t nx = (*mop.begin()).size();
  const Int_t nx = mop[0].size();
  const Int_t ny = mop.size();

  assert(nx == ny);

  std::cout << "createMatrix: block matrix shape: " << nx << " " << ny << std::endl;

  const auto m0 = mop[0][0];
  const Int_t NX = m0->GetNcols() * nx;
  const Int_t NY = m0->GetNrows() * ny;

  std::cout << "createMatrix: Markov process matrix shape: " << NX << " " << NY << std::endl;

  // TODO: try to use sparse matrix and its GetMatrix method instead
  M = std::make_unique<TMatrixD>(NX,NY);

  Int_t i(0), j(0);
  for (Int_t mopx = 0; mopx<nx; ++mopx) {
    for (Int_t x = 0; x<m0->GetNcols(); ++x) {
      for (Int_t mopy = 0; mopy<ny; ++mopy) {
	for (Int_t y = 0; y<m0->GetNrows(); ++y) {
	  const auto m = mop[mopx][mopy];
	  std::cout.width(2);
	  std::cout << (*m)[x][y] << " " << std::flush;
	  (*M)[j][i] = (*m)[x][y];
	  i++;
	} // y
      } // mopy
      i=0;
      j++;
      std::cout << std::endl;
    } // x
  } // mopx

  //  M->Print();
}

data_t Markov::run(const size_t n)
{
  // Run the Markov process n times

  const auto r = sdef['n'];
  auto sdefv = r->GetVector();
  const Int_t N = sdefv->GetNrows();
  std::cout << "size: " << N << std::endl;
  sdefv->ResizeTo(M->GetNrows());

  const auto sdefm =
    std::make_unique<TMatrixD>(1,sdefv->GetNrows());
  for (Int_t i=0; i<sdefv->GetNrows(); ++i)
    (*sdefm)[0][i] = (*sdefv)[i];

  std::cout << "SDEF matrix:" << std::endl;
  sdefm->Print();

  std::cout << "start multiplying" << std::endl;
  Double_t ref(0.0); // reflected backwards
  Double_t sum(0.0); // transmitted forward
  for (size_t i=0; i<n; ++i) {
    std::cout <<  i << " out of " << n << std::endl;
    (*sdefm) *= (*M);
    ref += (*sdefm)[0][0];
    (*sdefm)[0][0] = 0.0;
    sum += (*sdefm)[0][sdefm->GetNcols()-1];
    (*sdefm)[0][sdefm->GetNcols()-1] = 0.0;
    sdefm->Print();
    std::cout << "ref and sum: " << ref << " " << sum << std::endl;

  }
  std::cout << "end multiplying: " << ref << " " << sum << std::endl;

  sdefm->Print();


  sdefv->ResizeTo(N);
  for (Int_t i=0; i<N; ++i)
    (*sdefv)[i] = (*sdefm)[0][sdefm->GetNcols()-N+i];

  sdefv->Print();

  return sdef;
}
