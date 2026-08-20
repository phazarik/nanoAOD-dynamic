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

void nanoAna::Begin(TTree * /*tree*/)
{
  TString option = GetOption();
}
void nanoAna::SlaveBegin(TTree *tree)
{
  // Before SlaveBegin runs, make sure to provide the following in ana.C
  // _data using SetData();
  // _era using SetEra() -> This will automatically set _year;

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

  // Set _year from _era:
  if     (_era.Contains("16")) _year = 2016;
  else if(_era.Contains("17")) _year = 2017;
  else if(_era.Contains("18")) _year = 2018;
  else if(_era.Contains("22")) _year = 2022;
  else if(_era.Contains("23")) _year = 2023;
  else if(_era.Contains("24")) _year = 2024;
  else cout<<"\033[31m[ERROR] nanoAna.C: Provide correct _era.\033[0m"<<endl;

  cout<<"Input parameters:"<<endl;
  cout<<"Data = "<< _data << " (0=MC, 1=Data)" <<endl;
  cout<<"Year = "<<_year<<endl; 

  // Initialization of the counters:
  time(&start);
  nEvtRan        = 0;
  nEvtTotal      = 0;

  // Constants:
  nEvtGen = tree->GetEntries(); 
  test_event = 100;
  
  // Create a TFile to write on.
  // Call any other function that does any kind of initialization of objects/variables.
  _HstFile = new TFile(_HstFileName,"recreate");
  BookHistograms();

  // ----------------------------------------------------------------------------------
  // Loading DNNs and scaling parameters:

  // One environment to rule them all
  ort_env = new Ort::Env(ORT_LOGGING_LEVEL_WARNING, "MultiDNN_Inference");
  Ort::SessionOptions session_options;
  session_options.SetIntraOpNumThreads(1);
    
  // Load model for DY-vs-VLLD:
  TString path_dy = "trained_models/DY-vs-VLLD_Run3_Feb19/";
  session_dy = new Ort::Session(*ort_env, (path_dy + "model_DY-vs-VLLD_Run3_Feb19.onnx").Data(), session_options);
  scale_min_dy = loadScalingParameters((path_dy + "scaling_parameters_min.txt").Data());
  scale_max_dy = loadScalingParameters((path_dy + "scaling_parameters_max.txt").Data());

  // Similarly load other models ...
  //-----------------------------------------------------------------------------------

  // Header for progress on screen:
  cout << "\n"
     << right << setw(8) << "Progress"
     << right << setw(12) << "nEvents"
     << right << setw(8) << "Time" << endl;
}
void nanoAna::SlaveTerminate()
{
  _HstFile->Write();
  _HstFile->Close();

  // Display summary:
  cout<<"----------------------------------------"<<endl;
  cout<<"Total events ran  = "<<nEvtTotal<<endl;
  cout<<"Total good events = "<<nEvtRan<<endl;

  //The following lines are written on the sum_<process name>.txt file
  ofstream fout(_SumFileName);
  fout<<"Total events ran  = "<<nEvtTotal<<endl;
  fout<<"Total good events = "<<nEvtRan<<endl;

  // Print time taken:
  time(&end);
  double time_taken = double(end - start);
  cout<<"\n\033[34mTime taken to process = " << (int)time_taken << " seconds.\033[0m"<< endl;
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
  fReader.SetLocalEntry(entry);

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

  //The following flags throws away some trash events
  GoodEvt2018 = true; //(_year==2018 ? *Flag_goodVertices && *Flag_globalSuperTightHalo2016Filter && *Flag_HBHENoiseFilter && *Flag_HBHENoiseIsoFilter && *Flag_EcalDeadCellTriggerPrimitiveFilter && *Flag_BadPFMuonFilter && (_data ? *Flag_eeBadScFilter : 1) : 1);
  GoodEvt2017 = true; //(_year==2017 ? *Flag_goodVertices && *Flag_globalSuperTightHalo2016Filter && *Flag_HBHENoiseFilter && *Flag_HBHENoiseIsoFilter && *Flag_EcalDeadCellTriggerPrimitiveFilter && *Flag_BadPFMuonFilter && (_data ? *Flag_eeBadScFilter : 1) : 1);
  GoodEvt2016 = true; //(_year==2016 ? *Flag_goodVertices && *Flag_globalSuperTightHalo2016Filter && *Flag_HBHENoiseFilter && *Flag_HBHENoiseIsoFilter && *Flag_EcalDeadCellTriggerPrimitiveFilter && *Flag_BadPFMuonFilter && (_data ? *Flag_eeBadScFilter : 1) : 1);
  
  GoodEvt = GoodEvt2018 && GoodEvt2017 && GoodEvt2016;
  
  if(GoodEvt){

  }
  if(GoodEvt){

    nEvtRan++;  //Total number of good events
 
    //The analysis is done for these good events.

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
  }//GoodEvt
  
  return true;
}



