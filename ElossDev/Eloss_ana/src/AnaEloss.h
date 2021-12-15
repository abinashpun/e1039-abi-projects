#ifndef _H_AnaEloss_H__
#define _H_AnaEloss_H__

// STL includes
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <map>

#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <TMatrixD.h>
#include <TVector3.h>

#include <fun4all/SubsysReco.h>
#include <ktracker/GenFitExtrapolator.h>
#include <TGeoManager.h>
#include <TGeoMedium.h>
#include <TGeoNode.h>

class TVector3;
class TLorentzVector;
//class TClonesArray;


//Declaration the class to use in analysis module
class TFile;
class TTree;
class TH1;
class TH2;
class PHCompositeNode;
class PHG4TruthInfoContainer;
class SQHitVector;
class GeomSvc;
class PHG4HitContainer;
//class TGeoManager;
//class PHField;


//Defination of this analysis module class
class AnaEloss : public SubsysReco
{
 public:
  //Constructor
  AnaEloss(const std::string &name = "eloss_test.root");

  //Destructor
  virtual ~AnaEloss();

  //@
  //Basic Fun4All methods (SubsysReco functions) 

  int Init(PHCompositeNode *topNode); //called once you register the module with Fun4AllServer
  int InitRun(PHCompositeNode *topNode); //called before first event in new Run
  int process_event(PHCompositeNode *topNode); //called for every event (main function)
  int ResetEvent(PHCompositeNode *topNode); //clean up leftovers of the event
  int EndRun(const int runnumber); //called before InitRun
  int End(PHCompositeNode *topNode); //Last call
  //
  //@

  // void FindGeomInfo(TGeoNode* node);

 
 private:
  void ResetVars();
  void InitTree();

  ///get truth info
  //int TruthEval(PHCompositeNode *topNode);
  ///get track info
  //int TrkRecoEval(PHCompositeNode *topNode);
  ///get the required nodes
  int GetNodes(PHCompositeNode *topNode);
  // work area for saggita
  //int SagittaEval(PHCompositeNode *topNode);
  
  int ElossEval(PHCompositeNode *topNode);


  //int InitGeom(PHCompositeNode *topNode);

  PHG4TruthInfoContainer* _truth;
  // SQDimuonTruthInfoContainer* _dimuoninfo;
  //SRecEvent* _recEvent;
  //GeomSvc *p_geomSvc;
  //SQHitVector *_hit_vector;

  std::string outfilename;

  //trees and files
  TFile *file;
  //TTree *truth_tree;
  //TTree *track_tree;
  //TTree *sagitta_tree;
  TTree *eloss_tree;

  //truth variables
  //float truth_vtxx,truth_vtxy,truth_vtxz;
  //float truth_px, truth_py, truth_pz, truth_e;
  //float dimuon_xs;
  //int truth_pid;
  // float 

  //track variables
  //int krecstat;
  //float rectrack_px, rectrack_py, rectrack_pz;
  //float rectrack_vx, rectrack_vy, rectrack_vz;

  //Drift chamber variables
  //float x1, x2, x3, y1, y2, y3, z1, z2, z3, sagitta_2, sagitta_1, sag_rat, hit_pz1, hit_px1, hit_py1, hit_pz2, hit_px2, hit_py2, hit_pz3, hit_px3, hit_py3 , slope;
  //float pos_st1, pos_st2, pos_st3;


  ///Track extrapolator
  //GenFitExtrapolator _extrapolator;

  //TGeoManager * _t_geo_manager;


  //Eloss tree
  //float geant_eloss, genfit_eloss, fmaghit,d1uhit,genfit_final,fmaghit_p, d1uhit_p;
  //int flag;

  TVector3* fmag_mom, *fmag_pos, *det_pos, *det_mom;
  TVector3* truth_mom, *truth_pos;    

};

#endif  //
