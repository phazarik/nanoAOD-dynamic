#ifndef nanoAna_Init_h
#define nanoAna_Init_h
#include "../nanoAna.h"

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

  //-------------------------
  // Regular branches
  //-------------------------
  
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

  // Initialize IsoTrack
  nIsoTrack.Init(tree, fReader, "nIsoTrack", 0);
  IsoTrack_isHighPurityTrack.Init(tree, fReader, "IsoTrack_isHighPurityTrack");
  IsoTrack_isPFcand.Init(tree, fReader, "IsoTrack_isPFcand");
  IsoTrack_isFromLostTrack.Init(tree, fReader, "IsoTrack_isFromLostTrack");
  IsoTrack_charge.Init(tree, fReader, "IsoTrack_charge");
  IsoTrack_fromPV.Init(tree, fReader, "IsoTrack_fromPV");
  IsoTrack_pdgId.Init(tree, fReader, "IsoTrack_pdgId");
  IsoTrack_dxy.Init(tree, fReader, "IsoTrack_dxy");
  IsoTrack_dz.Init(tree, fReader, "IsoTrack_dz");
  IsoTrack_eta.Init(tree, fReader, "IsoTrack_eta");
  IsoTrack_pfRelIso03_all.Init(tree, fReader, "IsoTrack_pfRelIso03_all");
  IsoTrack_pfRelIso03_chg.Init(tree, fReader, "IsoTrack_pfRelIso03_chg");
  IsoTrack_phi.Init(tree, fReader, "IsoTrack_phi");
  IsoTrack_pt.Init(tree, fReader, "IsoTrack_pt");
  IsoTrack_miniPFRelIso_all.Init(tree, fReader, "IsoTrack_miniPFRelIso_all");
  IsoTrack_miniPFRelIso_chg.Init(tree, fReader, "IsoTrack_miniPFRelIso_chg");

  // Initialize Jet
  nJet.Init(tree, fReader, "nJet", 0);
  Jet_area.Init(tree, fReader, "Jet_area");
  Jet_chEmEF.Init(tree, fReader, "Jet_chEmEF");
  Jet_chHEF.Init(tree, fReader, "Jet_chHEF");
  Jet_eta.Init(tree, fReader, "Jet_eta");
  Jet_mass.Init(tree, fReader, "Jet_mass");
  Jet_muEF.Init(tree, fReader, "Jet_muEF");
  Jet_muonSubtrFactor.Init(tree, fReader, "Jet_muonSubtrFactor");
  Jet_neEmEF.Init(tree, fReader, "Jet_neEmEF");
  Jet_neHEF.Init(tree, fReader, "Jet_neHEF");
  Jet_phi.Init(tree, fReader, "Jet_phi");
  Jet_pt.Init(tree, fReader, "Jet_pt");
  Jet_electronIdx1.Init(tree, fReader, "Jet_electronIdx1");
  Jet_electronIdx2.Init(tree, fReader, "Jet_electronIdx2");
  Jet_jetId.Init(tree, fReader, "Jet_jetId");
  Jet_muonIdx1.Init(tree, fReader, "Jet_muonIdx1");
  Jet_muonIdx2.Init(tree, fReader, "Jet_muonIdx2");
  Jet_nElectrons.Init(tree, fReader, "Jet_nElectrons");
  Jet_nMuons.Init(tree, fReader, "Jet_nMuons");

  // Initialize b-tagging scores
  Jet_btagDeepFlavB.Init(tree, fReader, "Jet_btagDeepFlavB");
  Jet_btagDeepFlavCvB.Init(tree, fReader, "Jet_btagDeepFlavCvB");
  Jet_btagDeepFlavCvL.Init(tree, fReader, "Jet_btagDeepFlavCvL");
  Jet_btagDeepFlavQG.Init(tree, fReader, "Jet_btagDeepFlavQG");
  Jet_btagPNetB.Init(tree, fReader, "Jet_btagPNetB");
  Jet_btagPNetCvB.Init(tree, fReader, "Jet_btagPNetCvB");
  Jet_btagPNetCvL.Init(tree, fReader, "Jet_btagPNetCvL");
  Jet_btagPNetCvNotB.Init(tree, fReader, "Jet_btagPNetCvNotB");
  Jet_btagPNetQvG.Init(tree, fReader, "Jet_btagPNetQvG");
  Jet_btagPNetTauVJet.Init(tree, fReader, "Jet_btagPNetTauVJet");
  Jet_btagUParTAK4B.Init(tree, fReader, "Jet_btagUParTAK4B");
  Jet_btagUParTAK4CvB.Init(tree, fReader, "Jet_btagUParTAK4CvB");
  Jet_btagUParTAK4CvL.Init(tree, fReader, "Jet_btagUParTAK4CvL");
  Jet_btagUParTAK4CvNotB.Init(tree, fReader, "Jet_btagUParTAK4CvNotB");
  Jet_btagUParTAK4Ele.Init(tree, fReader, "Jet_btagUParTAK4Ele");
  Jet_btagUParTAK4Mu.Init(tree, fReader, "Jet_btagUParTAK4Mu");
  Jet_btagUParTAK4QvG.Init(tree, fReader, "Jet_btagUParTAK4QvG");
  Jet_btagUParTAK4SvCB.Init(tree, fReader, "Jet_btagUParTAK4SvCB");
  Jet_btagUParTAK4SvUDG.Init(tree, fReader, "Jet_btagUParTAK4SvUDG");
  Jet_btagUParTAK4TauVJet.Init(tree, fReader, "Jet_btagUParTAK4TauVJet");
  Jet_btagUParTAK4UDG.Init(tree, fReader, "Jet_btagUParTAK4UDG");
  Jet_btagUParTAK4probb.Init(tree, fReader, "Jet_btagUParTAK4probb");
  Jet_btagUParTAK4probbb.Init(tree, fReader, "Jet_btagUParTAK4probbb");
  Jet_hadronFlavour.Init(tree, fReader, "Jet_hadronFlavour");
  // Note: All other Jet_* will run on data, but Jet_hadronFlavor will not.
  // Do not initiallize with fallback value; use _data flag before using it.
  // Let the code crash in case it is used outside if(_data==1).
  
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

  // Initialize Photons
  nPhoton.Init(tree, fReader, "nPhoton", 0);
  Photon_energyErr.Init(tree, fReader, "Photon_energyErr");
  Photon_eta.Init(tree, fReader, "Photon_eta");
  Photon_hoe.Init(tree, fReader, "Photon_hoe");
  Photon_mvaID.Init(tree, fReader, "Photon_mvaID");
  Photon_phi.Init(tree, fReader, "Photon_phi");
  Photon_pt.Init(tree, fReader, "Photon_pt");
  Photon_r9.Init(tree, fReader, "Photon_r9");
  Photon_sieie.Init(tree, fReader, "Photon_sieie");
  Photon_cutBased.Init(tree, fReader, "Photon_cutBased");
  Photon_electronIdx.Init(tree, fReader, "Photon_electronIdx");
  Photon_jetIdx.Init(tree, fReader, "Photon_jetIdx");
  Photon_vidNestedWPBitmap.Init(tree, fReader, "Photon_vidNestedWPBitmap");
  Photon_electronVeto.Init(tree, fReader, "Photon_electronVeto");
  Photon_isScEtaEB.Init(tree, fReader, "Photon_isScEtaEB");
  Photon_isScEtaEE.Init(tree, fReader, "Photon_isScEtaEE");
  Photon_mvaID_WP80.Init(tree, fReader, "Photon_mvaID_WP80");
  Photon_mvaID_WP90.Init(tree, fReader, "Photon_mvaID_WP90");
  Photon_pixelSeed.Init(tree, fReader, "Photon_pixelSeed");
  Photon_seedGain.Init(tree, fReader, "Photon_seedGain");
  Photon_pfRelIso03_all.Init(tree, fReader, {"Photon_pfRelIso03_all_quadratic", "Photon_pfRelIso03_all"});
  Photon_pfRelIso03_chg.Init(tree, fReader, {"Photon_pfRelIso03_chg_quadratic", "Photon_pfRelIso03_chg"});

  // Initialize PuppiMET
  PuppiMET_phi.Init(tree, fReader, "PuppiMET_phi", 0);
  PuppiMET_phiJERUp.Init(tree, fReader, "PuppiMET_phiJERUp", 0);
  PuppiMET_phiJESUp.Init(tree, fReader, "PuppiMET_phiJESUp", 0);
  PuppiMET_pt.Init(tree, fReader, "PuppiMET_pt", 0);
  PuppiMET_ptJERUp.Init(tree, fReader, "PuppiMET_ptJERUp", 0);
  PuppiMET_ptJESUp.Init(tree, fReader, "PuppiMET_ptJESUp", 0);
  PuppiMET_sumEt.Init(tree, fReader, "PuppiMET_sumEt", 0);

  // Initialize Rho
  fixedGridRhoFastjetAll.Init(tree,fReader,{"Rho_fixedGridRhoFastjetAll","fixedGridRhoFastjetAll"},0.0);
  fixedGridRhoFastjetCentral.Init(tree,fReader,{"Rho_fixedGridRhoFastjetCentral","fixedGridRhoFastjetCentral"},0.0);
  fixedGridRhoFastjetCentralCalo.Init(tree,fReader,{"Rho_fixedGridRhoFastjetCentralCalo","fixedGridRhoFastjetCentralCalo"},0.0);
  fixedGridRhoFastjetCentralChargedPileUp.Init(tree,fReader,{"Rho_fixedGridRhoFastjetCentralChargedPileUp","fixedGridRhoFastjetCentralChargedPileUp"},0.0);
  fixedGridRhoFastjetCentralNeutral.Init(tree,fReader,{"Rho_fixedGridRhoFastjetCentralNeutral","fixedGridRhoFastjetCentralNeutral"},0.0);
 
  // Initialize Taus
  nTau.Init(tree, fReader, "nTau", 0);
  Tau_chargedIso.Init(tree, fReader, "Tau_chargedIso");
  Tau_dxy.Init(tree, fReader, "Tau_dxy");
  Tau_dz.Init(tree, fReader, "Tau_dz");
  Tau_eta.Init(tree, fReader, "Tau_eta");
  Tau_leadTkDeltaEta.Init(tree, fReader, "Tau_leadTkDeltaEta");
  Tau_leadTkDeltaPhi.Init(tree, fReader, "Tau_leadTkDeltaPhi");
  Tau_leadTkPtOverTauPt.Init(tree, fReader, "Tau_leadTkPtOverTauPt");
  Tau_mass.Init(tree, fReader, "Tau_mass");
  Tau_neutralIso.Init(tree, fReader, "Tau_neutralIso");
  Tau_phi.Init(tree, fReader, "Tau_phi");
  Tau_photonsOutsideSignalCone.Init(tree, fReader, "Tau_photonsOutsideSignalCone");
  Tau_pt.Init(tree, fReader, "Tau_pt");
  Tau_puCorr.Init(tree, fReader, "Tau_puCorr");
  Tau_rawDeepTau2017v2p1VSe.Init(tree, fReader, "Tau_rawDeepTau2017v2p1VSe");
  Tau_rawDeepTau2017v2p1VSjet.Init(tree, fReader, "Tau_rawDeepTau2017v2p1VSjet");
  Tau_rawDeepTau2017v2p1VSmu.Init(tree, fReader, "Tau_rawDeepTau2017v2p1VSmu");
  Tau_rawIso.Init(tree, fReader, "Tau_rawIso");
  Tau_rawIsodR03.Init(tree, fReader, "Tau_rawIsodR03");
  Tau_charge.Init(tree, fReader, "Tau_charge");
  Tau_decayMode.Init(tree, fReader, "Tau_decayMode");
  Tau_jetIdx.Init(tree, fReader, "Tau_jetIdx");
  Tau_idAntiMu.Init(tree, fReader, "Tau_idAntiMu");
  Tau_idDeepTau2017v2p1VSe.Init(tree, fReader, "Tau_idDeepTau2017v2p1VSe");
  Tau_idDeepTau2017v2p1VSjet.Init(tree, fReader, "Tau_idDeepTau2017v2p1VSjet");
  Tau_idDeepTau2017v2p1VSmu.Init(tree, fReader, "Tau_idDeepTau2017v2p1VSmu");

  // Initialize TrigObj
  nTrigObj.Init(tree, fReader, "nTrigObj", 0);
  TrigObj_pt.Init(tree, fReader, "TrigObj_pt");
  TrigObj_eta.Init(tree, fReader, "TrigObj_eta");
  TrigObj_phi.Init(tree, fReader, "TrigObj_phi");
  TrigObj_l1pt.Init(tree, fReader, "TrigObj_l1pt");
  TrigObj_l1pt_2.Init(tree, fReader, "TrigObj_l1pt_2");
  TrigObj_l2pt.Init(tree, fReader, "TrigObj_l2pt");
  TrigObj_id.Init(tree, fReader, "TrigObj_id");
  TrigObj_l1iso.Init(tree, fReader, "TrigObj_l1iso");
  TrigObj_l1charge.Init(tree, fReader, "TrigObj_l1charge");
  TrigObj_filterBits.Init(tree, fReader, "TrigObj_filterBits");

  //-------------------------
  // Booleans
  //-------------------------

  // Initialize Flags
  Flag_HBHENoiseFilter.Init(tree, fReader, "Flag_HBHENoiseFilter", true);
  Flag_HBHENoiseIsoFilter.Init(tree, fReader, "Flag_HBHENoiseIsoFilter", true);
  Flag_CSCTightHaloFilter.Init(tree, fReader, "Flag_CSCTightHaloFilter", true);
  Flag_CSCTightHaloTrkMuUnvetoFilter.Init(tree, fReader, "Flag_CSCTightHaloTrkMuUnvetoFilter", true);
  Flag_CSCTightHalo2015Filter.Init(tree, fReader, "Flag_CSCTightHalo2015Filter", true);
  Flag_globalTightHalo2016Filter.Init(tree, fReader, "Flag_globalTightHalo2016Filter", true);
  Flag_globalSuperTightHalo2016Filter.Init(tree, fReader, "Flag_globalSuperTightHalo2016Filter", true);
  Flag_HcalStripHaloFilter.Init(tree, fReader, "Flag_HcalStripHaloFilter", true);
  Flag_hcalLaserEventFilter.Init(tree, fReader, "Flag_hcalLaserEventFilter", true);
  Flag_EcalDeadCellTriggerPrimitiveFilter.Init(tree, fReader, "Flag_EcalDeadCellTriggerPrimitiveFilter", true);
  Flag_EcalDeadCellBoundaryEnergyFilter.Init(tree, fReader, "Flag_EcalDeadCellBoundaryEnergyFilter", true);
  Flag_ecalBadCalibFilter.Init(tree, fReader, "Flag_ecalBadCalibFilter", true);
  Flag_goodVertices.Init(tree, fReader, "Flag_goodVertices", true);
  Flag_eeBadScFilter.Init(tree, fReader, "Flag_eeBadScFilter", true);
  Flag_ecalLaserCorrFilter.Init(tree, fReader, "Flag_ecalLaserCorrFilter", true);
  Flag_trkPOGFilters.Init(tree, fReader, "Flag_trkPOGFilters", true);
  Flag_chargedHadronTrackResolutionFilter.Init(tree, fReader, "Flag_chargedHadronTrackResolutionFilter", true);
  Flag_muonBadTrackFilter.Init(tree, fReader, "Flag_muonBadTrackFilter", true);
  Flag_BadChargedCandidateFilter.Init(tree, fReader, "Flag_BadChargedCandidateFilter", true);
  Flag_BadPFMuonFilter.Init(tree, fReader, "Flag_BadPFMuonFilter", true);
  Flag_BadPFMuonDzFilter.Init(tree, fReader, "Flag_BadPFMuonDzFilter", true);
  Flag_hfNoisyHitsFilter.Init(tree, fReader, "Flag_hfNoisyHitsFilter", true);
  Flag_BadChargedCandidateSummer16Filter.Init(tree, fReader, "Flag_BadChargedCandidateSummer16Filter", true);
  Flag_BadPFMuonSummer16Filter.Init(tree, fReader, "Flag_BadPFMuonSummer16Filter", true);
  Flag_trkPOG_manystripclus53X.Init(tree, fReader, "Flag_trkPOG_manystripclus53X", true);
  Flag_trkPOG_toomanystripclus53X.Init(tree, fReader, "Flag_trkPOG_toomanystripclus53X", true);
  Flag_trkPOG_logErrorTooManyClusters.Init(tree, fReader, "Flag_trkPOG_logErrorTooManyClusters", true);
  Flag_METFilters.Init(tree, fReader, "Flag_METFilters", true);
 
  // Initialize HLT paths
  HLT_IsoMu24.Init(tree, fReader, "HLT_IsoMu24");
  HLT_IsoMu27.Init(tree, fReader, "HLT_IsoMu27");
  HLT_Ele27_WPTight_Gsf.Init(tree, fReader, "HLT_Ele27_WPTight_Gsf");
  HLT_Ele30_WPTight_Gsf.Init(tree, fReader, "HLT_Ele30_WPTight_Gsf");
  HLT_Ele32_WPTight_Gsf.Init(tree, fReader, "HLT_Ele32_WPTight_Gsf");
  HLT_Ele35_WPTight_Gsf.Init(tree, fReader, "HLT_Ele35_WPTight_Gsf");
  HLT_Ele38_WPTight_Gsf.Init(tree, fReader, "HLT_Ele38_WPTight_Gsf");
  HLT_Ele40_WPTight_Gsf.Init(tree, fReader, "HLT_Ele40_WPTight_Gsf");
  HLT_Ele32_WPTight_Gsf_L1DoubleEG.Init(tree, fReader, "HLT_Ele32_WPTight_Gsf_L1DoubleEG");
  HLT_IsoMu20.Init(tree, fReader, "HLT_IsoMu20");
  HLT_IsoMu24_eta2p1.Init(tree, fReader, "HLT_IsoMu24_eta2p1");

  //-------------------------
  // Gen-level branches
  //-------------------------
  
  // Initialize GenMET
  GenMET_phi.Init(tree, fReader, "GenMET_phi");
  GenMET_pt.Init(tree, fReader, "GenMET_pt");

  // Initialize GenJet
  nGenJet.Init(tree, fReader, "nGenJet", 0);
  GenJet_eta.Init(tree, fReader, "GenJet_eta");
  GenJet_mass.Init(tree, fReader, "GenJet_mass");
  GenJet_phi.Init(tree, fReader, "GenJet_phi");
  GenJet_pt.Init(tree, fReader, "GenJet_pt");

  // Initialize GenParticles
  nGenPart.Init(tree, fReader, "nGenPart", 0);
  GenPart_eta.Init(tree, fReader, "GenPart_eta");
  GenPart_mass.Init(tree, fReader, "GenPart_mass");
  GenPart_phi.Init(tree, fReader, "GenPart_phi");
  GenPart_pt.Init(tree, fReader, "GenPart_pt");
  GenPart_genPartIdxMother.Init(tree, fReader, "GenPart_genPartIdxMother");
  GenPart_pdgId.Init(tree, fReader, "GenPart_pdgId");
  GenPart_status.Init(tree, fReader, "GenPart_status");

  // Initialize GenVisTau
  nGenVisTau.Init(tree, fReader, "nGenVisTau", 0);
  GenVisTau_eta.Init(tree, fReader, "GenVisTau_eta");
  GenVisTau_mass.Init(tree, fReader, "GenVisTau_mass");
  GenVisTau_phi.Init(tree, fReader, "GenVisTau_phi");
  GenVisTau_pt.Init(tree, fReader, "GenVisTau_pt");
  GenVisTau_charge.Init(tree, fReader, "GenVisTau_charge");
  GenVisTau_genPartIdxMother.Init(tree, fReader, "GenVisTau_genPartIdxMother");
  GenVisTau_status.Init(tree, fReader, "GenVisTau_status");

  // Initialize LHE weights
  LHEWeight_originalXWGTUP.Init(tree, fReader, "LHEWeight_originalXWGTUP", 1.0); // weight fallback: 1
  nLHEPdfWeight.Init(tree, fReader, "nLHEPdfWeight", 0);
  LHEPdfWeight.Init(tree, fReader, "LHEPdfWeight");
  nLHEReweightingWeight.Init(tree, fReader, "nLHEReweightingWeight", 0);
  LHEReweightingWeight.Init(tree, fReader, "LHEReweightingWeight");
  nLHEScaleWeight.Init(tree, fReader, "nLHEScaleWeight", 0);
  LHEScaleWeight.Init(tree, fReader, "LHEScaleWeight");

  // Initialize LHE (assigning absurt -1.0 fallback values. Use only when value is non negative)
  LHE_Njets.Init(tree, fReader, "LHE_Njets", -1); //Use it to stitch nJ-binned samples
  LHE_Nb.Init(tree, fReader, "LHE_Nb", -1);
  LHE_Nc.Init(tree, fReader, "LHE_Nc", -1);
  LHE_Nuds.Init(tree, fReader, "LHE_Nuds", -1);
  LHE_Nglu.Init(tree, fReader, "LHE_Nglu", -1);
  LHE_NpNLO.Init(tree, fReader, "LHE_NpNLO", -1);
  LHE_NpLO.Init(tree, fReader, "LHE_NpLO", -1);
  LHE_HT.Init(tree, fReader, "LHE_HT", -1.0);
  LHE_HTIncoming.Init(tree, fReader, "LHE_HTIncoming", -1.0); //Use it to stitch HT-binned samples
  LHE_Vpt.Init(tree, fReader, "LHE_Vpt", -1.0);
  LHE_AlphaS.Init(tree, fReader, "LHE_AlphaS", -1.0);

  // Initialize LHEPart
  nLHEPart.Init(tree, fReader, "nLHEPart", 0);
  LHEPart_firstMotherIdx.Init(tree, fReader, "LHEPart_firstMotherIdx");
  LHEPart_lastMotherIdx.Init(tree, fReader, "LHEPart_lastMotherIdx");
  LHEPart_pdgId.Init(tree, fReader, "LHEPart_pdgId");
  LHEPart_status.Init(tree, fReader, "LHEPart_status");
  LHEPart_spin.Init(tree, fReader, "LHEPart_spin");
  LHEPart_pt.Init(tree, fReader, "LHEPart_pt");
  LHEPart_eta.Init(tree, fReader, "LHEPart_eta");
  LHEPart_phi.Init(tree, fReader, "LHEPart_phi");
  LHEPart_mass.Init(tree, fReader, "LHEPart_mass");
  LHEPart_incomingpz.Init(tree, fReader, "LHEPart_incomingpz");
  
}

bool nanoAna::Notify()
{
  return true;
}

#endif // nanoAna_Init_h
