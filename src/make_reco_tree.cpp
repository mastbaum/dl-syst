/**
 * \file make_reco_tree.cpp
 * \brief Extract a tree with reco energy parallel to the weights tree
 * \author A. Mastbaum <mastbaum@uchicago.edu>
 *
 * The truth trees used to generate the weights only contain selected events,
 * while the tree with the reco energy covers all the events. This script
 * uses the run/subrun/event IDs to look up selected events in the reco
 * tree and build a new tree with only selected events, which will match
 * up with the truth and weight trees, and be friended up with them.
 *
 * Usage ...is a little unwieldy:
 *
 *     ./make_reco_tree    \
 *         OUTPUT.root     \  # The output ROOT file, with a tree called "reco"
 *         RECO_FILE.root  \  # The input file with the reco tree
 *         RECO_TREE_OBJ   \  # Name of the reco tree object (e.g. "numu_cocktail_tree")
 *         TRUTH_FILE.root \  # Input file with truth info
 *         WGH_FILE.root      # Input file with event weights
 *
 */

#include <cassert>
#include <iostream>
#include <map>
#include <utility>
#include <TTree.h>
#include <TFile.h>

int main(int argc, char* argv[]) {
  assert(argc == 6);

  // Output tree
  TFile fout(argv[1], "recreate");
  TTree* t = new TTree("reco", "");
  int eventID, runno, subrunno, eventno;
  double enu, ereco;
  t->Branch("runno", &runno);
  t->Branch("subrunno", &subrunno);
  t->Branch("eventno", &eventno);
  t->Branch("eventID", &eventID);
  t->Branch("Enu", &enu);
  t->Branch("Ereco", &ereco);
  eventID = 0;

  // Input reco tree
  TFile* fr = TFile::Open(argv[2]);
  assert(fr);
  TTree* tr = (TTree*) fr->Get(argv[3]);
  assert(tr);
  int runnoR, subrunnoR, eventnoR;
  double enuR, erecoR;
  tr->SetBranchAddress("run", &runnoR);
  tr->SetBranchAddress("subrun", &subrunnoR);
  tr->SetBranchAddress("event", &eventnoR);
  tr->SetBranchAddress("true_nu_energy", &enuR);
  tr->SetBranchAddress("reco_energy", &erecoR);

  std::map<int, std::map<int, std::map<int, std::pair<double, double> > > > rdata;

  for (long i=0; i<tr->GetEntries(); i++) {
    tr->GetEntry(i);

    if (rdata.find(runnoR) == rdata.end()) {
      rdata[runnoR];
    }
    if (rdata[runnoR].find(subrunnoR) == rdata[runnoR].end()) {
      rdata[runnoR][subrunnoR];
    }
    if (rdata[runnoR][subrunnoR].find(eventnoR) == rdata[runnoR][subrunnoR].end()) {
      rdata[runnoR][subrunnoR][eventnoR];
    }

    rdata.at(runnoR).at(subrunnoR).at(eventnoR) = std::make_pair(enuR, erecoR);
  }

  fr->Close();

  // Input truth tree
  TFile* ft = TFile::Open(argv[4]);
  assert(ft);
  TTree* tt = (TTree*) ft->Get("mc_tree");
  assert(tt);
  int runnoT, subrunnoT, eventnoT;
  tt->SetBranchAddress("run", &runnoT);
  tt->SetBranchAddress("subrun", &subrunnoT);
  tt->SetBranchAddress("event", &eventnoT);

  // Input weight tree (from TreeReader -> arborist)
  TFile* fw = TFile::Open(argv[5]);
  assert(fw);
  TTree* tw = (TTree*) fw->Get("events");
  assert(tw);
  double enuW;
  tw->SetBranchAddress("Enu", &enuW);

  assert(tw->GetEntries() == tt->GetEntries());

  // Event loop
  for (long i=0; i<tw->GetEntries(); i++) {
    tw->GetEntry(i);
    tt->GetEntry(i);

    runno = runnoT;
    subrunno = subrunnoT;
    eventno = eventnoT;
    eventID = i;
    enu = rdata[runno][subrunno][eventno].first;
    ereco = rdata[runno][subrunno][eventno].second;

    // Neutrino energy should match across trees
    if(!(abs(enu - enuW*1000) < 1e-2)) {
      std::cerr << "Enu mismatch for "
                << runno << "/" << subrunno << "/" << eventno << ": "
                << enu << " != " << enuW * 1000
                << std::endl;
    }

    t->Fill();
  }

  fout.cd();
  t->Write();

  return 0;
}

