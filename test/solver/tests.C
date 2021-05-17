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
    std::cerr << "OK: " << title << " with " << nlayers << " layers passed" << std::endl;
    return 0;
  } else
    std::cerr << title << " with " << nlayers << " layers failed" << std::endl;

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
  Int_t sum = 0;

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
  sum += cmp2("test1", "../../res.root", "n", 1, 10, 14);

  // 2 layers
  system("cd ../../ && ./gam-solve test1 2");
  sum += cmp2("test1", "../../res.root", "n", 2, 52, 76);

  // 3 layers
  system("cd ../../ && ./gam-solve test1 3");
  sum += cmp2("test1", "../../res.root", "n", 3, 280, 408);

  if (sum!=0)
    std::cerr << "Test 1 failed" << std::endl;

  return sum;
}

int test2(const char *fname="test2.root")
{
  /*
    1x2 source transporting 2 particle types: n and e
    Only transmission matrices tested.
    T and R matrices with 2 particles are tested in test5
   */
  Int_t sum = 0;

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
  sum += cmp2("test2n", "../../res.root", "n", 1, 10, 14);
  sum +=   cmp2("test2e", "../../res.root", "e", 1, 14, 18);

  // 2 layers
  system("cd ../../ && ./gam-solve test2 2");
  sum +=   cmp2("test2n", "../../res.root", "n", 2, 184, 240);
  sum +=   cmp2("test2e", "../../res.root", "e", 2, 240, 296);

  // 3 layers
  system("cd ../../ && ./gam-solve test2 3");
  sum +=   cmp2("test2n", "../../res.root", "n", 3, 3104, 4064);
  sum +=   cmp2("test2e", "../../res.root", "e", 3, 4064, 5024);

  if (sum!=0)
    std::cerr << "Test 2 failed" << std::endl;

  return sum;
}

int test3(const char *fname="test3.root")
{
  /*
    2x2 source transporting 3 particle types: n, p and e
    Only transmission matrices tested.
   */
  Int_t sum = 0;

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
  sum +=   cmp4("test3n", "../../res.root", "n", 1, 144, 160, 176, 192);
  sum +=   cmp4("test3e", "../../res.root", "e", 1, 160, 176, 192, 208);
  sum +=   cmp4("test3p", "../../res.root", "p", 1, 176, 192, 208, 224);

  // 2 layers
  system("cd ../../ && ./gam-solve test3 2");
  sum +=   cmp4("test3n", "../../res.root", "n", 2, 23424, 25632, 27840, 30048);
  sum +=   cmp4("test3e", "../../res.root", "e", 2, 25632, 27840, 30048, 32256);
  sum +=   cmp4("test3p", "../../res.root", "p", 2, 27840, 30048, 32256, 34464);

  // 3 layers
  system("cd ../../ && ./gam-solve test3 3");
  sum +=   cmp4("test3n", "../../res.root", "n", 3, 3658752, 4006080, 4353408, 4700736);
  sum +=   cmp4("test3e", "../../res.root", "e", 3, 4006080, 4353408, 4700736, 5048064);
  sum +=   cmp4("test3p", "../../res.root", "p", 3, 4353408, 4700736, 5048064, 5395392);

  if (sum!=0)
    std::cerr << "Test 3 failed" << std::endl;

  return sum;
}

int test4(const char *fname="test4.root")
{
  // test4 is test1 with reflection matrix

  Int_t sum = 0;

  // No sence to make this test with 1 layer, so start with 2:

  // 2 layers
  system("cd ../../ && ./gam-solve test4 2");
  sum += cmp2("test4", "../../res.root", "n", 2, 31156, 46428);

  // 3 layers
  system("cd ../../ && ./gam-solve test4 3");
  sum += cmp2("test4", "../../res.root", "n", 3, 100958120, 150444472);

  // 4 layers
  system("cd ../../ && ./gam-solve test4 4");
  sum += cmp2("test4", "../../res.root", "n", 4, 327143814608, 487498959984);

  if (sum!=0)
    std::cerr << "Test 4 failed" << std::endl;

  return sum;
}

int test5(const char *fname="test5.root")
{
  /*
    1x2 source transporting 2 particle types: n and e
    Both transmission and reflection matrices tested.
   */
  Int_t sum = 0;
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
  TH2D nRn("nRn", "nRn", n, 0, n, n, 0, n);
  TH2D nRe("nRe", "nRe", n, 0, n, n, 0, n);
  TH2D eRn("eRn", "eRn", n, 0, n, n, 0, n);
  TH2D eRe("eRe", "eRe", n, 0, n, n, 0, n);

  Int_t k=1;
  for (Int_t j=1; j<=n; ++j)
    for (Int_t i=1; i<=n; ++i) {
      nTn.SetBinContent(i,j,k);
      nTe.SetBinContent(i,j,k+1);
      eTn.SetBinContent(i,j,k+2);
      eTe.SetBinContent(i,j,k+3);
      nRn.SetBinContent(i,j,k/10.0);
      nRe.SetBinContent(i,j,k/10.0+1);
      eRn.SetBinContent(i,j,k/10.0+2);
      eRe.SetBinContent(i,j,k/10.0+3);
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
  system("cd ../../ && ./gam-solve test5 1");
  sum += cmp2("test5n", "../../res.root", "n", 1, 10, 14);
  sum += cmp2("test5e", "../../res.root", "e", 1, 14, 18);

  // 2 layers
  system("cd ../../ && ./gam-solve test5 2");
  sum += cmp2("test5n", "../../res.root", "n", 2, 11452.48, 14951.36);
  sum += cmp2("test5e", "../../res.root", "e", 2, 14951.36, 18450.24);

  // 3 layers
  system("cd ../../ && ./gam-solve test5 3");
  sum += cmp2("test5n", "../../res.root", "n", 3, 1.20698305024e+7, 1.57588614656e+7);
  sum += cmp2("test5e", "../../res.root", "e", 3, 1.57588614656e+7, 1.94478924288e+7);

  if (sum!=0)
    std::cerr << "Test 5 failed" << std::endl;

  return sum;
}

int tests()
{
  Int_t sum = 0;
  sum += test1("test1.root");
  sum += test2("test2.root");
  sum += test3("test3.root");
  sum += test4("test1.root");
  sum += test5("test5.root");

  if (sum == 0)
    std::cout << "All tests passed" << std::endl;

  return sum;
}
