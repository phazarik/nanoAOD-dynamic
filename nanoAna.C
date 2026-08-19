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

#include "nanoAna.h"
#include "Init.h"
#include <TH2.h>
#include <TStyle.h>

//Standard headers:
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>

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
  
  nEvtTotal++;
  
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

  //--------------------------------------------------------------------------
  //                                 DEBUG
  //--------------------------------------------------------------------------

  // Example usage of branches:
  for (int i = 0; i < (int)*nMuon; i++) {
    if (Muon_pt[i] > 5 && Muon_mediumId[i]) {
      cout<<"Found muon with pT: "<<Muon_pt[i]<<endl;
    }
  }

  // Missing/renamed variables fall back to defaults:
  // 1. Missing LHE branches: In data, as well as in QCD samples, the following returns 1.0 safely.
  float weight = *LHEWeight_originalXWGTUP;
  cout<<"LHE weight for this event = "<<weight<<endl;

  // Renamed Rho_* branches: One of the following branches always has a default value of zero.
  float rho = *fixedGridRhoFastjetAll;
  cout<<"Rho for this event = "<<rho<<endl;
  //--------------------------------------------------------------------------
  
  return true;
}

//////////////////////////////////////////////////////////////////////////////
//                          USER DEFINED FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

void nanoAna::BookHistograms()
{
  //The histograms are booked here.
  //Binning etc are done here.
  //These histograms are stored in the hst_<process name>.root file in the same order.

  h.hist.resize(2);
  h.hist[0] = new TH1F("leading_muon_pT", "leading muon pT", 200, 0, 200);
  h.hist[1] = new TH1F("ptr_fixedGridRhoFastjetAll", "ptr_fixedGridRhoFastjetAll", 200, 0, 200);
  //Example : new TH1F ("hst_name", "hst title", total bins, initial x, final x);

  for(int i = 0; i < (int)h.hist.size(); i++) h.hist[i]->Sumw2();
  
}

void nanoAna::SortPt(vector<Particle> &objarr)
{
  // Sort an object array in the decreasing order of pT
  for(int i=0; i<(int)objarr.size()-1; i++){
    for(int j=i+1; j<(int)objarr.size(); j++){
      if( objarr[i].v.Pt() < objarr[j].v.Pt() ) swap(objarr.at(i),objarr.at(j));
    }
  }
}

float nanoAna::DeltaPhi(float phi1, float phi2)
{
  //The correct deltaPhi falls in the interval [0 , pi]
  phi1 = TVector2::Phi_0_2pi(phi1);
  phi2 = TVector2::Phi_0_2pi(phi2);
  float dphi = fabs(phi1 - phi2);
  if(dphi>TMath::Pi()) dphi = 2*TMath::Pi() - dphi;
  return dphi;
}

float nanoAna::TransvMass(float E_lep, float MET, float dphi)
{
  //The inputs are the Energy of the lepton, MET and dPhi between the lepton and MET
  float mT = sqrt(2* E_lep * MET *(1-cos(dphi)));
  return mT;
}

/*
int nanoAna::GenMother(int ind, int mom_ind)
{
  int p_id = GenPart_pdgId[ind];
  int m_id = GenPart_pdgId[mom_ind];
  while(p_id==m_id){
    ind = mom_ind;
    mom_ind = GenPart_genPartIdxMother[ind];
    p_id = GenPart_pdgId[ind];
    m_id = GenPart_pdgId[mom_ind];
  }
  return m_id;
  }*/
