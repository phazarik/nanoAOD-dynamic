void nanoAna::SortPt(vector<Particle> &objarr)
{
  // Sort an object array in the decreasing order of pT
  for(int i=0; i<(int)objarr.size()-1; i++){
    for(int j=i+1; j<(int)objarr.size(); j++){
      if( objarr[i].v.Pt() < objarr[j].v.Pt() ) swap(objarr.at(i),objarr.at(j));
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
//                          USER DEFINED FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

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
}

//------------------------
// DNN specific functions:
//------------------------
vector<float> nanoAna::loadScalingParameters(const char* filename) 
{
  vector<float> params;
  ifstream file(filename);    
  if (!file.is_open()) {
    cerr << "ERROR: Could not open scaling parameter file: " << filename << endl;
    return params;
  }
  float val;
  while(file >> val)  params.push_back(val);
  file.close();
  return params;
}
float nanoAna::evaluateDNN(Ort::Session* session, 
                  std::vector<float> input_vars, 
                  const std::vector<float>& scale_min, 
                  const std::vector<float>& scale_max,
                  const char* input_name,
                  const char* output_name)
{
  int debug_evt = 500;
  if(nEvtTotal==debug_evt) cout << "[DEBUG] test event: "<<debug_evt<<endl;

  // 1. Normalize data between [-1, 1] (same as training)
  for(size_t i = 0; i < input_vars.size(); i++) {
    float raw = input_vars[i];
    float diff = scale_max[i] - scale_min[i];
    if(diff != 0) input_vars[i] = 2.0 * ((raw - scale_min[i]) / diff) - 1.0;
    else          input_vars[i] = 0.0; // Handle constant features
    if(nEvtTotal == debug_evt)
      cout << "[DEBUG] Feature " << i << ": Raw=" << raw << " | Scaled=" << input_vars[i] << endl;
  }
      
  // 2. Setup memory and define tensor shape: [batch_size, number_of_features]
  Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  std::vector<int64_t> input_shape = {1, static_cast<int64_t>(input_vars.size())}; 

  // 3. Convert the C++ vector into an ONNX tensor object
  Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
							    memory_info,         // CPU memory allocation
							    input_vars.data(),   // Pointer to scaled float data
							    input_vars.size(),   // Total number of data points
							    input_shape.data(),  // Pointer to the shape array
							    input_shape.size()); // Number of dimensions (2)

  // 4. Define the exact node names baked into the ONNX model
  const char* input_names[]  = {input_name};  // Where the data goes in
  const char* output_names[] = {output_name}; // Where the prediction comes out

  // 5. Fire the inference engine
  auto output_tensors = session->Run(
				     Ort::RunOptions{nullptr}, // Default run options
				     input_names,              // Target input layer, depends on the model type
				     &input_tensor,            // Our formatted data
				     1,                        // Number of input tensors being passed
				     output_names,             // Target output layer, depends on the model type
				     1);                       // Number of output tensors are expected back

  // 6. Extract the raw prediction score and return it
  float* floatarr = output_tensors.front().GetTensorMutableData<float>();
  float final_score = floatarr[0];
  if(nEvtTotal==debug_evt) cout << "[DEBUG] >>> Model Output Score: " << final_score << endl;
  return final_score;
}
