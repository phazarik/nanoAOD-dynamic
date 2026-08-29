#define nanoAna_cxx

//----------------------------------------------------------------------------
// The following methods are defined in this file:
//    Begin():        called every time a loop on the tree starts,
//                    a convenient place to create your histograms.
//    SlaveBegin():   called after Begin(), when on PROOF called only on the
//                    slave servers.
//    Process():      called for each event, in this function you decide what
//                    to read and fill your histograms.
//    SlaveTerminate: called at the end of the loop on the tree, when on PROOF
//                    called only on the slave servers.
//    Terminate():    called at the end of the loop on the tree,
//                    a convenient place to draw/fit your histograms.
//----------------------------------------------------------------------------

#include <TH2.h>
#include <TStyle.h>

//Standard headers:
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>

//Headers for this setup
#include "nanoAna.h" //Loads the analysis class, ONNX and the dynamic readers 
#include "headers/Init.h"
#include "headers/BookHistograms.h"
#include "headers/CustomFunctions.h"
#include "headers/loadGoldenJSONs.h"

void nanoAna::Begin(TTree * /*tree*/)
{
  TString option = GetOption();
}
void nanoAna::SlaveBegin(TTree *tree)
{
  //------------------------------------------------------------------
  // Note on _era (TString):
  // Make sure to use the same naming convention for _era.
  // It is used to make year/campaign dependent choices downstream.
  // The following options are recommended.
  // 2016-preVFP, 2016-postVFP, 2017, 2018,
  // 2022-preEE, 2022-postEE, 2023-preBPix, 2023-postBPix
  // new: 2024, 2025
  //------------------------------------------------------------------
  
  TString option = GetOption();

  // Set year and b-tagging working point from _era:
  // b-tagging threshold source: https://btv-wiki.docs.cern.ch/ScaleFactors/
  _btaggingWP = -1; _year = -1;
  if      (_era.Contains("2016-pre"))  {_btaggingWP = 0.2598; _year = 2016;}
  else if (_era.Contains("2016-post")) {_btaggingWP = 0.2489; _year = 2016;}
  else if (_era.Contains("2017"))      {_btaggingWP = 0.3040; _year = 2017;}
  else if (_era.Contains("2018"))      {_btaggingWP = 0.2783; _year = 2018;}
  else if (_era.Contains("22-pre"))    {_btaggingWP = 0.3086; _year = 2022;}
  else if (_era.Contains("22-post"))   {_btaggingWP = 0.3196; _year = 2022;}
  else if (_era.Contains("23-pre"))    {_btaggingWP = 0.2431; _year = 2023;}
  else if (_era.Contains("23-post"))   {_btaggingWP = 0.2435; _year = 2023;}
  else if (_era.Contains("24"))        {_year = 2024;}
  if(_year < 0 ) cout<<"\033[31m[ERROR] Could not set year. Provide correct _era.\033[0m"<<endl;
  if(_btaggingWP < 0 ) cout<<"\033[31m[ERROR] Could not set b-tagging WP. Provide correct _era.\033[0m"<<endl;

  cout<<"\033[33m"<<endl;
  cout<<"Input parameters:"<<endl;
  cout<<"  - Data   : "<<_data<< " (0=MC, 1=Data)" <<endl;
  cout<<"  - Sample : "<<_sample<<endl;
  cout<<"  - Era    : "<<_era<<endl;
  cout<<"  - b-WP   : "<<_btaggingWP<<endl;
  cout<<"  - Year   : "<<_year<<endl;
  cout<<"\033[0m"<<endl;

  // Initialization of the counters:
  time(&start);
  nEvtTotal      = 0;
  nEvtRan        = 0;
  nEvtTrigger    = 0;
  nEvtPass       = 0;
  nThrown        = 0;
  genEventSumW   = 0;

  // Constants:
  nEvtGen = tree->GetEntries(); 
  test_event = 100;
  cout << ">> Total tree entries: " << nEvtGen << endl;

  // Load Golden JSON data:
  goldenJSONdata = loadGoldenJSON();
  
  // Create a TFile to write on.
  // Call any other function that does any kind of initialization of objects/variables.
  _HstFile = new TFile(_HstFileName,"recreate");
  BookHistograms();

  // ----------------------------------------------------------------------------------
  // Loading DNNs and scaling parameters:

  // One environment to rule them all
  ort_env = new Ort::Env(ORT_LOGGING_LEVEL_WARNING, "MultiDNN_Inference");
  cout << ">> Initialized ONNX environment." << endl;
  session_options.SetIntraOpNumThreads(1);

  //Load all DNNs and scaling parameters here
  loadAllDNNs();
  //-----------------------------------------------------------------------------------

  // Header for progress on screen:
  cout << "\n"
     << right << setw(8) << "Progress"
     << right << setw(12) << "nEvents"
     << right << setw(8) << "Time" << endl;
}
void nanoAna::SlaveTerminate()
{
  // Display summary:
  float goodevtfrac = ((float)nEvtRan)/((float)nEvtTotal);
  float trigevtfrac = ((float)nEvtTrigger)/((float)nEvtTotal);
  float notgoldenevtfrac  = ((float)nThrown)/((float)nEvtTotal);

  cout<<"---------------------------------------------"<<endl;
  cout<<"Summary:"<<fixed << setprecision(6)<<endl;
  cout<<"nEvtTotal   = "<<nEvtTotal<<endl;
  cout<<"nEvtRan     = "<<nEvtRan<<" ("<<goodevtfrac*100<<" %)"<<endl;
  cout<<"nEvtTrigger = "<<nEvtTrigger<<" ("<<trigevtfrac*100<<" %)"<<endl;
  if(_data==1) cout<<"nEvents not in golden json = "<<nThrown<<" ("<<notgoldenevtfrac*100<<" %)"<<endl;
  if(_data==0) cout<<"Sum of generator weights = "<<fixed<<setprecision(2)<<genEventSumW<<endl;
  cout<<"---------------------------------------------"<<endl;

  /*
  //The following lines are written on the sum_<process name>.txt file
  ofstream fout(_SumFileName);
  fout<<"Total events ran  = "<<nEvtTotal<<endl;
  fout<<"Total good events = "<<nEvtRan<<endl;
  fout<<"Total HLT events = "<<nEvtTrigger<<endl;*/

  //----------------------------------------------------------------
  //Storing event information in a histogram:
  // Navigate to the file's directory in ROOT memory before creating histograms
  _HstFile->cd();
  TH1D *hCount = new TH1D("hCount", "hCount;;", 3, 0.5, 3.5);
  hCount->SetBinContent(1,nEvtTotal); //-> luminosity calculation
  hCount->SetBinContent(2,nEvtRan);
  hCount->SetBinContent(3,nEvtTrigger);
  hCount->GetXaxis()->SetBinLabel(1,"nEvtGen");
  hCount->GetXaxis()->SetBinLabel(2,"nEvtRan");
  hCount->GetXaxis()->SetBinLabel(3,"nEvtTrigger");

  TH1D *hWt = new TH1D("hWt", "hWt;;", 1, 0, 1);
  hWt->SetBinContent(1,genEventSumW);
  hWt->GetXaxis()->SetBinLabel(1,"genEventSumW"); //-> gen-level weights
  //----------------------------------------------------------------

  // Close the file AFTER writing everything
  _HstFile->Write();
  _HstFile->Close();

  // Print time taken:
  time(&end);
  int time_taken = int(end - start);
  int H = time_taken / 3600;
  int M = (time_taken % 3600) / 60;
  int S = time_taken % 60;
  cout << "\n>> Process() time: \033[34m"<< Form("%02d:%02d:%02d", H, M, S)<< "\033[0m" << endl;
  
}
void nanoAna::Terminate()
{

}

//============================================================================
bool nanoAna::Process(Long64_t entry)
{
  // if _era/_data contains invalid values, use Abort() and display an error message in red.
  
  nEvtTotal++; //Counts the total number of events processed from the file
  
  // The following fReader can access both data and MC branches.
  // Any branch that is not in the ROOT file returns a default value.
  // (For example, gen-level branches in data.)
  // MUST load the entry first before accessing ANY branch values!
  fReader.SetLocalEntry(entry);

  // Count the sum of generator weights for later use.
  if(_data==0) genEventSumW += (float) *Generator_weight;

  // Setting verbosity:
  if (nEvtTotal % _verbosity == 0) {
    time(&buffer);
    double time_buff = double(buffer-start);
    double frac = (double)nEvtTotal / nEvtGen * 100.0;
    string progress = (ostringstream() << fixed << setprecision(2) << frac << "%").str();
    cout << right << setw(8) << progress
	 << right << setw(12) << nEvtTotal
	 << right << setw(8) << fixed << setprecision(0) << time_buff
	 << setprecision(6) << endl;
  }

  //-----------------------------------------------------------------------------------------------
  // MET filters:
  // 1. Flag_goodVertices = Primary vertex filter.
  // 2. Flag_globalSuperTightHalo2016Filter = Removes events affected by beam halo muons.
  // 3. Flag_EcalDeadCellTriggerPrimitiveFilter = Removes events where dead ECAL cells affect trigger primitives.   
  // 4. Flag_BadPFMuonFilter = Removes events containing misreconstructed PF muons.
  // 5. Flag_BadPFMuonDzFilter = Additional filter for PF muons with large dz.
  // 6. Flag_HBHENoiseFilter = Rejects events with noise from the HCAL.
  // 7. Flag_HBHENoiseIsoFilter = Removes isolated HCAL noise not caught by the standard filter.
  // 8. Flag_eeBadScFilter  = Removes events with problematic superclusters in ECAL endcap.
  // 9. Flag_ecalBadCalibFilter = Rejects events with problematic ECAL crystals due to bad calibrations.
  //
  // Things to keep in mind:
  // - Flag_HBHENoiseFilter and Flag_HBHENoiseIsoFilter can be replaced with Flag_hfNoisyHitsFilter.
  // - Do not use the default Flag_ecalBadCalibFilter for 2022 and 2023 (NanoAODv12).
  // - Flag_BadChargedCandidateFilter is not recommended.
  // Reference: https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2
  //-----------------------------------------------------------------------------------------------

  GoodEvt =
    *Flag_goodVertices &&
    *Flag_globalSuperTightHalo2016Filter &&
    *Flag_EcalDeadCellTriggerPrimitiveFilter &&
    *Flag_BadPFMuonFilter &&
    *Flag_BadPFMuonDzFilter &&
    *Flag_hfNoisyHitsFilter && // both 6 and 7
    *Flag_eeBadScFilter &&
    (_year<2019 ? *Flag_ecalBadCalibFilter: 1);
  
  if(GoodEvt){
    nEvtRan++;  //Total number of good events

    bool trigger = true; //default, always true for MC
      
    if(_data==1){
      // Construct the trigger logic using the HLT_* branches and the _.
      // It may be different for sample with names "Muon"/"Electron/" or the year.
      // The following example uses muon and electron trigger paths and avoids overlap of events in datasets.
      // Note: Dafault not set: code will break if the HLT path is not found in the file (by design).
      bool trigger_mu  = true;  bool trigger_ele = true;
      if     (_year == 2016){trigger_mu = *HLT_IsoMu24; trigger_ele=*HLT_Ele27_WPTight_Gsf;}
      else if(_year == 2017){trigger_mu = *HLT_IsoMu27; trigger_ele=*HLT_Ele32_WPTight_Gsf_L1DoubleEG;}
      else if(_year == 2018){trigger_mu = *HLT_IsoMu24; trigger_ele=*HLT_Ele32_WPTight_Gsf;}
      else if(_year == 2022){trigger_mu = *HLT_IsoMu24; trigger_ele=*HLT_Ele32_WPTight_Gsf;}
      else if(_year == 2023){trigger_mu = *HLT_IsoMu24; trigger_ele=*HLT_Ele32_WPTight_Gsf;}
      else cout<<"\033[31m[ERROR] fix _era. Trigger path not chosen for year = "<<_year<<"\033[0m"<<endl;
      bool overlapping_events = trigger_mu && trigger_ele;
      
      //Applying trigger on data: (logic: overlapping events are taken from the muon dataset)
      if(_sample.Contains("Muon"))   trigger = trigger_mu;                 //includes overlap.
      if(_sample.Contains("EGamma")) trigger = trigger_ele && !trigger_mu; //excludes overlap.
      if(_sample.Contains("SingleElectron")) trigger = trigger_ele && !trigger_mu; //excludes overlap.
	
      //Throw awaying bad data that are not included in the GoldenJSON:
      int runno = (int)*run;
      int lumisection = (int)*luminosityBlock;
      bool golden_event = checkGoldenJSON(runno, lumisection);
      if(!golden_event){
	nThrown++;
	trigger = false;
      }
    }

    if (trigger){
      nEvtTrigger++;
    
      //The analysis is done for the good events which pass the trigger.

      /////////////////////////////////////
      //  Construction of the arrays:
      /////////////////////////////////////
    
      //Reco Muon array :
      int nmu = 0;                         // This counts the number of muons in each event.
      RecoMu.clear();                      // Make sure that the array is empty before filling it up.
      for(int i=0; i<(int)*nMuon; i++){
	// This loop runs over all the muon candidates. Some of them will pass our selection criteria.
	// These will be stored in the RecoMu array.
	Particle temp;                       // 'temp' is the i-th candidate.
	temp.v.SetPtEtaPhiM(Muon_pt[i],Muon_eta[i],Muon_phi[i],0.105);
	temp.charge = Muon_charge[i];
	temp.id = -13*Muon_charge[i];      //pdgID for mu- = 13, pdgID for mu+ = -13  
	temp.ind = i;

	//These are the flags the 'temp' object has to pass.
	bool passCuts = temp.v.Pt()>15 && fabs(temp.v.Eta())<2.4 && Muon_mediumId[i];
	passCuts = passCuts && Muon_pfRelIso04_all[i]<0.15;
	passCuts = passCuts && fabs(Muon_dxy[i])<0.05 && fabs(Muon_dz[i])<0.1;
      
	if(passCuts){
	  RecoMu.push_back(temp);          // If 'temp' satisfies all the conditions, it is pushed back into RecoMu
	  nmu++;                           // Everytime a 'temp' passes the flags, this counter increases by one.
	}
      }                                    // This 'for' loop has created a RecoMu array.
    
      SortPt(RecoMu);                      //The RecoMu array has been organised in the decreasing order of pT.


      //Reco jets:
      int njet = 0;
      RecoJet.clear();
      for(int i=0; i<(int)*nJet; i++){
	Particle temp;
	temp.v.SetPtEtaPhiM(Jet_pt[i], Jet_eta[i], Jet_phi[i], Jet_mass[i]);

	temp.ind = i;
	bool passCuts = temp.v.Pt()>50 && fabs(temp.v.Eta())<2.4;
	if(_year<2024) passCuts = passCuts && (int)Jet_jetId[i]>=2; //Only used in NanoAODv12 and below
	if(passCuts){
	  RecoJet.push_back(temp);
	  njet++;
	}
      }
      SortPt(RecoJet);
    
      //Other arrays, such as RecoEle, GenMu, GenEle can be constructed here.






      /////////////////////////////////////
      //          Analysis block
      /////////////////////////////////////

      // Plotting the leading muon pT in each event.
      if((int)RecoMu.size()>0){             //If there is atleast one muon in the event
	h.hist[0] -> Fill(RecoMu.at(0).v.Pt());
      }
    
      // Acessing variables that have different name across Run2/Run3, or unavailable:
      // LHEweights: unavailable in QCD samples, default is 1.
      // rho: resisual energy desnity : named as Rho_fixedGridRhoFastjetAll in Run3.
      // Both are unavailable in data.
      float LHEweight = *LHEWeight_originalXWGTUP;
      float rho = *fixedGridRhoFastjetAll;         
      h.hist[1]->Fill(*fixedGridRhoFastjetAll);

      // --------------- DNN section -----------------
      // The example model is trained to classify VLL against DY in 2L phase space.
      // It requires input variables that are defined in 2L events.
      // In this example, we will strick to exclusively 2-mu events.
      //
      // Files needed:
      //  - model_DY-vs-VLLD_Run3_Feb19.onnx
      //  - scaling_parameters_min.txt
      //  - scaling_parameters_max.txt
      //
      // input variables (in order): dilep_dphi, dilep_eta, dilep_ptratio, HT, LT, metpt

      float score_dy = -99; //dummy value to keep the invalid events
    
      if((int)RecoMu.size()==2){ //Pick the right phase space.
      
	// Prepare the training variables and create arrays for the DNNs.
	float dilep_dphi = DeltaPhi(RecoMu.at(0).v.Phi(), RecoMu.at(1).v.Phi());
	float dilep_eta = (RecoMu.at(0).v + RecoMu.at(1).v).Eta();
	float dilep_ptratio = RecoMu.at(1).v.Pt()/RecoMu.at(0).v.Pt();
	float HT = 0; for(int i=0; i<(int)RecoJet.size(); i++) HT += RecoJet.at(i).v.Pt();
	float LT = RecoMu.at(0).v.Pt() + RecoMu.at(1).v.Pt();
	float metpt = *PuppiMET_pt;

	// Plot input variables:
	h.dnn[0]->Fill(dilep_dphi);
	h.dnn[1]->Fill(dilep_eta);
	h.dnn[2]->Fill(dilep_ptratio);
	h.dnn[3]->Fill(HT);
	h.dnn[4]->Fill(LT);
	h.dnn[5]->Fill(metpt);

	// Evaluate score 1:
	vector<float> invar_dy = {dilep_dphi, dilep_eta, dilep_ptratio, HT, LT, metpt}; //Maintain order
	score_dy = evaluateDNN(session_dy, invar_dy, scale_min_dy, scale_max_dy, "input", "keras_tensor_3");

	// Note: The input and output node names ("input", "keras_tensor_3") are
	// specific to how this model was saved. Verify this via https://netron.app/.

	// Similarly evaluate for other scores.

      }
    
      h.hist[2]->Fill(score_dy); // Includes dummy values as well.*/
      // ---------------------------------------------
    
      //########### ANALYSIS ENDS HERE ##############
    }// trigger
  }//GoodEvt
  
  return true;
}



