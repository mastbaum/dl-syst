DL-LEE Systematics
==================
Tools for flux & cross section systematics studies with DL-LEE samples.

Building
--------
To build the tools, source `env.sh` on a uboone GPVM node, and run `make`.
This will populate the `lib` and `bin` directories.

Note: To rebuild, it's safest to `make clean && make`.

Workflow
--------

1. Generate weights from truth trees using the `TreeReader` LArSoft module,
   using the configurations provided in `fcl`
2. Extract those weights from the output art ROOT files into simple ROOT trees
   using the `arborist` utility.
3. Generate reco trees to match the truth trees (i.e. selected events in the
   same order) so that they can be merged, using `make_reco_tree`.
4. Make covariance matrices for each parameter, using `make_cov`.

The final step will output a ROOT file with a nue and numu energy spectrum
with an error band as well as covariance and correlation matrices for each
systematic parameter that was varied. Browse these files to get the plots, or
run

    $ budget DIRNAME

to print out the ranked fractional errors in the 350 MeV bin of the numu
Ereco distribution, as a measure of what uncertainties are dominant.

Quick Start
-----------
To run the chain end to end, source `env.sh`, then run:

    $ run_treereader_syst.sh

from a new directory in a `data` area. This will perform all of the workflow
steps in sequence.

Utilities
---------
Additional macros for plotting, etc. can be found in `tools`. See in-code
documentation for full details.

