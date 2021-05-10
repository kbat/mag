void draw(const char *fname="mctal.root")
{
  gStyle->SetOptStat(kFALSE);

  TFile *f = new TFile(fname);
  THnSparseF *f1 = f->Get<THnSparseF>("f1");
  f1->Print("a");

  TCanvas *c1 = new TCanvas;
  c1->Divide(1, 2);

  c1->cd(1); // Reflected
  f1->GetAxis(0)->SetRange(1,1);
  //  f1->GetAxis(5)->SetRangeUser(-1, 0);
  TH2D *R = f1->Projection(5,6);
  R->SetTitle("Reflected;Energy [MeV];#mu");
  R->SetMarkerSize(3.0);
  R->Draw("text,col");

  cout << R->GetXaxis()->GetBinLowEdge(2) << endl;

  c1->cd(2); // transmitted
  f1->GetAxis(0)->SetRange(2,2);
  //f1->GetAxis(5)->SetRangeUser(0, 1);
  TH2D *T = f1->Projection(5,6);
  T->SetTitle("Transmitted;Energy [MeV];#mu");
  T->SetMarkerSize(3.0);
  T->Draw("text,col");
}
