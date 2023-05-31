std::pair<TH1*, TH1*> get(const std::string fname1, const std::string tally,
			  const std::string fname2, const std::string hname)
{
  auto hnmcnp = dynamic_cast<THnSparseF*>(GetObjectFromFile(fname1.data(), tally.data()));
  hnmcnp->GetAxis(0)->SetRange(2,2);
  auto hmcnp = hnmcnp->Projection(6);
  hmcnp->SetLineColor(kBlack);
  //hmcnp->Scale(1, "width");

  auto h2gam  = dynamic_cast<TH2D*>(GetObjectFromFile(fname2.data(), hname.data()));
  auto hgam = h2gam->ProjectionX();
  hgam->SetLineColor(kRed);
  hgam->Scale(0.5);

  return std::make_pair<TH1*, TH1*>(hmcnp,hgam);
}

void draw(TCanvas *c1, const Int_t p1, const std::pair<TH1*, TH1*> data)
{
  c1->cd(p1);
  THStack *hs = new THStack("hs", "");
  hs->Add(data.first);
  hs->Add(data.second);
  hs->Draw("nostack hist e");
  // data.first->Draw("hist e");
  // data.second->Draw("hist e same");
  //  hs->GetXaxis()->SetRangeUser(1e-9, 1e3);
  gPad->SetLogy();
  gPad->SetLogx();
  gPad->SetGrid();

  c1->cd(p1+4);
  auto h = dynamic_cast<TH1D*>(data.first->Clone());
  h->Divide(data.second);
  h->SetTitle("MCNP/GAM;Energy [MeV]");
  h->Draw("hist,e");
  gPad->SetLogx();
  gPad->SetGrid();
}

void draw(const std::string fname="res.root")
{
  gStyle->SetOptStat(0);

  auto n = get("mctal.root", "f1", fname.data(),  "n");
  auto e = get("mctal.root", "f11", fname.data(), "p");
  auto p = get("mctal.root", "f21", fname.data(), "e");
  //  auto mu = get("mctal.root", "f31", fname.data(), "|");

  TCanvas *c1 = new TCanvas;
  c1->Divide(4,2);

  draw(c1, 1, n);
  draw(c1, 2, e);
  draw(c1, 3, p);
  //  draw(c1, 4, mu);

  c1->Print("draw.pdf");

  std::array<std::pair<TH1*, TH1*>,3> vec{n,e,p};

  const Int_t N = n.first->GetNbinsX();

  std::ofstream fout("draw.dat");
  for (Int_t i=1; i<=N; ++i) {
    fout << n.first->GetBinCenter(i) << " " << n.first->GetBinWidth(i)/2.0 << " ";
    for (const auto v : vec) {
      fout << v.first->GetBinContent(i) << " " << v.first->GetBinError(i) << " " << std::flush;
      fout << v.second->GetBinContent(i) << " " << v.second->GetBinError(i) << " ";
    }
    fout << std::endl;
  }
  fout.close();
}
