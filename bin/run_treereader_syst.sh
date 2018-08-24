#!/bin/bash

# Run the whole chain, from truth + reco trees to fractional systematic
# uncertainties on the reco distribution.

DATA_BASE="/uboone/data/users/yatesla/othersys/for_andy"

# Generate weights from truth trees using the TreeReader LArSoft module
lar -c run_treereader_some.fcl -o num_cocktail_wgh_some_art.root ${DATA_BASE}/mcc8v4_cocktail_test13_dllee_analysis_filtered.root
lar -c run_treereader_all.fcl -o num_cocktail_wgh_all_art.root ${DATA_BASE}/mcc8v4_cocktail_test13_dllee_analysis_filtered.root
lar -c run_treereader_some.fcl -o nue_intrinsic_wgh_some_art.root ${DATA_BASE}/mcc8v6_nuecosmic_truth_test13_dllee_analysis_filtered.root
lar -c run_treereader_all.fcl -o nue_intrinsic_wgh_all_art.root ${DATA_BASE}/mcc8v6_nuecosmic_truth_test13_dllee_analysis_filtered.root

# Extract weights from artroot files into nice simple trees
arborist num_cocktail_wgh_some.root num_cocktail_wgh_some_art.root
arborist num_cocktail_wgh_all.root num_cocktail_wgh_all_art.root
arborist nue_intrinsic_wgh_some.root nue_intrinsic_wgh_some_art.root
arborist nue_intrinsic_wgh_all.root nue_intrinsic_wgh_all_art.root

# Generate reco trees to match weight trees, so we can friend reco + weights
RECO_FILE=${DATA_BASE}/input_to_sbnfit.root
make_reco_tree \
  num_cocktail_reco.root \
  ${RECO_FILE} numu_cocktail_tree \
  ${DATA_BASE}/mcc8v4_cocktail_test13_dllee_analysis_filtered.root \
  ./num_cocktail_wgh_some.root

make_reco_tree \
  nue_intrinsic_reco.root \
  ${RECO_FILE} nue_intrinsic_tree \
  ${DATA_BASE}/mcc8v6_nuecosmic_truth_test13_dllee_analysis_filtered.root \
  ./nue_intrinsic_wgh_some.root

# Make covariance matrices
mkdir -p figures_all
make_cov figures_all \
  nue_intrinsic_reco.root nue_intrinsic_wgh_all.root \
  num_cocktail_reco.root num_cocktail_wgh_all.root 

mkdir -p figures_some
make_cov figures_some \
  nue_intrinsic_reco.root nue_intrinsic_wgh_some.root \
  num_cocktail_reco.root num_cocktail_wgh_some.root 

# Print the error budgets
budget figures_some

