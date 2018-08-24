/**
 * Plot the selected nue/numu phase space overlap.
 *
 * Usage:
 *
 *   root -l -x 'ps.C("NUE_TRUTH_FILE.root", "NUMU_TRUTH_FILE.root")'
 */

TH2F* plot(TTree* t) {
  int npart;
  int pdg[50];
  double epart[50], px[50], py[50], pz[50];
  double enu, nupx, nupy, nupz;
  t->SetBranchAddress("MCTruth_NParticles", &npart);
  t->SetBranchAddress("MCTruth_particles_PdgCode", &pdg);
  t->SetBranchAddress("MCTruth_particles_e0", &epart);
  t->SetBranchAddress("MCTruth_particles_px0", &px);
  t->SetBranchAddress("MCTruth_particles_py0", &py);
  t->SetBranchAddress("MCTruth_particles_pz0", &pz);
  t->SetBranchAddress("MCFlux_NuMomE", &enu);
  t->SetBranchAddress("MCFlux_NuMomX", &nupx);
  t->SetBranchAddress("MCFlux_NuMomY", &nupy);
  t->SetBranchAddress("MCFlux_NuMomZ", &nupz);

  TH2F* h = new TH2F("h", ";Three-momentum transfer q^{3} (GeV);Energy transfer q^{0} (GeV)", 100, 0, 1.25, 100, 0, 1.25);

  for (int i=0; i<t->GetEntries(); i++) {
    t->GetEntry(i);

    float elep = -999;
    TVector3 plep;

    for (int j=0; j<npart; j++) {
      std::cout << pdg[j] << " ";
      int p = abs(pdg[j]);
      if (p == 11 || p == 13 || p ==15) {
        elep = epart[j] - (p == 11 ? 511e-6 : 0.106);
        plep = TVector3(px[j], py[j], pz[j]);
        break;
      }
    }

    double q0 = enu - elep;
    double q3 = (TVector3(nupx, nupy, nupz) - plep).Mag();
    std::cout << " // " << q0 << " " << q3;
    h->Fill(q3, q0);

    std::cout << std::endl;
  }

  h->SetMarkerStyle(7);
  h->SetDirectory(0);
  return h;
}


TPaveText* genie_label() {
  TPaveText* l = new TPaveText(0.17, 0.6, 0.62, 0.88, "ndc");
  l->SetTextFont(132);
  l->SetTextAlign(12);
  l->SetBorderSize(0);
  l->SetFillColor(0);
  l->SetFillStyle(0);
  l->AddText("DL 1l1p selection");
  return l;
}

void ps(TString num_truth, TString nue_truth) {
  TFile* fm = TFile::Open(num_truth);
  TTree* tmu = (TTree*) fm->Get("mc_tree");
  TH2F* hm = plot(tmu);

  TFile* fe = TFile::Open(nue_truth);
  TTree* te = (TTree*) fe->Get("mc_tree");
  TH2F* he = plot(te);

  TCanvas* c1 = new TCanvas();
  hm->SetMarkerColor(kBlue);
  hm->Draw();
  he->SetMarkerColor(kRed);
  he->Draw("same");
  gStyle->SetOptStat(0);

  // Labels
  l1 = genie_label();
  l1->AddText("Lines W = 938, 1232, 1535 MeV");
  l1->AddText("Lines Q^{2} = 0.2 - 1.0 GeV");
  l1->Draw();

  // Q2 and W lines
  float lw = 1;
  std::vector<TF1*> qfs, wfs;
  std::vector<float> qs = {200, 400, 600, 800, 1000};
  for (size_t j=0; j<qs.size(); j++) {
    char fname[150];
    snprintf(fname, 150, "fq_%f", qs[j]);
    char ff[350];
    snprintf(ff, 350, "sqrt(TMath::Max(x**2-%f, 0.0))", qs[j]/1000);
    TF1* f = new TF1(fname, ff, qs[j]/1000, 1.3);
    f->SetNpx(1000);
    f->SetLineColor(kBlack);
    f->SetLineWidth(lw);
    f->Draw("same");
    qfs.push_back(f);
  }

  std::vector<float> ws = {0.938, 1.232, 1.535};
  for (size_t j=0; j<ws.size(); j++) {
    char fname[150];
    snprintf(fname, 150, "fw_%f", qs[j]);
    char ff[350];
    snprintf(ff, 350, "sqrt(%f**2 + x**2) - 0.938", ws[j]);
    TF1* f = new TF1(fname, ff, 0, 1.3);
    f->SetNpx(1000);
    f->SetLineColor(kBlack);
    f->SetLineWidth(lw);
    f->Draw("same");
    qfs.push_back(f);
  }

  c1->Update();
}

