/**
 * \class AnaBkg
 * \ module for generating bkg candidates for pile up 
 * \author Abinash Pun
 *
 * 
 */


#include "AnaBkg.h"

#include <geom_svc/GeomSvc.h>

#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/PHTFileServer.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>

#include <phhepmc/PHHepMCGenEventMap.h>
#include <g4main/PHG4TruthInfoContainer.h>
#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4Hit.h>
#include <g4main/PHG4Particle.h>
#include <g4main/PHG4HitDefs.h>
#include <g4main/PHG4VtxPoint.h>
#include <phhepmc/PHGenIntegral.h>

#include <TFile.h>
#include <TTree.h>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TClonesArray.h>

#include <cstring>
#include <cmath>
#include <cfloat>
#include <stdexcept>
#include <limits>
#include <tuple>

#include <boost/lexical_cast.hpp>

#define LogError(exp)		std::cout<<"ERROR: "  <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl
#define LogWarning(exp)	    std::cout<<"WARNING: "<<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl

using namespace std;

AnaBkg::AnaBkg(const std::string& name) :
  SubsysReco(name),
  _event(0),

  _out_name("eval.root")
{
 
}

int AnaBkg::Init(PHCompositeNode* topNode) {
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaBkg::InitRun(PHCompositeNode* topNode) {

  eventID = 0;

  ResetEvalVars();
  InitEvalTree();

  p_geomSvc = GeomSvc::instance();

  int ret = GetNodes(topNode);
  if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaBkg::process_event(PHCompositeNode* topNode) {



    ResetEvalVars();

    std::cout << "event ID = " << eventID << std::endl;


     for (PHHepMCGenEventMap::Iter iter = genevtmap->begin(); iter != genevtmap->end(); ++iter) {
                PHHepMCGenEvent *genevt = iter->second;
                HepMC::GenEvent *evt = genevt->getEvent();
		int jj=0;
                for (HepMC::GenEvent::particle_const_iterator it = evt->particles_begin();
                                it != evt->particles_end(); it++) {
                        const HepMC::GenParticle* par = *it;
                        int pdg_id = par->pdg_id();

			if (fabs(pdg_id)!=13) continue;
                        int depth = 0;
                        TraceParent(par, depth, parent_id_,mom__,parent_pid_);
			parent_pdg[jj]=parent_pid_[0];
			//mother_PDG[jj]=parent_pid_[0];
			++jj;		  
               }
		n_parents=jj;
	
		coll_vtx->SetXYZ(genevt->get_collision_vertex().x(),genevt->get_collision_vertex().y(),genevt->get_collision_vertex().z());
		
   }
    


 
  if(_truth) {
  //  int n=0;
    for(auto iter=_truth->GetPrimaryParticleRange().first;
	iter!=_truth->GetPrimaryParticleRange().second;
	++iter) {
      PHG4Particle * par = iter->second; 
     
      int trk_id = par->get_track_id();
      pdg[n] = par->get_pid();
      _extPDG[n] = par->get_pid();

      int vtx_id =  par->get_vtx_id();

      // generated momentum and postion at target
      PHG4VtxPoint* vtx = _truth->GetVtx(vtx_id);

      new((*pos)[n]) TVector3(vtx->get_x(), vtx->get_y(), vtx->get_z());
      new((*mom)[n]) TVector3(par->get_px(), par->get_py(), par->get_pz());

      ++n;
      //std::cout<<"npart: "<<n<<std::endl;
      if(n>=100) break;
           
    }//truth loop
  }//truth condition
//}

 tree1->Fill();  
 tree2->Fill();

  ++eventID;


  (*pos).Delete();
  (*mom).Delete();

 return Fun4AllReturnCodes::EVENT_OK;
}


//play ground for Abi==============================================
int AnaBkg::TruthRecoEval(PHCompositeNode* topNode)
{
 return Fun4AllReturnCodes::EVENT_OK;
}

///===========================
int AnaBkg::End(PHCompositeNode* topNode) {

  mi_gen_inte = findNode::getClass<PHGenIntegral >(topNode, "PHGenIntegral");
  if (!mi_gen_inte) return Fun4AllReturnCodes::ABORTEVENT;
  inte_lumi = mi_gen_inte->get_Integrated_Lumi();
  n_evt_gen = mi_gen_inte->get_N_Generator_Accepted_Event();
  n_evt_pro = mi_gen_inte->get_N_Processed_Event();

  cout.precision(10);
  cout << "Integrated luminosity = " << inte_lumi << " /pb\n"
       << "N of gen. events = " << n_evt_gen << "\n"
       << "N of processed events = " << n_evt_pro << endl;  

  _runinfo_tree->Fill();

  tree2->Show(0);
  tree1->Show(0);


  PHTFileServer::get().cd(_out_name.c_str());

  _runinfo_tree->Write();
  tree2->Write();
  tree1->Write(); 
	  
  if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT)
    std::cout << "AnaBkg::End" << std::endl;

  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaBkg::InitEvalTree() { 
  PHTFileServer::get().open(_out_name.c_str(), "RECREATE");
  

  mom = new TClonesArray("TVector3");
  pos = new TClonesArray("TVector3");
  parent = new TClonesArray("int");

  ///For track tree ======================
  tree1 = new TTree("bkg", "particle info for pile up");
  tree1->Branch("n_parents",&n_parents,           "n_parents/I");
  tree1->Branch("parent_pdg",           parent_pdg,                 "parent_pdg[n_parents]/I");
  tree1->Branch("coll_vtx", &coll_vtx, 256000, 99);
  tree1->Branch("eventID",      &eventID,           "eventID/I");
  tree1->Branch("n",      &n,           "n/I"); 
  tree1->Branch("pdg",           pdg,                 "pdg[n]/I");
  tree1->Branch("mom", &mom, 256000, 99);
  tree1->Branch("pos", &pos, 256000, 99);


  mom->BypassStreamer();
  pos->BypassStreamer();

  int Nmax = 100;
  tree2 = new TTree("mother_info", "mother particles i info for pile up");
  tree2->Branch("n_parents",&n_parents,           "n_parents/I");
  tree2->Branch("mother_PDG",mother_PDG,"mother_PDG[n_parents]/I");


  _runinfo_tree = new TTree("runinfo_tree", "run level information");
  _runinfo_tree->Branch("inte_lumi",        &inte_lumi,              "inte_lumi/F");
  _runinfo_tree->Branch("n_evt_gen",        &n_evt_gen,              "n_evt_gen/I");
  _runinfo_tree->Branch("n_evt_pro",        &n_evt_pro,              "n_evt_pro/I");


  return 0;
}

int AnaBkg::ResetEvalVars() {
  n=0;
  n_parents=0;
  for (int i = 0; i<100; ++i){
  _extPDG[i]=9999;
  pdg[i] = 9999;
  parent_pdg[i] = 9999; 
  mother_PDG[i]=9999;
  }

  return 0;
}

int AnaBkg::GetNodes(PHCompositeNode* topNode) {

  _truth = findNode::getClass<PHG4TruthInfoContainer>(topNode, "G4TruthInfo");
  if (!_truth) {
    LogError("!_truth");
    return Fun4AllReturnCodes::ABORTEVENT;
  }
 genevtmap = findNode::getClass<PHHepMCGenEventMap>(topNode, "PHHepMCGenEventMap");

  return Fun4AllReturnCodes::EVENT_OK;
}

   void AnaBkg::TraceParent(const HepMC::GenParticle* par, const int depth, int parent_id_[2],double mom__[4], int parent_pid_[10])
  {
          //cout << setw(5) << par->pdg_id() ;
          if (depth ==0)mu_ID=par->pdg_id();
          if(depth==1 && mu_ID==13)parent_id_[0]=par->pdg_id();
          if(depth==1 && mu_ID==-13)parent_id_[1]=par->pdg_id();
          TLorentzVector mom4;
          if(depth==1 && abs(mu_ID)==13){
                  mom__[0]= par->momentum().px();
                  mom__[1]= par->momentum().py();
                  mom__[2]= par->momentum().pz();
                  mom__[3]= par->momentum().e();
                  parent_pid_[0] = par->pdg_id();
         }
          const HepMC::GenVertex* vtx = par->production_vertex();
          if (! vtx) {
                 //cout << "\n";
                  return;
          } else {
                  //cout << " <= ";
                  bool line_1st = true;
                  for (HepMC::GenVertex::particles_in_const_iterator it = vtx->particles_in_const_begin(); it != vtx->particles_in_const_end(); it++) {
                          if (! line_1st) { // Fill spaces
                                 //cout << "      "; // "  Muon"
                                //  for (int dd = 0; dd < depth; dd++) //cout << "  depth ==   : "<<depth;
                          }
                          //TraceParent(*it, depth + 1);
                          TraceParent(*it, depth + 1,parent_id_,mom__, parent_pid_);
                          line_1st = false;
			
                  }
          }
  }
   
