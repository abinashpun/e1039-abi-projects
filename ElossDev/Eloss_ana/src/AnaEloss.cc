/*============================================================================= 
  Author: Abinash Pun
  
  ============================================================================*/
#include "AnaEloss.h"

/// Fun4All includes
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/PHTFileServer.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <g4main/PHG4TruthInfoContainer.h>
#include <g4main/PHG4Particle.h>
#include <g4main/PHG4VtxPoint.h>
#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4Hit.h>
#include <g4main/PHG4HitDefs.h>

/*
/// Tracking includes
#include <ktracker/SRecEvent.h>
#include <ktracker/GenFitExtrapolator.h>
#include <GenFit/FieldManager.h>
#include <GenFit/MaterialEffects.h>
#include <GenFit/TGeoMaterialInterface.h>
#include <GenFit/MeasuredStateOnPlane.h>
#include <GenFit/SharedPlanePtr.h>
#include <GenFit/RKTrackRep.h>
*/

/// ROOT includes
#include <TFile.h>
#include <TTree.h>
#include <TLorentzVector.h>
#include <TGeoManager.h>
/// C++ includes
#include <cstring>
#include <cmath>
#include <cfloat>
#include <iostream>
#include <TMatrixD.h>

#define LogError(exp)		std::cout<<"ERROR: "  <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl

using namespace std;

//
/// Constructor of the module
//
AnaEloss::AnaEloss(const std::string &name)
  : SubsysReco("AnaEloss")
{
  outfilename = name;
  ResetVars();
  InitTree(); 
  //add other initializers here 
}


//
/// Distructor of the module
//
AnaEloss::~AnaEloss()
{
  if(eloss_tree) delete eloss_tree;
  
}


//
/// Initialize the process
//
int AnaEloss::Init(PHCompositeNode *topNode)
{

  return Fun4AllReturnCodes::EVENT_OK;

}

//
/// Initialize the run
//
int AnaEloss::InitRun(PHCompositeNode *topNode)
{
 
  file = new TFile(outfilename.c_str(), "RECREATE");


  int ret = GetNodes(topNode);
  if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;
  return Fun4AllReturnCodes::EVENT_OK;

}

//
///  Main function which is called in each event (main play ground to extract what you want and implement your cuts etc. here)
//
int AnaEloss::process_event(PHCompositeNode *topNode)
{
  ElossEval(topNode);
 
  return Fun4AllReturnCodes::EVENT_OK;
}


//
/// called after the end of an event
//
int AnaEloss::ResetEvent(PHCompositeNode *topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}


//
/// called after the end of Run
//
int AnaEloss::EndRun(const int runnumber)
{
  return Fun4AllReturnCodes::EVENT_OK;
}


//
/// End the module (write files here)
//
int AnaEloss::End(PHCompositeNode *topNode)
{
  std::cout << " DONE PROCESSING " << endl;
  file->cd();
  eloss_tree->Write();
  file->Write();
  file->Close();
  return 0;
}


//
/// Function to get nodes required for analysis
//
int AnaEloss::GetNodes(PHCompositeNode* topNode)
{

  _truth = findNode::getClass<PHG4TruthInfoContainer>(topNode, "G4TruthInfo");
  if (!_truth) {
    LogError("!_truth");
    return Fun4AllReturnCodes::ABORTEVENT;
  }
  

  return Fun4AllReturnCodes::EVENT_OK;
}



//Genfit eloss test function
int AnaEloss::ElossEval(PHCompositeNode* topNode)
{
  double mass_mu = 105.6583715/1000.;//GeV
  PHG4HitContainer *FMAG_hits = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_fmag_0");

  if (!FMAG_hits) Fun4AllReturnCodes::ABORTEVENT;
  for(auto iter=FMAG_hits->getHits().first;
      iter!=FMAG_hits->getHits().second;
      ++iter) {
    PHG4Hit* g4hit= iter->second;
    if(g4hit->get_pz(0)<1.5) continue;
    fmag_mom->SetXYZ(g4hit->get_px(0),g4hit->get_py(0),g4hit->get_pz(0));
    fmag_pos->SetXYZ(g4hit->get_x(0),g4hit->get_y(0),g4hit->get_z(0));   

  }
 
  if(_truth) { 
    /// Loop over the primary truth particles
    for(auto iter=_truth->GetPrimaryParticleRange().first;
    	iter!=_truth->GetPrimaryParticleRange().second;
    	++iter) {
     

      /// Get this truth particle
      PHG4Particle * par= iter->second;
      float truth_pz = par->get_pz();
      if(truth_pz<10.) continue;
      truth_mom->SetXYZ(par->get_px(),par->get_py(),par->get_pz());

      int vtx_id =  par->get_vtx_id();
      PHG4VtxPoint* vtx = _truth->GetVtx(vtx_id);

      truth_pos->SetXYZ(vtx->get_x(),vtx->get_y(),vtx->get_z());
    }
  }
  

  PHG4HitContainer *det_hits = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_det_0");

  if (!det_hits) Fun4AllReturnCodes::ABORTEVENT;
  /// Loop over the primary truth particles
  for(auto iter=det_hits->getHits().first;
      iter!=det_hits->getHits().second;
      ++iter) {
    PHG4Hit* g4hit= iter->second;
    if(g4hit->get_pz(0)<1.5) continue;
    det_mom->SetXYZ(g4hit->get_px(0),g4hit->get_py(0),g4hit->get_pz(0));
    det_pos->SetXYZ(g4hit->get_x(0),g4hit->get_y(0),g4hit->get_z(0));  
  }


  eloss_tree->Fill();// Fill eloss Tree

  return 0;
}
//
/// Function to reset or initialize the member variables in this class
//
void AnaEloss::ResetVars()
{
}


//
/// Function to initialize tree with respective branch assigment
//
void AnaEloss::InitTree()
{

  eloss_tree = new TTree("elosstree","a tree with energy loss information via FMAG");
  eloss_tree->Branch("fmag_mom", "TVector3",&fmag_mom);
  eloss_tree->Branch("fmag_pos", "TVector3",&fmag_pos); 
  eloss_tree->Branch("det_pos", "TVector3",&det_pos);
  eloss_tree->Branch("det_mom", "TVector3",&det_mom);    
  eloss_tree->Branch("truth_pos", "TVector3",&truth_pos);
  eloss_tree->Branch("truth_mom", "TVector3",&truth_mom);

}

