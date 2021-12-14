/**
 * \class AnaBkg
 * \Analysis module for generating bkg
 * \author  Abinash Pun
 *
 * Created: 07-05-2020
 */

#ifndef _H_AnaBkg_H_
#define _H_AnaBkg_H_

// ROOT
#include <TMatrixD.h>
#include <TVector3.h>
#include <TLorentzVector.h>

// Fun4All includes
#include <fun4all/SubsysReco.h>

// STL includes
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <map>

namespace HepMC {
  class GenParticle;
};

class PHG4TruthInfoContainer;
class PHG4HitContainer;
class GeomSvc;
class PHHepMCGenEventMap;
class PHGenIntegral;

class TFile;
class TTree;
class TClonesArray;
class TVector3;
class TLorentzVector;

class AnaBkg: public SubsysReco {

 public:

  AnaBkg(const std::string &name = "AnaBkg.root");
  virtual ~AnaBkg() {
  }

  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  void set_legacy_rec_container(bool b) { legacyContainer = b; }

  int InitEvalTree();
  int ResetEvalVars();

  const std::string& get_out_name() const {
    return _out_name;
  }

  void set_out_name(const std::string& outName) {
    _out_name = outName;
  }

 private:

  int GetNodes(PHCompositeNode *topNode);

  int TruthRecoEval(PHCompositeNode *topNode);
  void TraceParent(const HepMC::GenParticle* par, const int depth, int parent_id_[2],double mom__[4], int parent_pid_[10]);


  bool legacyContainer;
  
  PHHepMCGenEventMap* genevtmap;
  PHGenIntegral * mi_gen_inte;

  float inte_lumi;
  long n_evt_gen;
  long n_evt_pro;

  int n;
  int n_parents;
  int mu_ID=0;
  int parent_id_[2];
  int parent_pid_[10];
  double mom__[4]={0};

  int parent_pdg[500];
  int mother_PDG[100];
  

  size_t _event;
  int eventID;

  PHG4TruthInfoContainer* _truth;

  std::string _out_name;
  TTree* tree1;
  TTree* tree2;
  TTree* _runinfo_tree;


  TFile* file;

  int pdg[100];
  int _extPDG[100];

  TVector3* gpos;
  TVector3* gmom;

  TVector3* coll_vtx;
  TClonesArray* pos;
  TClonesArray* mom;
  TClonesArray* parent;

  GeomSvc *p_geomSvc;
};


#endif /* _H_AnaBkg_H_ */
