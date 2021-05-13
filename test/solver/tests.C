#include <iostream>

const double epsilon = 0.01;

bool cmp2(const char *title,
	 const char *fname, const char *hname, const size_t nlayers, const double v1, const double v2)
{
  // 2 bins

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

bool cmp4(const char *title,
	 const char *fname, const char *hname, const size_t nlayers,
	  const double v1, const double v2,
	  const double v3, const double v4)
{
  // 4 bins
  TFile fres(fname);
  TH2D *h = fres.Get<TH2D>(hname);
  if ((std::abs(h->GetBinContent(1,1)-v1) < epsilon) &
      (std::abs(h->GetBinContent(2,1)-v2) < epsilon) &
      (std::abs(h->GetBinContent(1,2)-v3) < epsilon) &
      (std::abs(h->GetBinContent(2,2)-v4) < epsilon)) {
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
    Reflection matrix: see test4
   */
  const Int_t nx = 1;
  const Int_t ny = 2;

  TFile f(fname, "recreate");

  TH2D sdef("sdef", "sdef;Energy;#mu", nx, 0, nx, ny, 0, 1);
  sdef.SetBinContent(1,1,1);
  sdef.SetBinContent(1,2,3);

  const Int_t n = nx*ny;
  TH2D nTn("nTn", "nTn", n, 0, n, n, 0, n);
  TH2D nRn("nRn", "nRn", n, 0, n, n, 0, n);

  Int_t k=1;
  for (Int_t j=1; j<=n; ++j)
    for (Int_t i=1; i<=n; ++i) {
      nTn.SetBinContent(i,j,k);
      nRn.SetBinContent(i,j,k+10);
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
  system("cd ../../ && ./gam-solve test1 1");
  cmp2("test1", "../../res.root", "n", 1, 10, 14);

  // 2 layers
  system("cd ../../ && ./gam-solve test1 2");
  cmp2("test1", "../../res.root", "n", 2, 52, 76);

  // 3 layers
  system("cd ../../ && ./gam-solve test1 3");
  cmp2("test1", "../../res.root", "n", 3, 280, 408);

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
  cmp2("test2n", "../../res.root", "n", 1, 10, 14);
  cmp2("test2e", "../../res.root", "e", 1, 14, 18);

  // 2 layers
  system("cd ../../ && ./gam-solve test2 2");
  cmp2("test2n", "../../res.root", "n", 2, 184, 240);
  cmp2("test2e", "../../res.root", "e", 2, 240, 296);

  // 3 layers
  system("cd ../../ && ./gam-solve test2 3");
  cmp2("test2n", "../../res.root", "n", 3, 3104, 4064);
  cmp2("test2e", "../../res.root", "e", 3, 4064, 5024);

  return 0;
}

int test3(const char *fname="test1.root")
{
  /*
    2x2 source transporting 3 particle types: n, p and e
    Only transmission matrices tested.
   */
  const Int_t nx = 2;
  const Int_t ny = 2;

  TFile f(fname, "recreate");

  TH2D sdef("sdef", "sdef;Energy;#mu", nx, 0, nx, ny, 0, 1);
  sdef.SetBinContent(1,1,1);
  sdef.SetBinContent(1,2,3);
  sdef.SetBinContent(2,1,5);
  sdef.SetBinContent(2,2,7);

  const Int_t n = nx*ny;

  TH2D nTn("nTn", "nTn", n, 0, n, n, 0, n);
  TH2D nTe("nTe", "nTe", n, 0, n, n, 0, n);
  TH2D nTp("nTp", "nTp", n, 0, n, n, 0, n);

  TH2D eTn("eTn", "eTn", n, 0, n, n, 0, n);
  TH2D eTe("eTe", "eTe", n, 0, n, n, 0, n);
  TH2D eTp("eTp", "eTp", n, 0, n, n, 0, n);

  TH2D pTn("pTn", "pTn", n, 0, n, n, 0, n);
  TH2D pTe("pTe", "pTe", n, 0, n, n, 0, n);
  TH2D pTp("pTp", "pTp", n, 0, n, n, 0, n);



  Int_t k=1;
  for (Int_t j=1; j<=n; ++j)
    for (Int_t i=1; i<=n; ++i) {
      nTn.SetBinContent(i,j,k);
      nTe.SetBinContent(i,j,k+1);
      nTp.SetBinContent(i,j,k+2);

      eTn.SetBinContent(i,j,k+3);
      eTe.SetBinContent(i,j,k+4);
      eTp.SetBinContent(i,j,k+5);

      pTn.SetBinContent(i,j,k+6);
      pTe.SetBinContent(i,j,k+7);
      pTp.SetBinContent(i,j,k+8);

      k++;
    }

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
  system("cd ../../ && ./gam-solve test3 1");
  cmp4("test3n", "../../res.root", "n", 1, 144, 160, 176, 192);
  cmp4("test3e", "../../res.root", "e", 1, 160, 176, 192, 208);
  cmp4("test3p", "../../res.root", "p", 1, 176, 192, 208, 224);

  // 2 layers
  system("cd ../../ && ./gam-solve test3 2");
  cmp4("test3n", "../../res.root", "n", 2, 23424, 25632, 27840, 30048);
  cmp4("test3e", "../../res.root", "e", 2, 25632, 27840, 30048, 32256);
  cmp4("test3p", "../../res.root", "p", 2, 27840, 30048, 32256, 34464);

  // 3 layers
  system("cd ../../ && ./gam-solve test3 3");
  cmp4("test3n", "../../res.root", "n", 3, 3658752, 4006080, 4353408, 4700736);
  cmp4("test3e", "../../res.root", "e", 3, 4006080, 4353408, 4700736, 5048064);
  cmp4("test3p", "../../res.root", "p", 3, 4353408, 4700736, 5048064, 5395392);

  return 0;
}

int test4(const char *fname="test1.root")
{
  // test4 is test1 with reflection matrix

  // No sence to make this test with 1 layer, so start with 2:

  // 2 layers
  system("cd ../../ && ./gam-solve test4 2");
  cmp2("test4", "../../res.root", "n", 2, 31156, 46428);

  return 0;
}

void tests()
{
  // test1("test1.root");
  // test2("test2.root");
  // test3("test3.root");
  test4("test1.root");
}
