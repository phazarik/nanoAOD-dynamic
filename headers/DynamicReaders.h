// headers/DynamicReaders.h
#ifndef DYNAMIC_READERS_H
#define DYNAMIC_READERS_H

#include <TTree.h>
#include <TLeaf.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>

//////////////////////////////////////////////////////////////////
//                                                              //
//            DO NOT CHANGE THE FOLLOWING TEMPLATES             //
//                                                              //
//////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------
// DynamicArrayReader <OutType>
//
// This is a wrapper around TTreeReaderArray that enables dynamic reading of 
// NanoAOD collection branches across different CMS data formats and versions.
//
// In different NanoAOD versions, the branch types (as well as names) can change.
// This wrapper allows dynamic type-sasting to prevent strict-type crashes in
// TTreeReader, and alow allows to scan for possible alternative branch names
// before assigning the values to the array reader.
//
// If initialized with a default value, failure to find the branch results in
// falling back to the default value with a one-time warning. If no default value
// is provided during initialization, it throws a run-time error when that
// variable is accessed in the Process() function.
//-------------------------------------------------------------------------------

template <typename OutType>
class DynamicArrayReader {
public: 
  // ROOT's ACLiC needs these structs to be public to generate the dictionary
  struct Base {
    virtual ~Base() = default;
    virtual OutType At(size_t i) = 0;
    virtual size_t GetSize() = 0;
  };

  // Template implementation to handle casting from NanoAOD type (InType) to User type (OutType)
  template <typename InType>
  struct Impl : public Base {
    TTreeReaderArray<InType>* array;
    Impl(TTreeReader& reader, const char* name) {
      array = new TTreeReaderArray<InType>(reader, name);
    }
    ~Impl() override { delete array; }
    OutType At(size_t i) override { return static_cast<OutType>(array->At(i)); }
    size_t GetSize() override { return array->GetSize(); }
  };

private:
  Base* m_reader = nullptr;
  bool m_isValid = false;
  bool m_hasDefault = false;
  OutType m_defaultValue;
  std::string m_requestedBranch; // Keeps track of the branch name for error printing

public:
  DynamicArrayReader() = default;
  ~DynamicArrayReader() { delete m_reader; }

  //----------------------------------------------------------------------------
  // STRICT MODE: No default value provided. Will crash if branch is missing.
  //----------------------------------------------------------------------------
  void Init(TTree* tree, TTreeReader& reader, const std::vector<const char*>& possibleNames) {
    if (m_reader) { delete m_reader; m_reader = nullptr; }
    m_isValid = false;
    m_hasDefault = false; 
    
    // Save the primary requested name for potential error messages
    if (possibleNames.size() > 0) m_requestedBranch = possibleNames[0]; 
    if (!tree) return;

    for (const char* branchName : possibleNames) {
      TLeaf* leaf = tree->GetLeaf(branchName);
      if (!leaf) continue;

      TString type = leaf->GetTypeName();
      // TTreeReader requires exact type matching. Dynamically match NanoAOD types.
      if (type == "UChar_t" || type == "Byte_t") m_reader = new Impl<UChar_t>(reader, branchName);
      else if (type == "Char_t")   m_reader = new Impl<Char_t>(reader, branchName);
      else if (type == "Int_t")    m_reader = new Impl<Int_t>(reader, branchName);
      else if (type == "Short_t")  m_reader = new Impl<Short_t>(reader, branchName);
      else if (type == "UShort_t") m_reader = new Impl<UShort_t>(reader, branchName);
      else if (type == "UInt_t")   m_reader = new Impl<UInt_t>(reader, branchName);
      else if (type == "Float_t")  m_reader = new Impl<Float_t>(reader, branchName);
      else if (type == "Double_t") m_reader = new Impl<Double_t>(reader, branchName);
      else if (type == "Bool_t")   m_reader = new Impl<Bool_t>(reader, branchName);
      else continue;

      m_isValid = true;
      return;
    }
  }

  void Init(TTree* tree, TTreeReader& reader, const char* branchName) {
    Init(tree, reader, std::vector<const char*>{branchName});
  }

  //----------------------------------------------------------------------------
  // FALLBACK MODE: Default value provided. Safe to use if branch is missing.
  //----------------------------------------------------------------------------
  void Init(TTree* tree, TTreeReader& reader, const std::vector<const char*>& possibleNames, OutType def) {
    Init(tree, reader, possibleNames); // First setup in strict mode
    m_hasDefault = true;               // Then override with fallback flags
    m_defaultValue = def;
    
    // Print a one-time dim warning if the branch was not found in the tree
    if (!m_isValid && tree != nullptr) {
      std::cout << "\033[2m[WARNING] branch: " << m_requestedBranch 
                << " not found, using default value: " << m_defaultValue << "\033[0m\n";
    }
  }

  void Init(TTree* tree, TTreeReader& reader, const char* branchName, OutType def) {
    Init(tree, reader, std::vector<const char*>{branchName}, def);
  }

  //--------
  // Debug
  //--------
  bool IsValid() const { return m_isValid; }
  size_t GetSize() { return m_isValid ? m_reader->GetSize() : 0; }
  
  // Safe bracket operator with colored error messaging
  OutType operator[](size_t i) { 
    if (m_isValid) return m_reader->At(i);
    if (m_hasDefault) return m_defaultValue;
    
    // Triggers if the branch is missing AND it was initialized in strict mode
    std::string err = "\n\n\033[31m[FATAL ERROR]\033[0m \033[33mDynamicArrayReader\033[0m: "
                      "Missing branch \033[31m" + m_requestedBranch + "\033[0m without a default fallback!\n";
    throw std::runtime_error(err);
  }
};

//-------------------------------------------------------------------------------
// DynamicValueReader <OutType>
//
// This is a wrapper around TTreeReaderValue for reading single, event-level
// variables (e.g., MET, event weights, flags) across different NanoAOD versions.
//
// Similar to the previous wrapper, it accepts a list of possible branch names,
// binding to the first one found in the TTree. In case none of the names match,
// it falls back to any default value provided, or throws a run-time error.
//-------------------------------------------------------------------------------

template <typename OutType>
class DynamicValueReader {
public:
  struct Base {
    virtual ~Base() = default;
    virtual OutType Get() = 0;
  };

  template <typename InType>
  struct Impl : public Base {
    TTreeReaderValue<InType>* value;
    Impl(TTreeReader& reader, const char* name) {
      value = new TTreeReaderValue<InType>(reader, name);
    }
    ~Impl() override { delete value; }
    OutType Get() override { return static_cast<OutType>(**value); }
  };

private:
  Base* m_reader = nullptr;
  OutType m_defaultValue;
  bool m_isValid = false;
  bool m_hasDefault = false;
  std::string m_requestedBranch; // Keeps track of the branch name for error printing

public:
  DynamicValueReader() = default;
  ~DynamicValueReader() { delete m_reader; }

  //----------------------------------------------------------------------------
  // STRICT MODE: No default value provided. Will crash if branch is missing.
  //----------------------------------------------------------------------------
  void Init(TTree* tree, TTreeReader& reader, const std::vector<const char*>& possibleNames) {
    if (m_reader) { delete m_reader; m_reader = nullptr; }
    m_isValid = false;
    m_hasDefault = false;

    // Save the primary requested name for potential error messages
    if (possibleNames.size() > 0) m_requestedBranch = possibleNames[0];
    if (!tree) return;

    for (const char* branchName : possibleNames) {
      TLeaf* leaf = tree->GetLeaf(branchName);
      if (!leaf) continue;

      TString type = leaf->GetTypeName();
      // TTreeReader requires exact type matching. Dynamically match NanoAOD types.
      if (type == "UChar_t" || type == "Byte_t") m_reader = new Impl<UChar_t>(reader, branchName);
      else if (type == "Char_t")   m_reader = new Impl<Char_t>(reader, branchName);
      else if (type == "Int_t")    m_reader = new Impl<Int_t>(reader, branchName);
      else if (type == "Short_t")  m_reader = new Impl<Short_t>(reader, branchName);
      else if (type == "UShort_t") m_reader = new Impl<UShort_t>(reader, branchName);
      else if (type == "UInt_t")   m_reader = new Impl<UInt_t>(reader, branchName);
      else if (type == "Float_t")  m_reader = new Impl<Float_t>(reader, branchName);
      else if (type == "Double_t") m_reader = new Impl<Double_t>(reader, branchName);
      else if (type == "Bool_t")   m_reader = new Impl<Bool_t>(reader, branchName);
      else continue;

      m_isValid = true;
      return;
    }
  }

  void Init(TTree* tree, TTreeReader& reader, const char* branchName) {
    Init(tree, reader, std::vector<const char*>{branchName});
  }

  //----------------------------------------------------------------------------
  // FALLBACK MODE: Default value provided. Safe to use if branch is missing.
  //----------------------------------------------------------------------------
  void Init(TTree* tree, TTreeReader& reader, const std::vector<const char*>& possibleNames, OutType def) {
    Init(tree, reader, possibleNames); // First setup in strict mode
    m_hasDefault = true;               // Then override with fallback flags
    m_defaultValue = def;

    // Print a one-time dim warning if the branch was not found in the tree
    if (!m_isValid && tree != nullptr) {
      std::cout << "\033[2m[WARNING] branch: " << m_requestedBranch
                << " not found, using default value: " << m_defaultValue << "\033[0m\n";
    }
  }

  void Init(TTree* tree, TTreeReader& reader, const char* branchName, OutType def) {
    Init(tree, reader, std::vector<const char*>{branchName}, def);
  }

  //--------
  // Debug
  //--------
  bool IsValid() const { return m_isValid; }

  // Dereference operator with colored error messaging
  OutType operator*() {
    if (m_isValid) return m_reader->Get();
    if (m_hasDefault) return m_defaultValue;

    // Triggers if the branch is missing AND it was initialized in strict mode
    std::string err = "\n\n\033[31m[FATAL ERROR]\033[0m \033[33mDynamicValueReader\033[0m: "
                      "Missing branch \033[31m" + m_requestedBranch + "\033[0m without a default fallback!\n";
    throw std::runtime_error(err);
  }
};

#endif // DYNAMIC_READERS_H
