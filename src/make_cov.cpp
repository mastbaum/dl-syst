/**
 * \file solo.cxx
 * \brief Loop through a set of weights, get covariance for each
 * \author A. Mastbaum <mastbaum@uchicago.edu>
 */

#include <cassert>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
#include "TFile.h"
#include "TTree.h"
#include "Covariance.h"

int main(int argc, char* argv[]) {
  assert(argc == 6);

  const char* out_dir = argv[1];
  const char* nue_reco = argv[2];
  const char* nue_wghs = argv[3];
  const char* num_reco = argv[4];
  const char* num_wghs = argv[5];

  // nue intrinsic
  TFile* fre = TFile::Open(nue_reco);
  assert(fre->IsOpen());
  TTree* tre = (TTree*) fre->Get("reco");
  assert(tre && tre->GetEntries() > 0);
  TFriendElement* fee = tre->AddFriend("events", nue_wghs);
  assert(fee);

  // numu cocktail
  TFile* frm = TFile::Open(num_reco);
  assert(frm->IsOpen());
  TTree* trm = (TTree*) frm->Get("reco");
  assert(trm && trm->GetEntries() > 0);
  TFriendElement* fem = trm->AddFriend("events", num_wghs);
  assert(fem);

  std::vector<std::string> fcns = {
    // Flux
    "kminus_PrimaryHadronNormalization",
    "kplus_PrimaryHadronFeynmanScaling",
    "kzero_PrimaryHadronSanfordWang",
    "nucleoninexsec_FluxUnisim",
    "nucleonqexsec_FluxUnisim",
    "nucleontotxsec_FluxUnisim",
    "piminus_PrimaryHadronSWCentralSplineVariation",
    "pioninexsec_FluxUnisim",
    "pionqexsec_FluxUnisim",
    "piontotxsec_FluxUnisim",
    "piplus_PrimaryHadronSWCentralSplineVariation",
    "expskin_FluxUnisim",
    "horncurrent_FluxUnisim",

    // GENIE
    "genie_qema_Genie",
    "genie_ccresAxial_Genie",
    "genie_ccresVector_Genie",
    "genie_ncelAxial_Genie",
    "genie_ncresAxial_Genie",
    "genie_FermiGasModelKf_Genie",
    "genie_NC_Genie",
    "genie_ResDecayTheta_Genie",
    "genie_IntraNukeNabs_Genie",
    "genie_IntraNukeNcex_Genie",
    "genie_IntraNukeNel_Genie",
    "genie_IntraNukeNinel_Genie",
    "genie_IntraNukeNmfp_Genie",
    "genie_IntraNukeNpi_Genie",
    "genie_IntraNukePIabs_Genie",
    "genie_IntraNukePIcex_Genie",
    "genie_IntraNukePIel_Genie",
    "genie_IntraNukePIinel_Genie",
    "genie_IntraNukePImfp_Genie",
    "genie_IntraNukePIpi_Genie",

    "genie_AGKYpT_Genie",
    "genie_AGKYxF_Genie",
    "genie_DISAth_Genie",
    "genie_DISBth_Genie",
    "genie_DISCv1u_Genie",
    "genie_DISCv2u_Genie",
    "genie_FermiGasModelSf_Genie",
    "genie_FormZone_Genie",
    "genie_NonResRvbarp1pi_Genie",
    "genie_NonResRvbarp2pi_Genie",
    "genie_NonResRvp1pi_Genie",
    "genie_NonResRvp2pi_Genie",
    "genie_ResDecayEta_Genie",
    "genie_ResDecayGamma_Genie",
    "genie_cohMA_Genie",
    "genie_cohR0_Genie",
    "genie_ncelEta_Genie",
    "genie_ncresVector_Genie",
    "genie_qevec_Genie",
    "model_q0q3_ccqe_HistogramWeight",
    "model_q0q3_ccmec_HistogramWeight"
    "genie_some_Genie"
  };

  for (size_t i=0; i<fcns.size(); i++) {
    Covariance cov;

    cov.AddInputTree(tre, fre, 0.011);
    cov.AddInputTree(trm, frm, 0.21);
    cov.AddWeight(fcns[i]);
    cov.SetOutputFile(std::string(out_dir) + "/cov_" + fcns[i] + ".root");

    cov.init();
    cov.analyze();
  }

  return 0;
}

