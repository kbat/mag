void draw1()
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

void draw2()
{
  TFile *f = new TFile("test2.root");
  TH2D *sdef = f->Get<TH2D>("sdef");

  TH2D *nTn = f->Get<TH2D>("nTn");
  TH2D *nTe = f->Get<TH2D>("nTe");

  TH2D *eTn = f->Get<TH2D>("eTn");
  TH2D *eTe = f->Get<TH2D>("eTe");

  TCanvas *c1 = new TCanvas;
  c1->Divide(3,2);

  c1->cd(1);
  sdef->Draw("col,text");

  c1->cd(2);
  nTn->Draw("col,text");

  c1->cd(3);
  nTe->Draw("col,text");

  c1->cd(4);
  eTn->Draw("col,text");

  c1->cd(5);
  eTe->Draw("col,text");
}

void draw3()
{
  TFile *f = new TFile("test3.root");
  TH2D *sdef = f->Get<TH2D>("sdef");

  TH2D *nTn = f->Get<TH2D>("nTn");
  TH2D *nTe = f->Get<TH2D>("nTe");
  TH2D *nTp = f->Get<TH2D>("nTp");

  TH2D *eTn = f->Get<TH2D>("eTn");
  TH2D *eTe = f->Get<TH2D>("eTe");
  TH2D *eTp = f->Get<TH2D>("eTp");

  TH2D *pTn = f->Get<TH2D>("pTn");
  TH2D *pTe = f->Get<TH2D>("pTe");
  TH2D *pTp = f->Get<TH2D>("pTp");

  TCanvas *c1 = new TCanvas;
  c1->Divide(4,3);

  c1->cd(1);
  sdef->Draw("col,text");

  c1->cd(2);
  nTn->Draw("col,text");

  c1->cd(3);
  nTe->Draw("col,text");

  c1->cd(4);
  nTp->Draw("col,text");

  c1->cd(5);
  //  eTe->Draw("col,text");
}

void draw()
{
  gStyle->SetOptStat(0);
  //  gStyle->SetMarkerSize(5);
  //  draw1();
  // draw2();
  draw3();
}
