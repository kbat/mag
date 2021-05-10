#include <iostream>

const double epsilon = 0.01;

bool cmp(const char *fname, const size_t nbins, const double v1, const double v2)
{
  TFile fres(fname);
  TH2D *h = fres.Get<TH2D>("h");
  if ((std::abs(h->GetBinContent(1,1)-v1) < epsilon) &
      (std::abs(h->GetBinContent(1,2)-v2) < epsilon)) {
    fres.Close();
    std::cout << "OK: test1 with " << nbins << " bins passed" << std::endl;
    return 0;
  } else
    std::cout << "test1 with " << nbins << " bins failed" << std::endl;

  h->Print("range");
  fres.Close();
  return 1;
}

int test1(const char *fname="test1.root")
{
  /*
    1x2 source transporting 1 particle type only
   */
  const Int_t nx = 1;
  const Int_t ny = 2;

  TFile f(fname, "recreate");

  TH2D sdef("sdef", "sdef;Energy;#mu", nx, 0, nx, ny, 0, 1);
  sdef.SetBinContent(1,1,1);

  const Int_t n = nx*ny;
  TH2D nTn("nTn", "nTn", n, 0, n, n, 0, n);

  Int_t k=0;
  for (Int_t j=1; j<=n; ++j)
    for (Int_t i=1; i<=n; ++i) {
      nTn.SetBinContent(i,j,++k);
    }
  // nTn.SetBinContent(1,1,1);
  // nTn.SetBinContent(2,1,1);
  // nTn.SetBinContent(2,2,1);

  k=1;
  for (Int_t i=1; i<=nx; ++i)
    for (Int_t j=1; j<=ny; ++j) {
      nTn.GetXaxis()->SetBinLabel(k, Form("E_{%d}#mu_{%d}", j, i));
      nTn.GetYaxis()->SetBinLabel(k, Form("E_{%d}#mu_{%d}", j, i));
      k++;
    }

  f.Write();
  f.Close();

  // 1 layer
  system("cd ../../ && ./gam-solve test1 1");

  cmp("../../res.root", 1, 1, 2);

  // 2 layers
  system("cd ../../ && ./gam-solve test1 2");

  cmp("../../res.root", 2, 7, 10);

  return 1;
}


void tests()
{
  test1("test1.root");
}
