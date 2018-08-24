#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>
#include <set>
#include <string>
#include <vector>
#include "TCanvas.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TPaveText.h"
#include "TTree.h"
#include "TStyle.h"
#include "Covariance.h"

/*****************************************************************************
 ** Covariance::EventSample implementation                                  **
 *****************************************************************************/

Covariance::EventSample::EventSample(std::string _name,
                                       size_t nbins,
                                       double elo, double ehi,
                                       size_t nweights)
    : name(_name), enu(nullptr), cov(nullptr) {
  enu = new TH1D(("enu_" + name).c_str(),
                 ";Energy [MeV];Entries per bin",
                 nbins, elo, ehi);
  enu->Sumw2();

  Resize(nweights);
}


Covariance::EventSample::~EventSample() {
  delete enu;
}


TGraphErrors* Covariance::EventSample::EnuErrors() {
  // Compute the covariance matrix first, if we haven't already
  if (!cov) {
    CovarianceMatrix();
  }

  // x/y values and (symmetric) errors
  const size_t nbins = enu->GetNbinsX();
  double xv[nbins];
  double xe[nbins];
  double yv[nbins];
  double ye[nbins];

  for (size_t i=0; i<nbins; i++) {
    xv[i] = enu->GetBinCenter(i+1);
    xe[i] = enu->GetBinWidth(i+1) / 2;
    yv[i] = enu->GetBinContent(i+1);
    ye[i] = sqrt(cov->GetBinContent(i+1, i+1));
  }

  return new TGraphErrors(nbins, xv, yv, xe, ye);
}


void Covariance::EventSample::Resize(size_t nweights) {
  enu_syst.clear();
  for (size_t i=0; i<nweights; i++) {
    std::string hname = Form("enu_%s_%zu", name.c_str(), i);
    TH1D* h = (TH1D*) enu->Clone(hname.c_str());
    h->SetDirectory(NULL);
    enu_syst.push_back(h);
  }
}


TH2D* Covariance::EventSample::CovarianceMatrix(
    TH1D* nom, std::vector<TH1D*> syst) {
  int nbins = nom->GetNbinsX();

  TH2D* _cov = new TH2D("cov", "", nbins, 0, nbins, nbins, 0, nbins);

  for (int i=1; i<nbins+1; i++) {
    for (int j=1; j<nbins+1; j++) {
      double vij = 0;
      for (size_t k=0; k<syst.size(); k++) {
        double vi = nom->GetBinContent(i) - syst[k]->GetBinContent(i);
        double vj = nom->GetBinContent(j) - syst[k]->GetBinContent(j);
        vij += vi * vj / syst.size();
      }
      _cov->SetBinContent(i, j, vij);
    }
  }

  return _cov;
}


TH2D* Covariance::EventSample::CovarianceMatrix() {
  delete cov;
  cov = CovarianceMatrix(enu, enu_syst);
  cov->SetName(("cov_" + name).c_str());
  return cov;
}



TH2D* Covariance::EventSample::CorrelationMatrix(TH2D* _cov) {
  TH2D* cor = (TH2D*) _cov->Clone("cor");

  for (int i=1; i<_cov->GetNbinsX()+1; i++) {
    for (int j=1; j<_cov->GetNbinsY()+1; j++) {
      double vij = _cov->GetBinContent(i, j);
      double si = sqrt(_cov->GetBinContent(i, i));
      double sj = sqrt(_cov->GetBinContent(j, j));
      cor->SetBinContent(i, j, vij / (si * sj));
    }
  }

  return cor;
}


TH2D* Covariance::EventSample::CorrelationMatrix() {
  // Compute the covariance matrix first, if we haven't already
  if (!cov) {
    CovarianceMatrix();
  }

  TH2D* cor = CorrelationMatrix(cov);
  cor->SetName(("cor_" + name).c_str());
  return cor;
}


/*****************************************************************************
 ** Covariance implementation                                               **
 *****************************************************************************/

Covariance::Covariance() {}


Covariance::~Covariance() {
  for (size_t i=0; i<samples.size(); i++) {
    delete samples[i];
  }
  samples.clear();
}


void Covariance::AddInputTree(TTree* _t, TFile* _f, float scale) {
  fInputTrees.push_back(_t);
  fInputFiles.push_back(_f);
  fTreeScales.push_back(scale);
}


void Covariance::AddWeight(std::string w) {
  use_weights.insert(w);
}


void Covariance::init() {
  assert(!fInputTrees.empty());

  // Define event samples
  samples.push_back(new EventSample("nue",  9,   0, 1800));
  samples.push_back(new EventSample("num", 10, 200, 1200));

  std::cout << "Covariance::init: Weights: ";
  for (auto it : use_weights) {
    std::cout << it << " ";
  }
  std::cout << std::endl;
}


void Covariance::analyze() {
  bool wgh_available = false;

  for (size_t ii=0; ii<fInputTrees.size(); ii++) {
    TTree* _tree = fInputTrees[ii];
    assert(_tree && _tree->GetEntries() > 0);
    //std::cout << "Entries " << _tree->GetEntries() << std::endl;
  
    // Input branches
    double ereco;
    std::map<std::string, std::vector<double> >* wgh = new std::map<std::string, std::vector<double> >;
    _tree->SetBranchAddress("weights", &wgh);
    _tree->SetBranchAddress("Ereco", &ereco);

    // Event loop
    for (long k=0; k<_tree->GetEntries(); k++) {
      _tree->GetEntry(k);
  
      // Iterate through all the weighting functions to compute a set of
      // total weights for this event. mcWeight is a mapping from reweighting
      // function name to a vector of weights for each "universe."
      double bnbwgh = 0;
      std::vector<double> weights;
      size_t wmin = 2000;

      for (auto const& it : *wgh) {
        if (it.first.find("bnbcorrection") != std::string::npos) {
          assert(it.second.size() == 1);
          bnbwgh = it.second[0];
        }
        if (use_weights.find(it.first) == use_weights.end()) { continue; }
        if (it.first.find("bnbcorrection") == std::string::npos && it.second.size() < wmin) {
          wmin = it.second.size();
        }

        assert(wmin < 1000000);
        weights.resize(wmin, 1.0);

  
        // Compute universe-wise product of all requsted weights
        if (use_weights.find("*") != use_weights.end() ||
            use_weights.find(it.first) != use_weights.end()) {
          for (size_t i=0; i<weights.size(); i++) {
            weights[i] *= it.second[i];
          }
        }
      }

      // Check that we actually have the requested weights
      wgh_available = !weights.empty();
      if (!wgh_available) {
        return;
      }

      // The observable
      double nuEnergy = ereco;
  
      EventSample* sample = samples[ii];
      double fs = fTreeScales[ii];  // Scale factor
  
      // Apply BNB weight
      assert(bnbwgh > 0);
      fs *= bnbwgh;
  
      // Fill histograms for this event sample
      if (sample->enu_syst.empty()) {
        sample->Resize(weights.size());
      }
      else {
        assert(sample->enu_syst.size() == weights.size());
      }
  
      // CV histogram
      sample->enu->Fill(nuEnergy, fs);
  
      // Systematics histograms with weights
      for (size_t i=0; i<weights.size(); i++) {
        sample->enu_syst[i]->Fill(nuEnergy, weights[i] * fs);
      }
    }
  }

  // Output
  fFile = TFile::Open(fOutputFile.c_str(), "recreate");
  assert(fFile);
  fFile->cd();
  std::cout << "Covariance::init: Writing output to " << fOutputFile << std::endl;

  // Write out sample=-wise distributions
  size_t total_bins = 0;
  for (size_t i=0; i<samples.size(); i++) {
    samples[i]->enu->Write();
    total_bins += samples[i]->enu->GetNbinsX();

    TH2D* cov = samples[i]->CovarianceMatrix();
    cov->Write();

    TH2D* cor = samples[i]->CorrelationMatrix();
    cor->Write();

    TGraphErrors* g = samples[i]->EnuErrors();
    g->SetName(("err_" + samples[i]->name).c_str());
    g->Write();
  }

  // Global (sample-to-sample) distributions
  // Build glued-together energy spectra for the nominal and each systematics
  // universe, and feed into the correlation matrix calculator.
  TH1D hg("hg", ";E_{#nu};Entries per bin", total_bins, 0, total_bins);
  hg.Sumw2();
  std::vector<TH1D*> hgsys;
  for (size_t i=0; i<samples[0]->enu_syst.size(); i++) {
    hgsys.push_back(new TH1D(Form("hg%zu", i), "", total_bins, 0, total_bins));
    hgsys[i]->Sumw2();
  }
  size_t ibin = 1;
  for (size_t i=0; i<samples.size(); i++) {
    for(int j=1; j<samples[i]->enu->GetNbinsX()+1; j++) {
      hg.SetBinContent(ibin, samples[i]->enu->GetBinContent(j));
      hg.SetBinError(ibin, samples[i]->enu->GetBinError(j));
      if (hgsys.size() != samples[i]->enu_syst.size()) {
        continue;
      }
      for (size_t k=0; k<hgsys.size(); k++) {
        hgsys[k]->SetBinContent(ibin, samples[i]->enu_syst[k]->GetBinContent(j));
      }
      ibin++;
    }
  }

  hg.Write();

  TH2D* gcov = EventSample::CovarianceMatrix(&hg, hgsys);
  gcov->Write();

  TH2D* gcor = EventSample::CorrelationMatrix(gcov);
  gcor->Write();

  fFile->Close();
}

