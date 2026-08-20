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
  
}
