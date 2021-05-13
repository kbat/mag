void res()
{
  gStyle->SetOptStat(0);

  std::map<char, TH2D* > h2;
  std::map<char, TH1D* > h, hMC;
  const std::set<char> particles {'n', 'p', 'e', '|'};
  const std::map<char, std::string> tally{{'n', "f1"}, {'p', "f11"},
					  {'e', "f21"}, {'|', "f31"}};

  TFile *f = new TFile("res.root");
  for (auto p : particles) {
    h2[p] = f->Get<TH2D>(std::string(1, p).data());
    h[p] = dynamic_cast<TH1D*>(h2[p]->ProjectionX());
    h[p]->SetDirectory(0);
    h[p]->SetTitle("Transport matrices");
    h[p]->SetLineWidth(2);
  }
  f->Close();

  f = new TFile("Poly/case753/mctal.root");
  for (auto p : particles) {
    THnSparseF *f1 = f->Get<THnSparseF>(tally.at(p).data());
    f1->GetAxis(0)->SetRange(2,2);
    f1->GetAxis(5)->SetRange(10, 18);
    hMC[p] = dynamic_cast<TH1D*>(f1->Projection(6));
    hMC[p]->SetTitle("MonteCarlo");
    hMC[p]->SetLineColor(kRed);
    hMC[p]->SetLineWidth(2);
  }

  TCanvas *c1 = new TCanvas;
  c1->Divide(4,2);

  Int_t pad=1;
  for (auto p : particles) {
    THStack *hs = new THStack("hs", Form("%c;Energy [MeV]", p));
    TLegend *leg = new TLegend(0.12, 0.75, 0.6, 0.87);


    c1->cd(pad);
    addl(leg, hs, h[p]);
    addl(leg, hs, hMC[p]);

    hs->DrawClone("nostack,hist,e");
    hs->SetMinimum(1e-7);
    leg->DrawClone();

    gPad->SetGridy();
    gPad->SetLogx();
    gPad->SetLogy();

    c1->cd(pad+4);
    TH1D *hRatio = dynamic_cast<TH1D*>(hMC[p]->Clone("hRatio"));
    hRatio->Divide(h[p]);
    hRatio->SetTitle("MC/GAM;Energy [MeV]");
    hRatio->DrawClone("hist,e");
    gPad->SetLogx();
    pad++;
  }

  c1->Print("res.pdf");
}
