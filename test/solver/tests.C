#include <iostream>

const double epsilon = 0.01;

bool cmp(const char *title,
	 const char *fname, const char *hname, const size_t nlayers, const double v1, const double v2)
{
  TFile fres(fname);
  TH2D *h = fres.Get<TH2D>(hname);
  if ((std::abs(h->GetBinContent(1,1)-v1) < epsilon) &
      (std::abs(h->GetBinContent(1,2)-v2) < epsilon)) {
    fres.Close();
    std::cout << "OK: " << title << " with " << nlayers << " layers passed" << std::endl;
    return 0;
  } else
    std::cout << title << " with " << nlayers << " layers failed" << std::endl;

  h->Print("range");
  fres.Close();
  return 1;
}

int test1(const char *fname="test1.root")
{
  /*
    1x2 source transporting 1 particle type.
    Only transmission matrices tested.
   */
  const Int_t nx = 1;
  const Int_t ny = 2;

  TFile f(fname, "recreate");

  TH2D sdef("sdef", "sdef;Energy;#mu", nx, 0, nx, ny, 0, 1);
  sdef.SetBinContent(1,1,1);
  sdef.SetBinContent(1,2,3);

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
  cmp("test1", "../../res.root", "n", 1, 10, 14);

  // 2 layers
  system("cd ../../ && ./gam-solve test1 2");
  cmp("test1", "../../res.root", "n", 2, 52, 76);

  // 3 layers
  system("cd ../../ && ./gam-solve test1 3");
  cmp("test1", "../../res.root", "n", 3, 280, 408);

  return 0;
}

int test2(const char *fname="test1.root")
{
  /*
    1x2 source transporting 2 particle types: n and e
    Only transmission matrices tested.
   */
  const Int_t nx = 1;
  const Int_t ny = 2;

  TFile f(fname, "recreate");

  TH2D sdef("sdef", "sdef;Energy;#mu", nx, 0, nx, ny, 0, 1);
  sdef.SetBinContent(1,1,1);
  sdef.SetBinContent(1,2,3);

  const Int_t n = nx*ny;
  TH2D nTn("nTn", "nTn", n, 0, n, n, 0, n);
  TH2D nTe("nTe", "nTe", n, 0, n, n, 0, n);
  TH2D eTn("eTn", "eTn", n, 0, n, n, 0, n);
  TH2D eTe("eTe", "eTe", n, 0, n, n, 0, n);

  Int_t k=1;
  for (Int_t j=1; j<=n; ++j)
    for (Int_t i=1; i<=n; ++i) {
      nTn.SetBinContent(i,j,k);
      nTe.SetBinContent(i,j,k+1);
      eTn.SetBinContent(i,j,k+2);
      eTe.SetBinContent(i,j,k+3);
      k++;
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
  system("cd ../../ && ./gam-solve test2 1");
  cmp("test2n", "../../res.root", "n", 1, 10, 14);
  cmp("test2e", "../../res.root", "e", 1, 14, 18);

  // 2 layers
  system("cd ../../ && ./gam-solve test2 2");
  cmp("test2n", "../../res.root", "n", 2, 184, 240);
  cmp("test2e", "../../res.root", "e", 2, 240, 296);

  // 3 layers
  system("cd ../../ && ./gam-solve test2 3");
  cmp("test2n", "../../res.root", "n", 3, 3104, 4064);
  cmp("test2e", "../../res.root", "e", 3, 4064, 5024);

  return 0;
}


void tests()
{
  //test1("test1.root");
  test2("test2.root");
}
