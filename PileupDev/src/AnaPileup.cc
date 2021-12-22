/**
 * \class AnaPileup
 * \ module for pileup study 
 * \author Abinash Pun
 *
 * 
 */


#include "AnaPileup.h"


#include <interface_main/SQHitVector.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQMCEvent.h>
#include <interface_main/SQTrackVector.h>
#include <interface_main/SQDimuonVector.h>

#include <ktracker/SRecEvent.h>
#include <geom_svc/GeomSvc.h>

#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/PHTFileServer.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>


#include <TFile.h>
#include <TTree.h>
#include <TClonesArray.h>


#define LogError(exp)		std::cout<<"ERROR: "  <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl
#define LogWarning(exp)	    std::cout<<"WARNING: "<<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl

using namespace std;

AnaPileup::AnaPileup(const std::string& name) :
  SubsysReco(name),
  _event(0),
  _event_header(nullptr),
  _hit_vector(nullptr),
  _out_name("eval.root"),
  legacyContainer(true)
{

}
AnaPileup::~AnaPileup()
{

  delete pos1;
  delete pos2;
  delete pos3;
  delete posvtx;
  delete mom1;
  delete mom2;
  delete mom3;
  delete momvtx;
  delete rec_mom1;
  delete rec_momvtx;
  delete rec_posvtx;
  delete rec_momtgt;
  delete rec_postgt;

  delete rec_pmom;
  delete rec_nmom;
  delete rec_ppos;
  delete rec_npos;
  delete rec_vtx;

}


int AnaPileup::Init(PHCompositeNode* topNode) {

  pos1 = new TVector3();
  pos2 = new TVector3();
  pos3 = new TVector3();
  posvtx = new TVector3();
  mom1 = new TVector3();
  mom2 = new TVector3();
  mom3 = new TVector3();
  momvtx = new TVector3();
  rec_mom1 = new TVector3();
  rec_momvtx = new TVector3();
  rec_posvtx = new TVector3();
  rec_momtgt = new TVector3();
  rec_postgt = new TVector3();

  rec_pmom = new TVector3();
  rec_nmom = new TVector3();
  rec_ppos = new TVector3();
  rec_npos = new TVector3();
  rec_vtx  = new TVector3();

  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaPileup::InitRun(PHCompositeNode* topNode) {

  ResetEvalVars();
  InitEvalTree();

  p_geomSvc = GeomSvc::instance();

  int ret = GetNodes(topNode);
  if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaPileup::process_event(PHCompositeNode* topNode) {

  fpga1 = _event_header->get_trigger(SQEvent::MATRIX1);
  fpga2 = _event_header->get_trigger(SQEvent::MATRIX2);
  fpga3 = _event_header->get_trigger(SQEvent::MATRIX3);
  fpga4 = _event_header->get_trigger(SQEvent::MATRIX4);
  fpga5 = _event_header->get_trigger(SQEvent::MATRIX5);

  nim1 = _event_header->get_trigger(SQEvent::NIM1);
  nim2 = _event_header->get_trigger(SQEvent::NIM2);

  n_bucketsize = mi_evt_true->get_cross_section();
  eventID =  _event_header->get_event_id();

  Eval_sqhit(topNode);
  Eval_trkdim(topNode);

  evt_tree->Fill();

  ++_event;

  return Fun4AllReturnCodes::EVENT_OK;
}


///===========================
int AnaPileup::End(PHCompositeNode* topNode) {
  if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT)
    std::cout << "AnaPileup::End" << std::endl;

  PHTFileServer::get().cd(_out_name.c_str());
  _sqhits_tree->Write();
  trkTree->Write();
  dimTree->Write();
  evt_tree->Write();

  return Fun4AllReturnCodes::EVENT_OK;
}
//Evaluation fuction==============================================
int AnaPileup::Eval_sqhit(PHCompositeNode* topNode)
{
  ResetEvalVars();

  h1Lhit = 0;
  h1Rhit = 0;

  h1Bhit = 0;
  h1Thit = 0;

  GeomSvc* geomSvc = GeomSvc::instance();

  for(int ihit=0; ihit<_hit_vector->size(); ++ihit) {
    SQHit *sqhit = _hit_vector->at(ihit);
    int sq_detid = sqhit->get_detector_id();

    /// ST.1 HODOS 
    if (sq_detid == geomSvc->getDetectorID("H1B")){
      new((*pos_H1B)[h1Bhit]) TVector3(sqhit->get_truth_x(), sqhit->get_truth_y(), sqhit->get_truth_z());
      new((*mom_H1B)[h1Bhit]) TVector3(sqhit->get_truth_px(), sqhit->get_truth_py(), sqhit->get_truth_pz());
      eID_H1B[h1Bhit] = sqhit->get_element_id();
      ++h1Bhit;
    }
    if (sq_detid == geomSvc->getDetectorID("H1T")){
      new((*pos_H1T)[h1Thit]) TVector3(sqhit->get_truth_x(), sqhit->get_truth_y(), sqhit->get_truth_z());
      new((*mom_H1T)[h1Thit]) TVector3(sqhit->get_truth_px(), sqhit->get_truth_py(), sqhit->get_truth_pz());
      eID_H1T[h1Thit] = sqhit->get_element_id();
      ++h1Thit;
    }
    if (sq_detid == geomSvc->getDetectorID("H1L")){
      eID_H1L[h1Lhit] = sqhit->get_element_id();
      new((*pos_H1L)[h1Lhit]) TVector3(sqhit->get_truth_x(), sqhit->get_truth_y(), sqhit->get_truth_z());
      new((*mom_H1L)[h1Lhit]) TVector3(sqhit->get_truth_px(), sqhit->get_truth_py(), sqhit->get_truth_pz());
      ++h1Lhit;
    }
    if (sq_detid == geomSvc->getDetectorID("H1R")){
      eID_H1R[h1Rhit] = sqhit->get_element_id();
      new((*pos_H1R)[h1Rhit]) TVector3(sqhit->get_truth_x(), sqhit->get_truth_y(), sqhit->get_truth_z());
      new((*mom_H1R)[h1Rhit]) TVector3(sqhit->get_truth_px(), sqhit->get_truth_py(), sqhit->get_truth_pz());
      ++h1Rhit;
    }


    D0Xhit = 0;
    /// DOX CHAMBER
    if (sq_detid == geomSvc->getDetectorID("D0X") ) {
      eID_D0X[D0Xhit] = sqhit->get_element_id();
      new((*pos_D0X)[D0Xhit]) TVector3(sqhit->get_truth_x(), sqhit->get_truth_y(), sqhit->get_truth_z());
      new((*mom_D0X)[D0Xhit]) TVector3(sqhit->get_truth_px(), sqhit->get_truth_py(), sqhit->get_truth_pz());
      ++D0Xhit;
    }



  }

  _sqhits_tree->Fill();

  pos_H1B->Delete();
  pos_H1R->Delete();
  pos_H1L->Delete();
  pos_H1T->Delete();

  mom_H1B->Delete();
  mom_H1L->Delete();
  mom_H1R->Delete();
  mom_H1T->Delete();

  pos_D0X->Delete();
  mom_D0X->Delete();

  return Fun4AllReturnCodes::EVENT_OK;
}

///==============
int AnaPileup::Eval_trkdim(PHCompositeNode* topNode)
{
  int nTracks = trackVector->size();
  int nRecTracks = legacyContainer ? _recEvent->getNTracks() : recTrackVector->size();
  for(int i = 0; i < nTracks; ++i)
    {
      SQTrack* track = trackVector->at(i);
      *pos1 = track->get_pos_st1();
      *mom1 = track->get_mom_st1().Vect();
      *pos3 = track->get_pos_st3();
      *mom3 = track->get_mom_st3().Vect();
      *posvtx = track->get_pos_vtx();
      *momvtx = track->get_mom_vtx().Vect();

      int recid = track->get_rec_track_id();
      if(recid >= 0 && recid < nRecTracks)
	{
	  SRecTrack* recTrack = legacyContainer ? &(_recEvent->getTrack(recid)) : dynamic_cast<SRecTrack*>(recTrackVector->at(recid));
	  *rec_mom1 = recTrack->getMomentumVecSt1();
	  *rec_momvtx = recTrack->getVertexMom();
	  *rec_posvtx = recTrack->getVertexPos();
	  *rec_momtgt = recTrack->getTargetMom();
	  *rec_postgt = recTrack->getTargetPos();
	}
      else
	{
	  rec_mom1->SetXYZ(-999., -999., -999.);
	  rec_momvtx->SetXYZ(-999., -999., -999.);
	  rec_posvtx->SetXYZ(-999., -999., -999.);
	}

      trkTree->Fill();
    }


  ///==== Reco Dimuons===
  int nDimuons = dimuonVector->size();
  int nRecDimuons = legacyContainer ? _recEvent->getNDimuons() : (recDimuonVector ? recDimuonVector->size() : -1);
  for(int i = 0; i < nRecDimuons; ++i)
    {
      SRecDimuon *recDimuon = &(_recEvent->getDimuon(i));
      rec_mass = recDimuon->mass;
      *rec_pmom = recDimuon->p_pos.Vect();
      *rec_nmom = recDimuon->p_neg.Vect();
      *rec_ppos = recDimuon->vtx_pos;
      *rec_npos = recDimuon->vtx_neg;
      *rec_vtx  = recDimuon->vtx;

      dimTree->Fill();
    }
  return Fun4AllReturnCodes::EVENT_OK;
}


int AnaPileup::InitEvalTree() {

  PHTFileServer::get().open(_out_name.c_str(), "RECREATE");

  mom_H1T = new TClonesArray("TVector3");
  pos_H1T = new TClonesArray("TVector3");
  mom_H1B = new TClonesArray("TVector3");
  pos_H1B = new TClonesArray("TVector3");

  mom_H1L = new TClonesArray("TVector3");
  pos_H1L = new TClonesArray("TVector3");
  mom_H1R = new TClonesArray("TVector3");
  pos_H1R = new TClonesArray("TVector3");

  mom_D0X = new TClonesArray("TVector3");
  pos_D0X = new TClonesArray("TVector3");


  evt_tree = new TTree("pileup_evt", "Event level info from piling up bkg interactions");

  evt_tree->Branch("n_bucketsize",   &n_bucketsize,        "n_bucketsize/I");
  evt_tree->Branch("eventID", &eventID, "eventID/I");

  evt_tree->Branch("fpga1",      &fpga1,           "fpga1/I");
  evt_tree->Branch("fpga2",      &fpga2,           "fpga2/I");
  evt_tree->Branch("fpga3",      &fpga3,           "fpga3/I");
  evt_tree->Branch("fpga4",      &fpga4,           "fpga4/I");
  evt_tree->Branch("fpga5",      &fpga5,           "fpga5/I");

  evt_tree->Branch("nim1",      &nim1,           "nim1/I");
  evt_tree->Branch("nim2",      &nim2,           "nim2/I");

  ///For the pileup tree with hit information ======================
  _sqhits_tree = new TTree("pileup_sqhits", "Analsysis of sqhits from piling up bkg interactions");

  _sqhits_tree->Branch("n_bucketsize",   &n_bucketsize,        "n_bucketsize/I");

  /// Trigger info
  _sqhits_tree->Branch("fpga1",      &fpga1,           "fpga1/I");
  _sqhits_tree->Branch("fpga2",      &fpga2,           "fpga2/I");
  _sqhits_tree->Branch("fpga3",      &fpga3,           "fpga3/I");
  _sqhits_tree->Branch("fpga4",      &fpga4,           "fpga4/I");
  _sqhits_tree->Branch("fpga5",      &fpga5,           "fpga5/I");

  _sqhits_tree->Branch("nim1",      &nim1,           "nim1/I");
  _sqhits_tree->Branch("nim2",      &nim2,           "nim2/I");

  ///HODO HITS
  _sqhits_tree->Branch("h1Bhit",   &h1Bhit,        "h1Bhit/I");
  _sqhits_tree->Branch("h1Thit",   &h1Thit,        "h1Thit/I");
  _sqhits_tree->Branch("eID_H1T", eID_H1T, "eID_H1T[h1Thit]/I");
  _sqhits_tree->Branch("eID_H1B", eID_H1B, "eID_H1B[h1Bhit]/I");

  _sqhits_tree->Branch("h1Lhit",   &h1Lhit,        "h1Lhit/I");
  _sqhits_tree->Branch("h1Rhit",   &h1Rhit,        "h1Rhit/I");
  _sqhits_tree->Branch("eID_H1L", eID_H1L, "eID_H1L[h1Lhit]/I");
  _sqhits_tree->Branch("eID_H1R", eID_H1R, "eID_H1R[h1Rhit]/I");

  _sqhits_tree->Branch("mom_H1T", &mom_H1T, 256000, 99);
  _sqhits_tree->Branch("pos_H1T", &pos_H1T, 256000, 99);
  _sqhits_tree->Branch("mom_H1B", &mom_H1B, 256000, 99);
  _sqhits_tree->Branch("pos_H1B", &pos_H1B, 256000, 99);

  _sqhits_tree->Branch("mom_H1L", &mom_H1L, 256000, 99);
  _sqhits_tree->Branch("pos_H1L", &pos_H1L, 256000, 99);
  _sqhits_tree->Branch("mom_H1R", &mom_H1R, 256000, 99);
  _sqhits_tree->Branch("pos_H1R", &pos_H1R, 256000, 99);

  ///CHAMBER HITS
  _sqhits_tree->Branch("D0Xhit",   &D0Xhit,        "D0Xhit/I");
  _sqhits_tree->Branch("eID_D0X", eID_D0X, "eID_D0X[D0Xhit]/I");
  _sqhits_tree->Branch("mom_D0X", &mom_D0X, 256000, 99);
  _sqhits_tree->Branch("pos_D0X", &pos_D0X, 256000, 99);

  mom_H1T->BypassStreamer();
  pos_H1T->BypassStreamer(); 
  mom_H1B->BypassStreamer();
  pos_H1B->BypassStreamer();

  mom_H1L->BypassStreamer();
  pos_H1L->BypassStreamer();
  mom_H1R->BypassStreamer();
  pos_H1R->BypassStreamer();

  mom_D0X->BypassStreamer();
  pos_D0X->BypassStreamer();

  trkTree = new TTree("pileup_trk", "Track Tree Created by AnaPileup Module");
  trkTree->Branch("pos1", &pos1, 256000, 99);
  trkTree->Branch("pos2", &pos2, 256000, 99);
  trkTree->Branch("pos3", &pos3, 256000, 99);
  trkTree->Branch("posvtx", &posvtx, 256000, 99);
  trkTree->Branch("mom1", &mom1, 256000, 99);
  trkTree->Branch("mom2", &mom2, 256000, 99);
  trkTree->Branch("mom3", &mom3, 256000, 99);
  trkTree->Branch("momvtx", &momvtx, 256000, 99);
  trkTree->Branch("rec_mom1", &rec_mom1, 256000, 99);
  trkTree->Branch("rec_momvtx", &rec_momvtx, 256000, 99);
  trkTree->Branch("rec_posvtx", &rec_posvtx, 256000, 99);
  trkTree->Branch("rec_momtgt", &rec_momtgt, 256000, 99);
  trkTree->Branch("rec_postgt", &rec_postgt, 256000, 99);

  dimTree = new TTree("pileup_dim", "Dimuon Tree Created by AnaPileup Module");
  dimTree->Branch("rec_mass", &rec_mass, "rec_mass/D");
  dimTree->Branch("rec_pmom", &rec_pmom, 256000, 99);
  dimTree->Branch("rec_nmom", &rec_nmom, 256000, 99);
  dimTree->Branch("rec_ppos", &rec_ppos, 256000, 99);
  dimTree->Branch("rec_npos", &rec_npos, 256000, 99);
  dimTree->Branch("rec_vtx", &rec_vtx, 256000, 99);

  return 0;
}

int AnaPileup::ResetEvalVars() {

  run_id = std::numeric_limits<int>::max();
  spill_id = std::numeric_limits<int>::max();
  event_id = std::numeric_limits<int>::max();
  emu_trigger = 0;


  return 0;
}

int AnaPileup::GetNodes(PHCompositeNode* topNode) {


  _event_header = findNode::getClass<SQEvent>(topNode, "SQEvent");
  if (!_event_header) {
    LogError("!_event_header");
    //return Fun4AllReturnCodes::ABORTEVENT;
  }

  mi_evt_true = findNode::getClass<SQMCEvent     >(topNode, "SQMCEvent");


  _hit_vector = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if (!_hit_vector) {
    LogError("!_hit_vector");
    return Fun4AllReturnCodes::ABORTEVENT;
  }


  trackVector  = findNode::getClass<SQTrackVector>(topNode, "SQTruthTrackVector");
  dimuonVector = findNode::getClass<SQDimuonVector>(topNode, "SQTruthDimuonVector");
  if(!trackVector || !dimuonVector)
    {
      return Fun4AllReturnCodes::ABORTEVENT;
    }

  if(legacyContainer)
    {
      _recEvent = findNode::getClass<SRecEvent>(topNode, "SRecEvent");
      if (!_recEvent) 
	{
	  LogError("!_recEvent");
	  //return Fun4AllReturnCodes::ABORTEVENT;
	}
    }
  else
    {
      recTrackVector  = findNode::getClass<SQTrackVector>(topNode, "SQRecTrackVector");
      recDimuonVector = findNode::getClass<SQDimuonVector>(topNode, "SQRecDimuonVector");
      if(!recTrackVector)
	{
	  return Fun4AllReturnCodes::ABORTEVENT;
	}
    }


  return Fun4AllReturnCodes::EVENT_OK;
}

