#include <set>
#include <string>
#include <vector>

class TFile;
class TGraphErrors;
class TH1D;
class TH2D;
class TTree;

/**
 * Covariance matrix calculator.
 */
class Covariance {
public:
  /** Constructor. */
  Covariance();

  /** Destructor. */
  virtual ~Covariance();

  /** Add a TTree for input, along with the file and a normalization. */
  void AddInputTree(TTree* _t, TFile* _f, float scale=1.0);

  /** Set the output file path. */
  void SetOutputFile(std::string _f) { fOutputFile = _f; }

  /** Add a weight to consider. */
  void AddWeight(std::string w);

  /** Set up the calculator. */
  void init();

  /** Run the calculator. */
  void analyze();

  /**
   * Container for an event sample, e.g. nue/numu/etc.
   */
  class EventSample {
    public:
      /**
       * Constructor.
       *
       * Note: You can optionally specify the number of systematics
       * universes up front with the nweights parameter. If this isn't
       * known until runtime, call Resize() later.
       *
       * \param _name String name for the event sample
       * \param nbins Number of energy bins
       * \param elo Lower limit of energy spectrum
       * \param ehi Upper limit of energy spectrum
       * \param nweights Number of systematics universes (see note)
       */
      EventSample(std::string _name="sample", size_t nbins=25,
                  double elo=0, double ehi=3000, size_t nweights=0);

      /** Destructor. */
      ~EventSample();

      /**
       * Get the energy spectrum as a graph with error bars representing
       * the systematic uncertainty.
       */
      TGraphErrors* EnuErrors();

      /** Set the number of universes. */
      void Resize(size_t nweights);

      /**
       * Covariance Matrix
       *
       *   i && j     = energy bins
       *   n          = number of weights
       *   N^cv_i     = number of events in bin i for the central value
       *   N^syst_i,m = number of events in bin i for the systematic
       *                variation in universe "m"
       *   E_ij       = the covariance (square of the uncertainty) for
       *                bins i,j
       *   E_ij = (1/n) Sum((N^cv_i - N^syst_i,m)*(N^cv_j - N^syst_j,m), m)
       */
      static TH2D* CovarianceMatrix(TH1D* nom, std::vector<TH1D*> syst);

      /** Covariance matrix using internal histograms */
      TH2D* CovarianceMatrix();

      /** Correlation matrix: Corr[ij] = Cov[ij]/Sqrt(Cov[ii]*Cov[jj]) */
      static TH2D* CorrelationMatrix(TH2D* _cov);

      /** Correlation matrix using internal histograms */
      TH2D* CorrelationMatrix();

      std::string name;  //!< String name for this event sample
      TH1D* enu;  //!< "Nominal" energy spectrum
      std::vector<TH1D*> enu_syst;  //!< Spectra for each systematic universe

    protected:
      TH2D* cov;  //!< Cached covariance matrix
  };

private:
  std::vector<TFile*> fInputFiles;  //!< Input files
  std::vector<TTree*> fInputTrees;  //!< Input files
  std::vector<float> fTreeScales;  //!< Input file weights
  std::string fOutputFile;  //!< Output file
  std::set<std::string> use_weights;  //!< Weight functions to use
  std::vector<EventSample*> samples;  //!< Event samples
  TFile* fFile;  //!< File for output
};

