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

// Header for reading branches with dynamic casting
#include "headers/DynamicReaders.h"

// Headers specific to ONNX
#include "onnxruntime-linux-x64-1.24.4/include/onnxruntime_cxx_api.h"
#include <TVector3.h>
#include <TVector2.h>

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

  //-------------------------
  // Regular branches
  //-------------------------
  
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

  // IsoTracks
  DynamicValueReader<int> nIsoTrack;
  DynamicArrayReader<bool> IsoTrack_isHighPurityTrack;
  DynamicArrayReader<bool> IsoTrack_isPFcand;
  DynamicArrayReader<bool> IsoTrack_isFromLostTrack;
  DynamicArrayReader<int> IsoTrack_charge;
  DynamicArrayReader<int> IsoTrack_fromPV;
  DynamicArrayReader<int> IsoTrack_pdgId;
  DynamicArrayReader<float> IsoTrack_dxy;
  DynamicArrayReader<float> IsoTrack_dz;
  DynamicArrayReader<float> IsoTrack_eta;
  DynamicArrayReader<float> IsoTrack_pfRelIso03_all;
  DynamicArrayReader<float> IsoTrack_pfRelIso03_chg;
  DynamicArrayReader<float> IsoTrack_phi;
  DynamicArrayReader<float> IsoTrack_pt;
  DynamicArrayReader<float> IsoTrack_miniPFRelIso_all;
  DynamicArrayReader<float> IsoTrack_miniPFRelIso_chg;

  // Jets
  DynamicValueReader<int> nJet;
  DynamicArrayReader<float> Jet_area;
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
  DynamicArrayReader<int> Jet_hadronFlavour; // Gen
  // Note on Jet Flavor: All other Jet_* will run on data, but Jet_hadronFlavor will not.
  // Do not initiallize with fallback value; use _data flag before using it.
  // Let the code crash in case it is used outside if(_data==1).

  // b-tagging scores
  DynamicArrayReader<float> Jet_btagDeepFlavB;
  DynamicArrayReader<float> Jet_btagDeepFlavCvB;
  DynamicArrayReader<float> Jet_btagDeepFlavCvL;
  DynamicArrayReader<float> Jet_btagDeepFlavQG;
  DynamicArrayReader<float> Jet_btagPNetB;
  DynamicArrayReader<float> Jet_btagPNetCvB;
  DynamicArrayReader<float> Jet_btagPNetCvL;
  DynamicArrayReader<float> Jet_btagPNetCvNotB;
  DynamicArrayReader<float> Jet_btagPNetQvG;
  DynamicArrayReader<float> Jet_btagPNetTauVJet;
  DynamicArrayReader<float> Jet_btagUParTAK4B;
  DynamicArrayReader<float> Jet_btagUParTAK4CvB;
  DynamicArrayReader<float> Jet_btagUParTAK4CvL;
  DynamicArrayReader<float> Jet_btagUParTAK4CvNotB;
  DynamicArrayReader<float> Jet_btagUParTAK4Ele;
  DynamicArrayReader<float> Jet_btagUParTAK4Mu;
  DynamicArrayReader<float> Jet_btagUParTAK4QvG;
  DynamicArrayReader<float> Jet_btagUParTAK4SvCB;
  DynamicArrayReader<float> Jet_btagUParTAK4SvUDG;
  DynamicArrayReader<float> Jet_btagUParTAK4TauVJet;
  DynamicArrayReader<float> Jet_btagUParTAK4UDG;
  DynamicArrayReader<float> Jet_btagUParTAK4probb;
  DynamicArrayReader<float> Jet_btagUParTAK4probbb;
  
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

  // Photons
  DynamicValueReader<int> nPhoton;
  DynamicArrayReader<float> Photon_energyErr;
  DynamicArrayReader<float> Photon_eta;
  DynamicArrayReader<float> Photon_hoe;
  DynamicArrayReader<float> Photon_mvaID;
  DynamicArrayReader<float> Photon_phi;
  DynamicArrayReader<float> Photon_pt;
  DynamicArrayReader<float> Photon_r9;
  DynamicArrayReader<float> Photon_sieie;
  DynamicArrayReader<int> Photon_cutBased;
  DynamicArrayReader<int> Photon_electronIdx;
  DynamicArrayReader<int> Photon_jetIdx;
  DynamicArrayReader<int> Photon_vidNestedWPBitmap;
  DynamicArrayReader<bool> Photon_electronVeto;
  DynamicArrayReader<bool> Photon_isScEtaEB;
  DynamicArrayReader<bool> Photon_isScEtaEE;
  DynamicArrayReader<bool> Photon_mvaID_WP80;
  DynamicArrayReader<bool> Photon_mvaID_WP90;
  DynamicArrayReader<bool> Photon_pixelSeed;
  DynamicArrayReader<unsigned char> Photon_seedGain;
  DynamicArrayReader<float> Photon_pfRelIso03_all;
  DynamicArrayReader<float> Photon_pfRelIso03_chg;

  // PuppiMET
  DynamicValueReader<float> PuppiMET_phi;
  DynamicValueReader<float> PuppiMET_phiJERUp;
  DynamicValueReader<float> PuppiMET_phiJESUp;
  DynamicValueReader<float> PuppiMET_pt;
  DynamicValueReader<float> PuppiMET_ptJERUp;
  DynamicValueReader<float> PuppiMET_ptJESUp;
  DynamicValueReader<float> PuppiMET_sumEt;

  // Rho (used during JEC/JES)
  DynamicValueReader<float> fixedGridRhoFastjetAll;
  DynamicValueReader<float> fixedGridRhoFastjetCentral;
  DynamicValueReader<float> fixedGridRhoFastjetCentralCalo;
  DynamicValueReader<float> fixedGridRhoFastjetCentralChargedPileUp;
  DynamicValueReader<float> fixedGridRhoFastjetCentralNeutral;

  // Taus
  DynamicValueReader<int> nTau;
  DynamicArrayReader<float> Tau_chargedIso;
  DynamicArrayReader<float> Tau_dxy;
  DynamicArrayReader<float> Tau_dz;
  DynamicArrayReader<float> Tau_eta;
  DynamicArrayReader<float> Tau_leadTkDeltaEta;
  DynamicArrayReader<float> Tau_leadTkDeltaPhi;
  DynamicArrayReader<float> Tau_leadTkPtOverTauPt;
  DynamicArrayReader<float> Tau_mass;
  DynamicArrayReader<float> Tau_neutralIso;
  DynamicArrayReader<float> Tau_phi;
  DynamicArrayReader<float> Tau_photonsOutsideSignalCone;
  DynamicArrayReader<float> Tau_pt;
  DynamicArrayReader<float> Tau_puCorr;
  DynamicArrayReader<float> Tau_rawDeepTau2017v2p1VSe;
  DynamicArrayReader<float> Tau_rawDeepTau2017v2p1VSjet;
  DynamicArrayReader<float> Tau_rawDeepTau2017v2p1VSmu;
  DynamicArrayReader<float> Tau_rawIso;
  DynamicArrayReader<float> Tau_rawIsodR03;
  DynamicArrayReader<int> Tau_charge;
  DynamicArrayReader<int> Tau_decayMode;
  DynamicArrayReader<int> Tau_jetIdx;
  DynamicArrayReader<unsigned char> Tau_idAntiMu;
  DynamicArrayReader<unsigned char> Tau_idDeepTau2017v2p1VSe;
  DynamicArrayReader<unsigned char> Tau_idDeepTau2017v2p1VSjet;
  DynamicArrayReader<unsigned char> Tau_idDeepTau2017v2p1VSmu;

  // TrigObj
  DynamicValueReader<int> nTrigObj;
  DynamicArrayReader<float> TrigObj_pt;
  DynamicArrayReader<float> TrigObj_eta;
  DynamicArrayReader<float> TrigObj_phi;
  DynamicArrayReader<float> TrigObj_l1pt;
  DynamicArrayReader<float> TrigObj_l1pt_2;
  DynamicArrayReader<float> TrigObj_l2pt;
  DynamicArrayReader<int> TrigObj_id;
  DynamicArrayReader<int> TrigObj_l1iso;
  DynamicArrayReader<int> TrigObj_l1charge;
  DynamicArrayReader<int> TrigObj_filterBits;

  //-------------------------
  // Booleans
  //-------------------------
  
  // Flags
  DynamicValueReader<bool> Flag_HBHENoiseFilter;
  DynamicValueReader<bool> Flag_HBHENoiseIsoFilter;
  DynamicValueReader<bool> Flag_CSCTightHaloFilter;
  DynamicValueReader<bool> Flag_CSCTightHaloTrkMuUnvetoFilter;
  DynamicValueReader<bool> Flag_CSCTightHalo2015Filter;
  DynamicValueReader<bool> Flag_globalTightHalo2016Filter;
  DynamicValueReader<bool> Flag_globalSuperTightHalo2016Filter;
  DynamicValueReader<bool> Flag_HcalStripHaloFilter;
  DynamicValueReader<bool> Flag_hcalLaserEventFilter;
  DynamicValueReader<bool> Flag_EcalDeadCellTriggerPrimitiveFilter;
  DynamicValueReader<bool> Flag_EcalDeadCellBoundaryEnergyFilter;
  DynamicValueReader<bool> Flag_ecalBadCalibFilter;
  DynamicValueReader<bool> Flag_goodVertices;
  DynamicValueReader<bool> Flag_eeBadScFilter;
  DynamicValueReader<bool> Flag_ecalLaserCorrFilter;
  DynamicValueReader<bool> Flag_trkPOGFilters;
  DynamicValueReader<bool> Flag_chargedHadronTrackResolutionFilter;
  DynamicValueReader<bool> Flag_muonBadTrackFilter;
  DynamicValueReader<bool> Flag_BadChargedCandidateFilter;
  DynamicValueReader<bool> Flag_BadPFMuonFilter;
  DynamicValueReader<bool> Flag_BadPFMuonDzFilter;
  DynamicValueReader<bool> Flag_hfNoisyHitsFilter;
  DynamicValueReader<bool> Flag_BadChargedCandidateSummer16Filter;
  DynamicValueReader<bool> Flag_BadPFMuonSummer16Filter;
  DynamicValueReader<bool> Flag_trkPOG_manystripclus53X;
  DynamicValueReader<bool> Flag_trkPOG_toomanystripclus53X;
  DynamicValueReader<bool> Flag_trkPOG_logErrorTooManyClusters;
  DynamicValueReader<bool> Flag_METFilters;

  // HLT paths
  DynamicValueReader<bool> HLT_IsoMu24;
  DynamicValueReader<bool> HLT_IsoMu27;
  DynamicValueReader<bool> HLT_Ele27_WPTight_Gsf;
  DynamicValueReader<bool> HLT_Ele30_WPTight_Gsf;
  DynamicValueReader<bool> HLT_Ele32_WPTight_Gsf;
  DynamicValueReader<bool> HLT_Ele35_WPTight_Gsf;
  DynamicValueReader<bool> HLT_Ele38_WPTight_Gsf;
  DynamicValueReader<bool> HLT_Ele40_WPTight_Gsf;
  DynamicValueReader<bool> HLT_Ele32_WPTight_Gsf_L1DoubleEG;
  DynamicValueReader<bool> HLT_IsoMu20;
  DynamicValueReader<bool> HLT_IsoMu24_eta2p1;

  //-------------------------
  // Gen-level branches
  //-------------------------

  // GenJet
  DynamicValueReader<int> nGenJet;
  DynamicArrayReader<float> GenJet_eta;
  DynamicArrayReader<float> GenJet_mass;
  DynamicArrayReader<float> GenJet_phi;
  DynamicArrayReader<float> GenJet_pt;
  
  // GenMET
  DynamicValueReader<float> GenMET_phi;
  DynamicValueReader<float> GenMET_pt;
  
  // GenParticles
  DynamicValueReader<int> nGenPart;
  DynamicArrayReader<float> GenPart_eta;
  DynamicArrayReader<float> GenPart_mass;
  DynamicArrayReader<float> GenPart_phi;
  DynamicArrayReader<float> GenPart_pt;
  DynamicArrayReader<int> GenPart_genPartIdxMother;
  DynamicArrayReader<int> GenPart_pdgId;
  DynamicArrayReader<int> GenPart_status;

  //GenWeight
  DynamicValueReader<float> Generator_weight;

  // GenVisTau
  DynamicValueReader<int> nGenVisTau;
  DynamicArrayReader<float> GenVisTau_eta;
  DynamicArrayReader<float> GenVisTau_mass;
  DynamicArrayReader<float> GenVisTau_phi;
  DynamicArrayReader<float> GenVisTau_pt;
  DynamicArrayReader<int> GenVisTau_charge;
  DynamicArrayReader<int> GenVisTau_genPartIdxMother;
  DynamicArrayReader<int> GenVisTau_status;

  // LHE weights (non-QCD MC)
  DynamicValueReader<float> LHEWeight_originalXWGTUP;
  DynamicValueReader<int> nLHEPdfWeight;
  DynamicArrayReader<float> LHEPdfWeight;
  DynamicValueReader<int> nLHEReweightingWeight;
  DynamicArrayReader<float> LHEReweightingWeight;
  DynamicValueReader<int> nLHEScaleWeight;
  DynamicArrayReader<float> LHEScaleWeight;

  // LHE
  DynamicValueReader<int> LHE_Njets;
  DynamicValueReader<int> LHE_Nb;
  DynamicValueReader<int> LHE_Nc;
  DynamicValueReader<int> LHE_Nuds;
  DynamicValueReader<int> LHE_Nglu;
  DynamicValueReader<int> LHE_NpNLO;
  DynamicValueReader<int> LHE_NpLO;
  DynamicValueReader<float> LHE_HT;
  DynamicValueReader<float> LHE_HTIncoming;
  DynamicValueReader<float> LHE_Vpt;
  DynamicValueReader<float> LHE_AlphaS;

  // LHEPart
  DynamicValueReader<int> nLHEPart;
  DynamicArrayReader<int> LHEPart_firstMotherIdx;
  DynamicArrayReader<int> LHEPart_lastMotherIdx;
  DynamicArrayReader<int> LHEPart_pdgId;
  DynamicArrayReader<int> LHEPart_status;
  DynamicArrayReader<int> LHEPart_spin;
  DynamicArrayReader<float> LHEPart_pt;
  DynamicArrayReader<float> LHEPart_eta;
  DynamicArrayReader<float> LHEPart_phi;
  DynamicArrayReader<float> LHEPart_mass;
  DynamicArrayReader<float> LHEPart_incomingpz;

  //-----------------------------------------------------------------------

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
    vector<TH1F *> dnn;
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
  int nEvtTotal, nEvtRan, nEvtTrigger, nEvtPass;

  //Variables:
  float metpt, metphi, genEventSumW;

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
