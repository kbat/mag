void draw()
{
  TFile *f = new TFile("test1.root");
  TH2D *sdef = f->Get<TH2D>("sdef");
  TH2D *eTe = f->Get<TH2D>("eTe");

  TCanvas *c1 = new TCanvas;
  c1->Divide(2,2);

  c1->cd(1);
  sdef->Draw("col,text");

  c1->cd(2);
  eTe->Draw("col,text");
}
