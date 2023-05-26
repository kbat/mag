#include <TMath.h>
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

  std::vector<std::vector<std::shared_ptr<TMatrixD>>> mop; // TODO: use the ROOT block matrix class

  const size_t N = layers.size();
  // An additional layers: N=1th - transmission forward from the last layer:
  const size_t nx = N+2;
  // An additonal layer to account for the reflection matrix
  const size_t ny = N+2;

  const auto empty = layers[0]->getEmpty();
  const auto unit = layers[0]->getUnit();
  std::shared_ptr<TMatrixD> m = nullptr; // TODO: is it better to declare it inside loops?
  std::string title; // TODO: is it better to declare it inside loops? see the ESC19 notes

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

  const Int_t nx = mop[0].size();
  const Int_t ny = mop.size();

  assert(nx == ny);

  std::cout << "createMatrix: block matrix shape: " << nx << " " << ny << std::endl;

  const auto m0 = mop[0][0];
  const Int_t NX = m0->GetNcols() * nx;
  const Int_t NY = m0->GetNrows() * ny;

  std::cout << "createMatrix: Markov process plain matrix shape: " << NX << " " << NY << std::endl;

  // TODO: use sparse matrix and its GetMatrix method instead
  M = std::make_unique<TMatrixDSparse>(NX,NY);

  Int_t i(0), j(0);
  std::vector<Int_t> rowindex, colindex;
  std::vector<Double_t> data;
  for (Int_t mopx = 0; mopx<nx; ++mopx) {
    for (Int_t x = 0; x<m0->GetNcols(); ++x) {
      for (Int_t mopy = 0; mopy<ny; ++mopy) {
	const auto m = mop[mopx][mopy];
  	for (Int_t y = 0; y<m0->GetNrows(); ++y) {
  	  // std::cout.width(2);
  	  // std::cout << (*m)[x][y] << " " << std::flush;
  	  const Double_t val = (*m)[x][y];
	  if (val>0.0) {
	    colindex.push_back(i);
	    rowindex.push_back(j);
	    data.push_back(val);
	  }
  	  i++;
  	} // y
      } // mopy
      i=0;
      j++;
      // std::cout << std::endl;
    } // x
  } // mopx

  M->SetMatrixArray(data.size(),rowindex.data(),colindex.data(),data.data());
  //  M->Print();
}

data_t Markov::run(const Double_t stop)
{
  // Run the Markov process
  // If stop>1 then run it round(stop) times, otherwise (if stop <= 1)
  // run until the ratio between the forward vector sum on the previous
  // step and the current step is below the stop fraction.
  // Negative stop values are not allowed.

  assert(stop>0.0);

  Int_t n = stop > 1.0 ? TMath::Nint(stop) : 0;

  const std::shared_ptr<Source> r = sdef['n']; //  TODO implement for all particles

  std::shared_ptr<TVectorD> sdefv = r->GetVector();
  const Int_t N = sdefv->GetNrows();
  // std::cout << "size: " << N << std::endl;
  sdefv->ResizeTo(M->GetNrows());

  TMatrixDSparse sdefm(1,sdefv->GetNrows()); //TODO: use sparse matrix also here
  std::vector<Int_t> rowindex;
  std::vector<Int_t> colindex;
  std::vector<Double_t> data;

  for (Int_t i=0; i<sdefv->GetNrows(); ++i) {
    const Double_t val = (*sdefv)[i];
    if (val>0.0) {
      rowindex.push_back(0);
      colindex.push_back(i);
      data.push_back(val);
      //      sdefm[0][i] = val;
    }
  }
  sdefm.SetMatrixArray(data.size(),rowindex.data(),colindex.data(),data.data());

  // std::cout << "SDEF matrix:" << std::endl;
  //  sdefm.Print();

  std::cout << "start multiplying" << std::endl;

  auto ref = std::make_unique<TVectorD>(N); // reflected backwards
  auto fwd = std::make_unique<TVectorD>(N); // transmitted forward

  Int_t i=0;
  Double_t sum=0.0; // forward sum at the current step
  Double_t sum1=0.0; // forward sum at the previous step
  for (;;) { // need to multiply n+1 times
    std::cout <<  i;    if (n>0) std::cout << " out of " << n;    std::cout << std::endl;

    sdefm *= (*M);

    for (Int_t j=0; j<N; ++j) {
      (*ref)[j] += sdefm[0][j];
      sdefm[0][j] = 0.0;
      (*fwd)[j] += sdefm[0][sdefm.GetNcols()-N+j];
      sdefm[0][sdefm.GetNcols()-N+j] = 0.0;
    }

    // std::cout << "ref and sum: " << std::endl;
    // ref->Print();
    // fwd->Print();
    sum = fwd->Sum();
    std::cout << "fwd sum: " << sum1 << " " << sum << "\t ref sum: " << ref->Sum() << std::endl;

    if ((n>0) && (i==n))
      break;
    else {
      if ((sum1>0.0) && (sum>0.0) && (TMath::Abs(1.0-sum1/sum)<stop))
	break;
    }
    sum1=sum;
    i++;
  }
  std::cout << "end multiplying: " << std::endl;

  // sdefm.Print();

  // set the sdef vector values to the forward spectrum, but the
  // former at first needs to be made compatible with the latter:
  r->GetVector()->ResizeTo(N);
  (*r->GetVector()) = (*fwd);

  //r->GetVector()->Print();

  return sdef;
}
