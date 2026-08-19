#ifndef nanoAna_Init_h
#define nanoAna_Init_h
#include "nanoAna.h"

void nanoAna::Init(TTree *tree)
{
  fReader.SetTree(tree);

  // --------------------------------------------------------------------------------------------
  // Dynamic Branch Initialization
  // Syntax: ReaderName.Init(tree, fReader, {"BranchOption1", "BranchOption2", ..}, DefaultValue);
  //
  // - ReaderName: The user-defined object declared in nanoAna.h.
  // - BranchOptions: A priority list of NanoAOD branch names. The reader 
  //                  dynamically binds to the first valid branch it finds.
  // - DefaultValue: The fallback value applied if none of the branches exist.
  //
  // Branch names are available here: https://cms-xpog.docs.cern.ch/autoDoc/
  // ---------------------------------------------------------------------------------------------
  
  fixedGridRhoFastjetAll.Init(tree, fReader, {"Rho_fixedGridRhoFastjetAll", "fixedGridRhoFastjetAll"}, 0.0);
  LHEWeight_originalXWGTUP.Init(tree, fReader, "LHEWeight_originalXWGTUP", 1.0);

  // Initialize Electrons
  nElectron.Init(tree, fReader, "nElectron", 0);
  Electron_convVeto.Init(tree, fReader, "Electron_convVeto");
  Electron_cutBased.Init(tree, fReader, "Electron_cutBased");
  Electron_cutBased_HEEP.Init(tree, fReader, "Electron_cutBased_HEEP");
  Electron_mvaIso_WP80.Init(tree, fReader, "Electron_mvaIso_WP80");
  Electron_mvaIso_WP90.Init(tree, fReader, "Electron_mvaIso_WP90");
  Electron_mvaIso_WPHZZ.Init(tree, fReader, "Electron_mvaIso_WPHZZ");
  Electron_mvaNoIso_WP80.Init(tree, fReader, "Electron_mvaNoIso_WP80");
  Electron_mvaNoIso_WP90.Init(tree, fReader, "Electron_mvaNoIso_WP90");
  Electron_seedGain.Init(tree, fReader, "Electron_seedGain");
  Electron_tightCharge.Init(tree, fReader, "Electron_tightCharge");
  Electron_jetIdx.Init(tree, fReader, "Electron_jetIdx");
  Electron_photonIdx.Init(tree, fReader, "Electron_photonIdx");
  Electron_charge.Init(tree, fReader, "Electron_charge");
  Electron_pdgId.Init(tree, fReader, "Electron_pdgId");
  Electron_deltaEtaSC.Init(tree, fReader, "Electron_deltaEtaSC");
  Electron_dr03EcalRecHitSumEt.Init(tree, fReader, "Electron_dr03EcalRecHitSumEt");
  Electron_dr03HcalDepth1TowerSumEt.Init(tree, fReader, "Electron_dr03HcalDepth1TowerSumEt");
  Electron_dr03TkSumPt.Init(tree, fReader, "Electron_dr03TkSumPt");
  Electron_dr03TkSumPtHEEP.Init(tree, fReader, "Electron_dr03TkSumPtHEEP");
  Electron_dxy.Init(tree, fReader, "Electron_dxy");
  Electron_dxyErr.Init(tree, fReader, "Electron_dxyErr");
  Electron_dz.Init(tree, fReader, "Electron_dz");
  Electron_dzErr.Init(tree, fReader, "Electron_dzErr");
  Electron_eta.Init(tree, fReader, "Electron_eta");
  Electron_hoe.Init(tree, fReader, "Electron_hoe");
  Electron_ip3d.Init(tree, fReader, "Electron_ip3d");
  Electron_mass.Init(tree, fReader, "Electron_mass");
  Electron_mvaIso.Init(tree, fReader, "Electron_mvaIso");
  Electron_mvaNoIso.Init(tree, fReader, "Electron_mvaNoIso");
  Electron_pfRelIso03_all.Init(tree, fReader, "Electron_pfRelIso03_all");
  Electron_pfRelIso03_chg.Init(tree, fReader, "Electron_pfRelIso03_chg");
  Electron_pfRelIso04_all.Init(tree, fReader, "Electron_pfRelIso04_all");
  Electron_phi.Init(tree, fReader, "Electron_phi");
  Electron_pt.Init(tree, fReader, "Electron_pt");
  Electron_r9.Init(tree, fReader, "Electron_r9");
  Electron_scEtOverPt.Init(tree, fReader, "Electron_scEtOverPt");
  Electron_sieie.Init(tree, fReader, "Electron_sieie");
  Electron_sip3d.Init(tree, fReader, "Electron_sip3d");
  Electron_superclusterEta.Init(tree, fReader, "Electron_superclusterEta");
  Electron_promptMVA.Init(tree, fReader, "Electron_promptMVA");

  // Initialize Muons
  nMuon.Init(tree, fReader, "nMuon", 0);
  Muon_looseId.Init(tree, fReader, "Muon_looseId");
  Muon_mediumId.Init(tree, fReader, "Muon_mediumId");
  Muon_mediumPromptId.Init(tree, fReader, "Muon_mediumPromptId");
  Muon_tightCharge.Init(tree, fReader, "Muon_tightCharge");
  Muon_tightId.Init(tree, fReader, "Muon_tightId");
  Muon_triggerIdLoose.Init(tree, fReader, "Muon_triggerIdLoose");
  Muon_jetIdx.Init(tree, fReader, "Muon_jetIdx");
  Muon_charge.Init(tree, fReader, "Muon_charge");
  Muon_pdgId.Init(tree, fReader, "Muon_pdgId");
  Muon_dxy.Init(tree, fReader, "Muon_dxy");
  Muon_dxyErr.Init(tree, fReader, "Muon_dxyErr");
  Muon_dxybs.Init(tree, fReader, "Muon_dxybs");
  Muon_dxybsErr.Init(tree, fReader, "Muon_dxybsErr");
  Muon_dz.Init(tree, fReader, "Muon_dz");
  Muon_dzErr.Init(tree, fReader, "Muon_dzErr");
  Muon_eta.Init(tree, fReader, "Muon_eta");
  Muon_ip3d.Init(tree, fReader, "Muon_ip3d");
  Muon_mass.Init(tree, fReader, "Muon_mass");
  Muon_mvaMuID.Init(tree, fReader, "Muon_mvaMuID");
  Muon_pfRelIso03_all.Init(tree, fReader, "Muon_pfRelIso03_all");
  Muon_pfRelIso03_chg.Init(tree, fReader, "Muon_pfRelIso03_chg");
  Muon_pfRelIso04_all.Init(tree, fReader, "Muon_pfRelIso04_all");
  Muon_phi.Init(tree, fReader, "Muon_phi");
  Muon_pt.Init(tree, fReader, "Muon_pt");
  Muon_ptErr.Init(tree, fReader, "Muon_ptErr");
  Muon_sip3d.Init(tree, fReader, "Muon_sip3d");
  Muon_tkRelIso.Init(tree, fReader, "Muon_tkRelIso");
  Muon_promptMVA.Init(tree, fReader, "Muon_promptMVA");
}

bool nanoAna::Notify()
{
  return true;
}

#endif // nanoAna_Init_h
