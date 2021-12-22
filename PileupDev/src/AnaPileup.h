/**
 * \class AnaPileup
 * \Analysis module for bkg pile up study
 * \author  Abinash Pun
 *
 * Created: December-2021
 */

#ifndef _H_AnaPileup_H_
#define _H_AnaPileup_H_

// ROOT
#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <TMatrixD.h>
#include <TVector3.h>

// Fun4All includes
#include <fun4all/SubsysReco.h>

// STL includes
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <map>

class TVector3;
class TLorentzVector;
class TClonesArray;

class SQEvent;
class SQHitVector;
class SQHit;
class SQMCEvent;
class SQTrackVector;
class SQDimuonVector;

//class PHG4TruthInfoContainer;
//class PHG4HitContainer;
class SRecEvent;
class SRecTrack;
class GeomSvc;

class TFile;
class TTree;

class AnaPileup: public SubsysReco {

 public:

  AnaPileup(const std::string &name = "AnaPileup.root");
  virtual ~AnaPileup();

  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  int InitEvalTree();
  int ResetEvalVars();
 

  const std::string& get_out_name() const {
    return _out_name;
  }

  void set_out_name(const std::string& outName) {
    _out_name = outName;
  }
  void set_legacy_rec_container(bool b) { legacyContainer = b; }

 private:

  int GetNodes(PHCompositeNode *topNode);

  int Eval_sqhit(PHCompositeNode *topNode); 
  int Eval_trkdim(PHCompositeNode *topNode);

  bool legacyContainer;

  GeomSvc *p_geomSvc;
  size_t _event;

  SQEvent * _event_header;
  SQMCEvent     * mi_evt_true;
  SQHitVector *_hit_vector;

  SQTrackVector* trackVector;
  SQDimuonVector* dimuonVector;

  SRecEvent* _recEvent;
  SQTrackVector*  recTrackVector;
  SQDimuonVector* recDimuonVector;

  std::string _out_name;

  TTree* evt_tree;	
  TTree* _sqhits_tree;
  TFile *file;

  int n_bucketsize;
  int run_id;
  int spill_id;
  int event_id;
  int eventID;
  unsigned short emu_trigger;

  /// Trigger info
  int  fpga1;
  int  fpga2;
  int  fpga3;
  int  fpga4;
  int  fpga5;

  int  nim1;
  int  nim2;

  /// truth info
  int n_tracks;
  //hodoscope sqhit info
  TClonesArray* pos_H1T;
  TClonesArray* pos_H1B;
  TClonesArray* mom_H1T;
  TClonesArray* mom_H1B;

  TClonesArray* pos_H1L;
  TClonesArray* pos_H1R;
  TClonesArray* mom_H1L;
  TClonesArray* mom_H1R;


  //Hodoscope element ID 
  int h1Bhit;
  int h1Thit;
  int eID_H1T[9999];
  int eID_H1B[9999];

  int h1Rhit;
  int h1Lhit;
  int eID_H1R[9999];
  int eID_H1L[9999];


  //DC sqhit info
  TClonesArray* pos_D0X;
  TClonesArray* mom_D0X;
  int D0Xhit;
  int eID_D0X[9999];

  ///stuffs from recodev===
  TTree* trkTree;
  int charge1;
  TVector3* pos1;
  TVector3* pos2;
  TVector3* pos3;
  TVector3* posvtx;
  TVector3* mom1;
  TVector3* mom2;
  TVector3* mom3;
  TVector3* momvtx;
  TVector3* rec_mom1;
  TVector3* rec_momvtx;
  TVector3* rec_posvtx;
  TVector3* rec_momtgt;
  TVector3* rec_postgt;

  TTree* dimTree;
  TVector3* rec_pmom;
  TVector3* rec_nmom;
  TVector3* rec_ppos;
  TVector3* rec_npos;
  TVector3* rec_vtx;
  double rec_mass;


};


#endif /* _H_AnaPileup_H_ */
