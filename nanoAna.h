/////////////////////////////////////////////////////////////////////////////////
//
// nanoAna: Dynamic NanoAOD Analyzer
//
// This class extends standard TSelector functionality to run seamlessly across 
// all CMS NanoAOD versions without requiring manual branch type modifications.
// 
// Core Features:
// - Standard TTreeReaderArray and TTreeReaderValue have been replaced with 
//   DynamicArrayReader and DynamicValueReader.
// - Automatically handles on-the-fly type casting for branches whose underlying 
//   ROOT data types change between NanoAOD campaigns.
// - Implements strict (crash-on-missing) and fallback (safe default) modes to 
//   robustly handle variables that are renamed or removed in newer datasets.
//
// Author: Prachurjya Hazarika (prachurjya.hazarika@cern.ch)
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef nanoAna_h
#define nanoAna_h

// Default headers
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>
#include <vector>

//Standard headers:
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>

// Headers needed by this particular selector
#include <vector>
#include <TH1.h>
#include <TH2.h>
#include <TMath.h>
#include "TLorentzVector.h"
#include "TString.h"
#include <bitset>

// Headers specific to ONNX
#include "onnxruntime-linux-x64-1.24.4/include/onnxruntime_cxx_api.h"
#include <TVector3.h>
#include <TVector2.h>

//////////////////////////////////////////////////////////////////
//                                                              //
//            DO NOT CHANGE THE FOLLOWING TEMPLATES             //
//                                                              //
//////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------
// DynamicArrayReader <OutType>
//
// This is a wrapper around TTreeReaderArray that enables dynamic reading of 
// NanoAOD collection branches across different CMS data formats and versions.
//
// In different NanoAOD versions, the branch types (as well as names) can change.
// This wrapper allows dynamic type-sasting to prevent strict-type crashes in
// TTreeReader, and alow allows to scan for possible alternative branch names
// before assigning the values to the array reader.
//
// If initialized with a default value, failure to find the branch results in
// falling back to the default value with a one-time warning. If no default value
// is provided during initialization, it throws a run-time error when that
// variable is accessed in the Process() function.
//-------------------------------------------------------------------------------

template <typename OutType>
class DynamicArrayReader {
public: 
  // ROOT's ACLiC needs these structs to be public to generate the dictionary
  struct Base {
    virtual ~Base() = default;
    virtual OutType At(size_t i) = 0;
    virtual size_t GetSize() = 0;
  };

  // Template implementation to handle casting from NanoAOD type (InType) to User type (OutType)
  template <typename InType>
  struct Impl : public Base {
    TTreeReaderArray<InType>* array;
    Impl(TTreeReader& reader, const char* name) {
      array = new TTreeReaderArray<InType>(reader, name);
    }
    ~Impl() override { delete array; }
    OutType At(size_t i) override { return static_cast<OutType>(array->At(i)); }
    size_t GetSize() override { return array->GetSize(); }
  };

private:
  Base* m_reader = nullptr;
  bool m_isValid = false;
  bool m_hasDefault = false;
  OutType m_defaultValue;
  std::string m_requestedBranch; // Keeps track of the branch name for error printing

public:
  DynamicArrayReader() = default;
  ~DynamicArrayReader() { delete m_reader; }

  //----------------------------------------------------------------------------
  // STRICT MODE: No default value provided. Will crash if branch is missing.
  //----------------------------------------------------------------------------
  void Init(TTree* tree, TTreeReader& reader, const std::vector<const char*>& possibleNames) {
    if (m_reader) { delete m_reader; m_reader = nullptr; }
    m_isValid = false;
    m_hasDefault = false; 
    
    // Save the primary requested name for potential error messages
    if (possibleNames.size() > 0) m_requestedBranch = possibleNames[0]; 
    if (!tree) return;

    for (const char* branchName : possibleNames) {
      TLeaf* leaf = tree->GetLeaf(branchName);
      if (!leaf) continue;

      TString type = leaf->GetTypeName();
      // TTreeReader requires exact type matching. Dynamically match NanoAOD types.
      if (type == "UChar_t" || type == "Byte_t") m_reader = new Impl<UChar_t>(reader, branchName);
      else if (type == "Char_t")   m_reader = new Impl<Char_t>(reader, branchName);
      else if (type == "Int_t")    m_reader = new Impl<Int_t>(reader, branchName);
      else if (type == "Short_t")  m_reader = new Impl<Short_t>(reader, branchName);
      else if (type == "UShort_t") m_reader = new Impl<UShort_t>(reader, branchName);
      else if (type == "UInt_t")   m_reader = new Impl<UInt_t>(reader, branchName);
      else if (type == "Float_t")  m_reader = new Impl<Float_t>(reader, branchName);
      else if (type == "Double_t") m_reader = new Impl<Double_t>(reader, branchName);
      else if (type == "Bool_t")   m_reader = new Impl<Bool_t>(reader, branchName);
      else continue;

      m_isValid = true;
      return;
    }
  }

  void Init(TTree* tree, TTreeReader& reader, const char* branchName) {
    Init(tree, reader, std::vector<const char*>{branchName});
  }

  //----------------------------------------------------------------------------
  // FALLBACK MODE: Default value provided. Safe to use if branch is missing.
  //----------------------------------------------------------------------------
  void Init(TTree* tree, TTreeReader& reader, const std::vector<const char*>& possibleNames, OutType def) {
    Init(tree, reader, possibleNames); // First setup in strict mode
    m_hasDefault = true;               // Then override with fallback flags
    m_defaultValue = def;
    
    // Print a one-time dim warning if the branch was not found in the tree
    if (!m_isValid && tree != nullptr) {
      std::cout << "\033[2m[WARNING] branch: " << m_requestedBranch 
                << " not found, using default value: " << m_defaultValue << "\033[0m\n";
    }
  }

  void Init(TTree* tree, TTreeReader& reader, const char* branchName, OutType def) {
    Init(tree, reader, std::vector<const char*>{branchName}, def);
  }

  //--------
  // Debug
  //--------
  bool IsValid() const { return m_isValid; }
  size_t GetSize() { return m_isValid ? m_reader->GetSize() : 0; }
  
  // Safe bracket operator with colored error messaging
  OutType operator[](size_t i) { 
    if (m_isValid) return m_reader->At(i);
    if (m_hasDefault) return m_defaultValue;
    
    // Triggers if the branch is missing AND it was initialized in strict mode
    std::string err = "\n\n\033[31m[FATAL ERROR]\033[0m \033[33mDynamicArrayReader\033[0m: "
                      "Missing branch \033[31m" + m_requestedBranch + "\033[0m without a default fallback!\n";
    throw std::runtime_error(err);
  }
};

//-------------------------------------------------------------------------------
// DynamicValueReader <T>
//
// This is a wrapper around TTreeReaderValue for reading single, event-level 
// variables (e.g., MET, event weights, flags) across different NanoAOD versions.
//
// Similar to the previous wrapper, it accepts a list of possible branch names,
// binding to the first one found in the TTree. In case none of the names match,
// it falls back to any default value provided, or thorows a run-time error.
//-------------------------------------------------------------------------------

template <typename T>
class DynamicValueReader {
private:
  TTreeReaderValue<T>* m_reader = nullptr;
  T m_defaultValue;
  bool m_isValid = false;
  bool m_hasDefault = false;
  std::string m_requestedBranch; // Keeps track of the branch name for error printing

public:
  DynamicValueReader() = default;
  ~DynamicValueReader() { delete m_reader; }

  //----------------------------------------------------------------------------
  // STRICT MODE: No default value provided. Will crash if branch is missing.
  //----------------------------------------------------------------------------
  void Init(TTree* tree, TTreeReader& reader, const std::vector<const char*>& possibleNames) {
    if (m_reader) { delete m_reader; m_reader = nullptr; }
    m_isValid = false;
    m_hasDefault = false;
    
    // Save the primary requested name for potential error messages
    if (possibleNames.size() > 0) m_requestedBranch = possibleNames[0];
    if (!tree) return;

    for (const char* name : possibleNames) {
      if (tree->GetBranch(name)) {
        m_reader = new TTreeReaderValue<T>(reader, name);
        m_isValid = true;
        return;
      }
    }
  }
    
  void Init(TTree* tree, TTreeReader& reader, const char* branchName) {
    Init(tree, reader, std::vector<const char*>{branchName});
  }

  //----------------------------------------------------------------------------
  // FALLBACK MODE: Default value provided. Safe to use if branch is missing.
  //----------------------------------------------------------------------------
  void Init(TTree* tree, TTreeReader& reader, const std::vector<const char*>& possibleNames, T def) {
    Init(tree, reader, possibleNames); // First setup in strict mode
    m_hasDefault = true;               // Then override with fallback flags
    m_defaultValue = def;
    
    // Print a one-time dim warning if the branch was not found in the tree
    if (!m_isValid && tree != nullptr) {
      std::cout << "\033[2m[WARNING] branch: " << m_requestedBranch 
                << " not found, using default value: " << m_defaultValue << "\033[0m\n";
    }
  }
    
  void Init(TTree* tree, TTreeReader& reader, const char* branchName, T def) {
    Init(tree, reader, std::vector<const char*>{branchName}, def);
  }

  //--------
  // Debug
  //--------
  bool IsValid() const { return m_isValid; }
  
  // Dereference operator with colored error messaging
  T operator*() { 
    if (m_isValid) return **m_reader;
    if (m_hasDefault) return m_defaultValue;

    // Triggers if the branch is missing AND it was initialized in strict mode
    std::string err = "\n\n\033[31m[FATAL ERROR]\033[0m \033[33mDynamicValueReader\033[0m: "
                      "Missing branch \033[31m" + m_requestedBranch + "\033[0m without a default fallback!\n";
    throw std::runtime_error(err);
  }
};

//////////////////////////////////////////////////////////////////
//                                                              //
//              DEFINITION OF THE ANALYZER CLASS                //
//                                                              //
//////////////////////////////////////////////////////////////////

class nanoAna : public TSelector {
public :
  TTreeReader     fReader;  
  TTree          *fChain = 0;

  // The TTreeReader is used only for branches whose names and types are expected
  // to remain stable across current and future NanoAOD versions.
  // Other branches are handled using DynamicValueReader and DynamicArrayReader,
  // which allow the branch type to be determined and converted at runtime.
  // These dynamic readers are initialized in Init(), defined in Init.h.

  // Standard event information with stable branch names and types
  TTreeReaderValue<UInt_t> run = {fReader, "run"};
  TTreeReaderValue<UInt_t> luminosityBlock = {fReader, "luminosityBlock"};
  TTreeReaderValue<ULong64_t> event = {fReader, "event"};

  // Additional branches handled dynamically
  // The variable names and types below are user-defined and can be customized.
  // The corresponding NanoAOD branch names are specified and initialized in Init.h.
  DynamicValueReader<float> fixedGridRhoFastjetAll;
  DynamicValueReader<float> LHEWeight_originalXWGTUP;

  // Electrons
  DynamicValueReader<int> nElectron;
  DynamicArrayReader<bool> Electron_convVeto;
  DynamicArrayReader<int> Electron_cutBased;
  DynamicArrayReader<bool> Electron_cutBased_HEEP;
  DynamicArrayReader<bool> Electron_mvaIso_WP80;
  DynamicArrayReader<bool> Electron_mvaIso_WP90;
  DynamicArrayReader<bool> Electron_mvaIso_WPHZZ;
  DynamicArrayReader<bool> Electron_mvaNoIso_WP80;
  DynamicArrayReader<bool> Electron_mvaNoIso_WP90;
  DynamicArrayReader<int> Electron_seedGain;
  DynamicArrayReader<int> Electron_tightCharge;
  DynamicArrayReader<int> Electron_jetIdx;
  DynamicArrayReader<int> Electron_photonIdx;
  DynamicArrayReader<int> Electron_charge;
  DynamicArrayReader<int> Electron_pdgId;
  DynamicArrayReader<float> Electron_deltaEtaSC;
  DynamicArrayReader<float> Electron_dr03EcalRecHitSumEt;
  DynamicArrayReader<float> Electron_dr03HcalDepth1TowerSumEt;
  DynamicArrayReader<float> Electron_dr03TkSumPt;
  DynamicArrayReader<float> Electron_dr03TkSumPtHEEP;
  DynamicArrayReader<float> Electron_dxy;
  DynamicArrayReader<float> Electron_dxyErr;
  DynamicArrayReader<float> Electron_dz;
  DynamicArrayReader<float> Electron_dzErr;
  DynamicArrayReader<float> Electron_eta;
  DynamicArrayReader<float> Electron_hoe;
  DynamicArrayReader<float> Electron_ip3d;
  DynamicArrayReader<float> Electron_mass;
  DynamicArrayReader<float> Electron_mvaIso;
  DynamicArrayReader<float> Electron_mvaNoIso;
  DynamicArrayReader<float> Electron_pfRelIso03_all;
  DynamicArrayReader<float> Electron_pfRelIso03_chg;
  DynamicArrayReader<float> Electron_pfRelIso04_all;
  DynamicArrayReader<float> Electron_phi;
  DynamicArrayReader<float> Electron_pt;
  DynamicArrayReader<float> Electron_r9;
  DynamicArrayReader<float> Electron_scEtOverPt;
  DynamicArrayReader<float> Electron_sieie;
  DynamicArrayReader<float> Electron_sip3d;
  DynamicArrayReader<float> Electron_superclusterEta;
  DynamicArrayReader<float> Electron_promptMVA;

  // Jets
  DynamicValueReader<int> nJet;
  DynamicArrayReader<float> Jet_area;
  DynamicArrayReader<float> Jet_btagDeepFlavB;
  DynamicArrayReader<float> Jet_chEmEF;
  DynamicArrayReader<float> Jet_chHEF;
  DynamicArrayReader<float> Jet_eta;
  DynamicArrayReader<float> Jet_mass;
  DynamicArrayReader<float> Jet_muEF;
  DynamicArrayReader<float> Jet_muonSubtrFactor;
  DynamicArrayReader<float> Jet_neEmEF;
  DynamicArrayReader<float> Jet_neHEF;
  DynamicArrayReader<float> Jet_phi;
  DynamicArrayReader<float> Jet_pt;
  DynamicArrayReader<int> Jet_electronIdx1;
  DynamicArrayReader<int> Jet_electronIdx2; 
  DynamicArrayReader<int> Jet_jetId;
  DynamicArrayReader<int> Jet_muonIdx1;     
  DynamicArrayReader<int> Jet_muonIdx2;     
  DynamicArrayReader<int> Jet_nElectrons;
  DynamicArrayReader<int> Jet_nMuons;

  // MET
  DynamicValueReader<float> PuppiMET_phi;
  DynamicValueReader<float> PuppiMET_phiJERUp;
  DynamicValueReader<float> PuppiMET_phiJESUp;
  DynamicValueReader<float> PuppiMET_pt;
  DynamicValueReader<float> PuppiMET_ptJERUp;
  DynamicValueReader<float> PuppiMET_ptJESUp;
  DynamicValueReader<float> PuppiMET_sumEt;

  // Muons
  DynamicValueReader<int> nMuon;
  DynamicArrayReader<bool> Muon_looseId;
  DynamicArrayReader<bool> Muon_mediumId;
  DynamicArrayReader<bool> Muon_mediumPromptId;
  DynamicArrayReader<int> Muon_tightCharge;
  DynamicArrayReader<bool> Muon_tightId;
  DynamicArrayReader<bool> Muon_triggerIdLoose;
  DynamicArrayReader<int> Muon_jetIdx;
  DynamicArrayReader<int> Muon_charge;
  DynamicArrayReader<int> Muon_pdgId;
  DynamicArrayReader<float> Muon_dxy;
  DynamicArrayReader<float> Muon_dxyErr;
  DynamicArrayReader<float> Muon_dxybs;
  DynamicArrayReader<float> Muon_dxybsErr;
  DynamicArrayReader<float> Muon_dz;
  DynamicArrayReader<float> Muon_dzErr;
  DynamicArrayReader<float> Muon_eta;
  DynamicArrayReader<float> Muon_ip3d;
  DynamicArrayReader<float> Muon_mass;
  DynamicArrayReader<float> Muon_mvaMuID;
  DynamicArrayReader<float> Muon_pfRelIso03_all;
  DynamicArrayReader<float> Muon_pfRelIso03_chg;
  DynamicArrayReader<float> Muon_pfRelIso04_all;
  DynamicArrayReader<float> Muon_phi;
  DynamicArrayReader<float> Muon_pt;
  DynamicArrayReader<float> Muon_ptErr;
  DynamicArrayReader<float> Muon_sip3d;
  DynamicArrayReader<float> Muon_tkRelIso;
  DynamicArrayReader<float> Muon_promptMVA;

  // GenParticles
  DynamicValueReader<int> nGenPart;
  DynamicArrayReader<float> GenPart_eta;
  DynamicArrayReader<float> GenPart_mass;
  DynamicArrayReader<float> GenPart_phi;
  DynamicArrayReader<float> GenPart_pt;
  DynamicArrayReader<int> GenPart_genPartIdxMother;
  DynamicArrayReader<int> GenPart_pdgId;
  DynamicArrayReader<int> GenPart_status;

  // GenMET
  DynamicValueReader<float> GenMET_phi;
  DynamicValueReader<float> GenMET_pt;

  // Jet Flavor
  DynamicArrayReader<int> Jet_hadronFlavour;

  nanoAna(TTree * /*tree*/ =0) { }
  ~nanoAna() override { }

  // Default functions:
  Int_t   Version() const override { return 2; }
  void    Begin(TTree *tree) override;
  void    SlaveBegin(TTree *tree) override;
  void    Init(TTree *tree) override;
  bool    Notify() override;
  bool    Process(Long64_t entry) override;
  Int_t   GetEntry(Long64_t entry, Int_t getall = 0) override { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
  void    SetOption(const char *option) override { fOption = option; }
  void    SetObject(TObject *obj) override { fObject = obj; }
  void    SetInputList(TList *input) override { fInput = input; }
  TList  *GetOutputList() const override { return fOutput; }
  void    SlaveTerminate() override;
  void    Terminate() override;

  //-----------------------------------------------------------------------
  //             Make changes here according to your needs
  //-----------------------------------------------------------------------
  
  // User defined structures to keep data from NanoAOD
  struct Hists {
    //Histograms are declared here as a collection of vectors.
    //These need to be dynamicaly expanded in BookHistograms() function.
    vector<TH1F *> hist;
  };
  struct Particle {
    TLorentzVector v;
    int id;
    int ind;
    int status;
    int charge;
    int momid;
    // Expand this to keep more information per object
  };
  
  // User defined functions:
  void SetHstFileName(const char *HstFileName){ _HstFileName = HstFileName;}
  void SetSumFileName(const char *SumFileName){ _SumFileName = SumFileName;}
  void SetData(int data){_data=data;}
  void SetEra(TString era){_era=era;}
  void SetVerbose(int verbose=10000){ _verbosity = verbose; }
  void BookHistograms();
  void SortPt(vector<Particle> &objarr);
  float DeltaPhi(float phi1, float phi2);
  float TransvMass(float E_lep, float MET, float dphi);
  int GenMother(int ind, int mom_ind);

protected:
  Hists h;

private:
  // Global variable declarations go here. Make them global only if necessary.
  // These need to be initiaized somewhere before the Process() function is called.

  TFile *_HstFile;

  // Absolute constants: (fed from the driver script)
  const char *_HstFileName;
  const char *_SumFileName;
  TString _era;
  int _verbosity;

  // Constants: (Initialized once; do not change during execution)
  int nEvtGen, test_event;
  int _data, _year;
  TString _samplename;

  // Flags
  bool GoodEvt, GoodEvt2016, GoodEvt2017, GoodEvt2018;
  
  // Event counters:.
  int nEvtTotal, nEvtRan, nEvtPass;

  //Variables:
  float metpt, metphi; //Used to toggle between different kinds of MET

  // Time counters:
  time_t start, end, buffer;
  
  // Arrays of objects used in analysis:
  vector<Particle> RecoMu, RecoJet;

  //---------------------------------------------------------------------------------
  // DNN block
  // Global variables for DNNs:
  Ort::Env* ort_env; 

  // DNN 1: DY-vs-VLLD
  Ort::Session* session_dy;
  std::vector<float> scale_min_dy;
  std::vector<float> scale_max_dy;

  // Add other DNN sessions here ...

  // DNN specific functions:
  vector<float> loadScalingParameters(const char* filename);
  float evaluateDNN(Ort::Session* session, 
		    std::vector<float> input_vars, 
		    const std::vector<float>& scale_min, 
		    const std::vector<float>& scale_max,
		    const char* input_name = "input",    //Specific to the model
		    const char* output_name = "output"); //Specific to the model
  //---------------------------------------------------------------------------------
  

  ClassDefOverride(nanoAna,0);
};

#endif
