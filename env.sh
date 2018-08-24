source /cvmfs/uboone.opensciencegrid.org/products/setup_uboone.sh
#setup uboonecode v06_26_01_22 -q e10:prof
source /uboone/app/users/mastbaum/uboonecode-v06_26_01_12/localProducts_larsoft_v06_26_01_10_e10_prof/setup
mrbsetenv
setup gallery v1_03_13 -q e10:prof:nu

HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export LD_LIBRARY_PATH=$HERE/lib:$LD_LIBRARY_PATH
export PATH=$HERE/bin:$PATH
export FHICL_FILE_PATH=$HERE/fcl:$FHICL_FILE_PATH

