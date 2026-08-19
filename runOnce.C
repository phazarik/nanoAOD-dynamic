#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <iostream>
#include <TString.h>
#include <string>
using namespace std;

void runOnce(
	 TString infile  = "inputs/DY_NanoAODv15.root",
	 TString outfile = "outputs/hst_test.root",
	 TString era     = "2018_UL",
	 TString sample  = "DYJetsToLL"
)
{
  // Note: Keep all the arguments as TString.
  // It becomes easy to execute this from bash in -q -b -l mode using strings.

  gErrorIgnoreLevel = kError;  // Suppress warnings below 'Error' level

  //ANSI color codes:
  TString RED    = "\033[31m";
  TString YELLOW = "\033[33m";
  TString BOLD   = "\033[1m";
  TString DIM    = "\033[2m";
  TString RESET  = "\033[0m";
  
  // Set of variables:
  TChain *chain = new TChain("Events");
  const char *rootfile = outfile;
  const char *sumfile  = outfile+".txt";
  nanoAna m_selec;

  // Note: if sample contains "Muon" or "EGamma" or "Electron",
  // Set data to 1, else keep it zero.
  int data = 0;
  if (sample.Contains("Muon") ||
      sample.Contains("EGamma") ||
      sample.Contains("Electron")) data = 1;
  
  //cout << "\033[33mInput  : " << infile <<"\033[0m"<< endl;
  //cout << "\033[33mOutput : " << outfile <<"\033[0m"<< endl;
  
  // Load the input file:
  chain->Add(infile);

  // Some debug statements:
  if(chain->GetListOfFiles()->GetEntries() == 0){
    cout<<RED<<"[ERROR] No input files found or they could not be opened!"<<RESET<<endl;
    return;
  }
  cout<<DIM<<"\nThe following input file(s) are loaded successfully:" << endl;
  for (int i = 0; i < chain->GetListOfFiles()->GetEntries(); i++)
    cout << "  - " << chain->GetListOfFiles()->At(i)->GetTitle() << endl;
  cout<<"\nThe following output files are being created:"<<endl;
  cout<<"Hist file: "<<rootfile<<endl;
  cout<<"Text file: "<<sumfile<<RESET<<endl;

  //Set the parameters on the analyzer:
  m_selec.SetHstFileName(rootfile);
  m_selec.SetSumFileName(sumfile);
  m_selec.SetData(data);
  m_selec.SetEra(era);
  m_selec.SetVerbose(1000); //After how many events do you want a report?
  
  // Call the process function which runs the code.
  cout<<YELLOW<<BOLD<<"\nRunning the event processor ... \n"<<RESET;
  chain->Process(&m_selec);
  cout<<"\nDone!"<<endl;
}
