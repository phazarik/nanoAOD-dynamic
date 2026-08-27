#include <TSystem.h>
#include <TROOT.h>
#include <TStopwatch.h>
#include <iostream>
#include <TString.h>
using namespace std;
string fmt(double t);

void compile_and_run(
    TString infile  = "inputs/DY_NanoAODv15.root",
    TString outfile = "outputs/hst_test.root",
    TString era     = "2024",
    TString sample  = "DYJetsToLL"
)
{
    TString scriptPath = gSystem->DirName(__FILE__);

    //ANSI color codes:
    TString RED    = "\033[31m";
    TString RESET  = "\033[0m";

    // Explicitly load necessary ROOT libraries before compiling
    cout << "\n>> Loading libraries ..." << endl;
    gSystem->Load("libTree");
    gSystem->Load("libTreePlayer");
    gSystem->Load("libPhysics");
    gSystem->Load("libHist");

    // Setup DNN libraries with RPATH and include paths
    TString onnxdir = scriptPath + "/onnxruntime-linux-x64-1.24.4";
    gSystem->AddIncludePath(Form("-I%s/include", onnxdir.Data()));

    // Bake the library directory directly into the compiled object via rpath
    // Use the standard .so link in REANA, since the python wrapper runJob.py rebuilds it
    gSystem->AddLinkedLibs(Form("-Wl,-rpath,%s/lib %s/lib/libonnxruntime.so", onnxdir.Data(), onnxdir.Data()));
    if (gSystem->Load(onnxdir + "/lib/libonnxruntime.so") < 0) {
      cerr << RED << "[ERROR] Could not find libonnxruntime.so!" << RESET << endl;
      return;
    }

    // -------------------------------------------------------------------------
    // ROOT ACLiC ISOLATION (Fix for HTCondor parallel job collisions)
    // -------------------------------------------------------------------------
    TString buildDir = Form("/tmp/aclic_%d", gSystem->GetPid());
    gSystem->SetBuildDir(buildDir.Data(), true); // true = create if it doesn't exist

    // Compile the source code:
    TStopwatch time_compile, time_run, time_total;
    time_compile.Start(); time_total.Start();
    cout << ">> Compiling the source code in " << buildDir << " ..." << endl;
    
    // Changed "kO" (Overwrite) to "k" (Keep/compile if changed) to save time
    // if ROOT decides to reuse the dictionary in the same session.
    if (gSystem->CompileMacro("nanoAna.C", "k") == 0) {
      cerr << RED << "[ERROR] Compilation of nanoAna.C failed!" << RESET << endl;
        return;
    }
    time_compile.Stop();
    cout << ">> Compilation time: \033[34m" << fmt(time_compile.RealTime()) <<"\033[0m"<< endl;

    //  Run runOnce.C:
    time_run.Start();
    cout << ">> Running runOnce.C ..." << endl;

    // Pass string arguments safely using quotes inside Form
    gROOT->ProcessLine(Form(".x runOnce.C(\"%s\", \"%s\", \"%s\", \"%s\")",
                            infile.Data(), outfile.Data(), era.Data(), sample.Data()));

    time_run.Stop();
    time_total.Stop();

    cout << ">> Total runtime for this sample: \033[34;1m" << fmt(time_total.RealTime()) <<"\033[0m"<< endl;
    cout << ">> File created: \033[33;1m"<< outfile <<"\033[0m\n"<< endl;
}

string fmt(double t) {
    int h = t / 3600;
    int m = ((int)t % 3600) / 60;
    int s = (int)t % 60;
    char buf[16];
    sprintf(buf, "%02d:%02d:%02d", h, m, s);
    return string(buf);
}
