void nanoAna::BookHistograms()
{
  //The histograms are booked here.
  //Binning etc are done here.
  //These histograms are stored in the hst_<process name>.root file in the same order.

  h.hist.resize(3);
  //Example : new TH1F ("hst_name", "hst title", total bins, initial x, final x);
  h.hist[0] = new TH1F("leading_muon_pT", "leading muon pT", 200, 0, 200);
  h.hist[1] = new TH1F("fixedGridRhoFastjetAll", "fixedGridRhoFastjetAll", 200, 0, 200);
  h.hist[2] = new TH1F("dnn_score_1", "dnn_score_1", 100, 0, 1);
  for(int i = 0; i < (int)h.hist.size(); i++) h.hist[i]->Sumw2();

  h.dnn.resize(6);
  h.dnn[0] = new TH1F("var0_dilep_dphi", "dilep_dphi", 100, 0, 6);
  h.dnn[1] = new TH1F("var1_dilep_eta",  "dilep_eta",  100, -10, 10);
  h.dnn[2] = new TH1F("var2_dilep_ptratio", "dilep_ptratio", 100, 0, 1);
  vector<float> GEVbins = {0, 50, 100, 200, 300, 400, 500};
  h.dnn[3] = new TH1F("var3_HT",  "HT",  GEVbins.size()-1, &GEVbins[0]);
  h.dnn[4] = new TH1F("var4_LT",  "LT",  GEVbins.size()-1, &GEVbins[0]);
  h.dnn[5] = new TH1F("var5_MET", "MET", GEVbins.size()-1, &GEVbins[0]);
  for(int i = 0; i < (int)h.dnn.size(); i++) h.dnn[i]->Sumw2();
  
}
