void res()
{
  gStyle->SetOptStat(0);

  TFile *f = new TFile("res.root");
  TH2D *h = f->Get<TH2D>("h");
  h->SetDirectory(0);
  f->Close();

  //  f = new TFile("Poly/case753/50cm/mctal.root");
  f = new TFile("Poly/case753/mctal.root");
  THnSparseF *f1 = f->Get<THnSparseF>("f1");
  f1->Print("a");
  f1->GetAxis(0)->SetRange(2,2);
  f1->GetAxis(5)->SetRange(10, 18);

  TH1D *hGAM = dynamic_cast<TH1D*>(h->ProjectionX());
  hGAM->SetTitle("Transport matrices");
  hGAM->SetLineWidth(2);
  TH1D *hMC  = dynamic_cast<TH1D*>(f1->Projection(6));
  hMC->SetTitle("MonteCarlo");
  hMC->SetLineColor(kRed);
  hMC->SetLineWidth(2);

  THStack *hs = new THStack("hs", ";Energy [MeV]");
  TLegend *leg = new TLegend(0.12, 0.75, 0.3, 0.87);

  TCanvas *c1 = new TCanvas;
  c1->Divide(2,1);

  c1->cd(1);
  addl(leg, hs, hGAM);
  addl(leg, hs, hMC);

  hs->Draw("nostack,hist,e");
  leg->Draw();

  gPad->SetGridy();
  gPad->SetLogx();
  gPad->SetLogy();

  c1->cd(2);
  TH1D *hRatio = dynamic_cast<TH1D*>(hMC->Clone("hRatio"));
  hRatio->Divide(hGAM);
  hRatio->SetTitle("MC/GAM");
  hRatio->Draw("hist,e");
  gPad->SetLogx();

  c1->Print("res.pdf");
}
