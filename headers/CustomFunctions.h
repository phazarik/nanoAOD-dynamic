//////////////////////////////////////////////////////////////////////////////
//                          USER DEFINED FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

// 4-vector related quantities/actions:
void nanoAna::SortPt(vector<Particle> &objarr){
  // Sort an object array in the decreasing order of pT
  for(int i=0; i<(int)objarr.size()-1; i++){
    for(int j=i+1; j<(int)objarr.size(); j++){
      if( objarr[i].v.Pt() < objarr[j].v.Pt() ) swap(objarr.at(i),objarr.at(j));
    }
  }
}
float nanoAna::DeltaPhi(float phi1, float phi2){
  //The correct deltaPhi falls in the interval [0 , pi]
  phi1 = TVector2::Phi_0_2pi(phi1);
  phi2 = TVector2::Phi_0_2pi(phi2);
  float dphi = fabs(phi1 - phi2);
  if(dphi>TMath::Pi()) dphi = 2*TMath::Pi() - dphi;
  return dphi;
}
float nanoAna::TransvMass(const TLorentzVector& obj){
  float dphi = DeltaPhi(obj.Phi(), metphi);
  float mT = sqrt(2 * obj.E() * metpt * (1 - cos(dphi)));
  return mT;
}

// Generator level:
int nanoAna::MotherID(int partindex, int momindex){
  int parid =GenPart_pdgId[partindex];
  int momid = GenPart_pdgId[momindex];
  while(parid==momid){
    partindex=momindex;
    momindex=GenPart_genPartIdxMother[momindex];
    parid =GenPart_pdgId[partindex];
    momid = GenPart_pdgId[momindex];
  }
  return momid;
}
bool nanoAna::isMatchingWithGen(Particle reco, vector<Particle> gencollection){
  float dR_min = 1000; //Initialize minimun dR with some large value.
  for(int i=0; i<(int)gencollection.size(); i++){
    Particle gen = gencollection.at(i);
    if(fabs(reco.id) == fabs(gen.id)){ //Compare reco-gen only if their IDs match. (flavor and charge)
      float dR = reco.v.DeltaR(gen.v);
      if(dR < dR_min) dR_min = dR; // Minimization logic
    }
  }
  if(dR_min < 0.4) return true;
  else return false;
}

//Cleaning:
bool nanoAna::cleanFromArray(Particle target, vector<Particle> array, float dRcut){
  bool result = true; //cleaned
  float dRmin = 1000;
  for(int i=0; i<(int)array.size(); i++){
    float dR = target.v.DeltaR(array.at(i).v);
    if(dR < dRmin) dRmin = dR;
  }
  //If the minimum dR of any Particle from the array
  //with the target Particle is less than the dRcut,
  //then the target is not isolated.
  if(dRmin < dRcut) result = false;
  return result;
}

//------------------------
// DNN specific functions:
//------------------------
vector<float> nanoAna::loadScalingParameters(const char* filename) {
  vector<float> params;
  ifstream file(filename);    
  if (!file.is_open()) {
    cerr << "ERROR: Could not open scaling parameter file: " << filename << endl;
    return params;
  }
  float val;
  while(file >> val) params.push_back(val);
  file.close();
  return params;
}

void nanoAna::loadOneDNN(const TString& dir_path, 
                         const TString& model_filename, 
                         Ort::Session*& session, 
                         std::vector<float>& scale_min, 
                         std::vector<float>& scale_max) {
  
    TString model_path = dir_path + "/" + model_filename;
    TString min_path   = dir_path + "/scaling_parameters_min.txt";
    TString max_path   = dir_path + "/scaling_parameters_max.txt";
    
    // Here is where the scaling parameters are read and assigned to the globals
    scale_min = loadScalingParameters(min_path.Data());
    scale_max = loadScalingParameters(max_path.Data());
    
    // Initialize the session (assumes session_options is accessible)
    session = new Ort::Session(*ort_env, model_path.Data(), session_options);
    cout<<">> loaded DNN model and scaling parameters: "<<model_path<<endl;
}

void nanoAna::loadAllDNNs() {
    
    // Load DNN: DY-vs-VLLD
    TString dy_path  = "trained_models/DY-vs-VLLD_Run3_Feb19";
    TString dy_model = "model_DY-vs-VLLD_Run3_Feb19.onnx";
    loadOneDNN(dy_path, dy_model, session_dy, scale_min_dy, scale_max_dy);

    // Similarly, load the other DNNs here ...
    // Declare the global variables: session_* and scale_* in the DNN block of nanoAna.h
}

float nanoAna::evaluateDNN(Ort::Session* session, 
                           std::vector<float> input_vars, 
                           const std::vector<float>& scale_min, 
                           const std::vector<float>& scale_max,
                           const char* input_name,
                           const char* output_name
			   )
{
  // --------------
  // Safety checks
  // --------------
  
  if (!session) {cout<<"\033[31m[ERROR] evaluateDNN: Ort::Session is not loaded!\033[0m"<<endl; return -99;}
  if (input_vars.empty()) {cout << "\033[31m[ERROR] evaluateDNN: input_vars is empty!\033[0m"<<endl; return -99;}
  if (input_vars.size() != scale_min.size() || input_vars.size() != scale_max.size()) {
    cout << "\033[31m[ERROR] evaluateDNN: Dimension mismatch! " 
         << "Inputs: "<<input_vars.size()<<", ScaleMin: "<<scale_min.size()<<", ScaleMax: "<<scale_max.size() 
         << " -> Returning -99.\033[0m" << endl;
    return -99;
  }

  // ------------------------------
  // Normalize input variables
  // Using min-max scaler here.
  // Same was used during training.
  // ------------------------------
  for(size_t i = 0; i < input_vars.size(); i++) {
    float raw = input_vars[i];
    float diff = scale_max[i] - scale_min[i];
    if(diff != 0) input_vars[i] = 2.0 * ((raw - scale_min[i]) / diff) - 1.0;
    else          input_vars[i] = 0.0; 
  }
      
  // ------------------------
  // Setup memory and tensor
  // ------------------------
  Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  std::vector<int64_t> input_shape = {1, static_cast<int64_t>(input_vars.size())}; 
  Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
							    memory_info,         // CPU memory allocation
							    input_vars.data(),   // Pointer to scaled float data
							    input_vars.size(),   // Total number of data points
							    input_shape.data(),  // Pointer to the shape array
							    input_shape.size()); // Number of dimensions (2)
  const char* input_names[]  = {input_name};  
  const char* output_names[] = {output_name}; 

  // --------------
  // Run inference
  // --------------
  // Using basic if-checks instead of heavy exception blocks keeps the loop fast
  auto output_tensors = session->Run(
				     Ort::RunOptions{nullptr}, // Run options (no special threading or anything)
				     input_names,              // Input layer name (e.g., "input")
				     &input_tensor,            // Pointer to the input variable tensor
				     1,                        // Number of input tensors passing in
				     output_names,             // Output layer name (e.g., "keras_tensor_3")
				     1                         // Expected umber of output tensors
				     );

  if (output_tensors.empty()) {cout << "\033[31m[ERROR] evaluateDNN: Empty tensor!" << endl; return -99;}
  float* floatarr = output_tensors.front().GetTensorMutableData<float>();
  float score = floatarr[0];
  return score;
}
