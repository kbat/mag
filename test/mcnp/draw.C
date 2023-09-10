std::pair<TH1*, TH1*> get(const std::string& fname1, const std::string& tally,
			  const std::string& fname2, const std::string& hname)
{
  auto hnmcnp = dynamic_cast<THnSparseF*>(GetObjectFromFile(fname1.data(), tally.data()));
  hnmcnp->GetAxis(0)->SetRange(2,2);
  auto hmcnp = hnmcnp->Projection(6);
  hmcnp->SetLineColor(kBlack);
  //hmcnp->Scale(1, "width");

  auto h2gam  = dynamic_cast<TH2D*>(GetObjectFromFile(fname2.data(), hname.data()));
  auto hgam = h2gam->ProjectionX();
  hgam->SetLineColor(kRed);

  return std::make_pair<TH1*, TH1*>(hmcnp,hgam);
}

Double_t getIntegral(const TH1* h)
{
  const Int_t N = h->GetNbinsX();
  Double_t sum = 0.0;
  for (Int_t i=1; i<=N; ++i) {
    const Double_t w = h->GetBinWidth(i);
    sum += w * h->GetBinContent(i);
  }
  return sum;
}

void draw(TCanvas *c1, const Int_t p1, const std::pair<TH1*, TH1*> data, const char *title)
{
  c1->cd(p1);
  THStack *hs = new THStack("hs", "");
  hs->Add(data.first);
  hs->Add(data.second);

  std::cout << "MCNP: " << getIntegral(data.first) << " GAM: " << getIntegral(data.second) << std::endl;

  hs->Draw("nostack hist e");
  // data.first->Draw("hist e");
  // data.second->Draw("hist e same");
  //  hs->GetXaxis()->SetRangeUser(1e-9, 1e3);
  gPad->SetLogy();
  gPad->SetLogx();
  gPad->SetGrid();

  c1->cd(p1+4);
  auto h = dynamic_cast<TH1D*>(data.first->Clone());
  auto a = h->Divide(data.second);
  if (a==0) {
    std::cout << "ERROR: Could not divide histograms" << std::endl;
    exit(0);
  }
  h->SetTitle("MCNP/GAM;Energy [MeV]");
  h->Draw("hist,e");
  gPad->SetLogx();
  gPad->SetGrid();

  std::array<TH1*,3> vec{data.first, data.second, h};

  const Int_t N = data.first->GetNbinsX();

  std::ofstream fout(Form("draw-%s.dat", title));
  for (Int_t i=1; i<=N; ++i) {
    fout << data.first->GetBinCenter(i) << " " << data.first->GetBinWidth(i)/2.0 << " ";
    for (const auto v : vec) {
      fout << v->GetBinContent(i) << " " << v->GetBinError(i) << " " << std::flush;
    }
    fout << std::endl;
  }
  fout.close();

}

void draw(const std::string fname="res.root")
{
  gStyle->SetOptStat(0);

  auto n = get("mctal.root", "f1", fname.data(),  "n");
  auto e = get("mctal.root", "f11", fname.data(), "p");
  auto p = get("mctal.root", "f21", fname.data(), "e");
  auto mu = get("mctal.root", "f31", fname.data(), "|");

  TCanvas *c1 = new TCanvas;
  c1->Divide(4,2);

  draw(c1, 1, n, "n");
  draw(c1, 2, e, "e");
  draw(c1, 3, p, "p");
  draw(c1, 4, mu, "mu");

  c1->Print("draw.pdf");
}
