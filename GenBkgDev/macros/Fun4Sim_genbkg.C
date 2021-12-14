#include <TSystem.h>

#include <top/G4_Beamline.C>
#include <top/G4_Target.C>
#include <top/G4_InsensitiveVolumes.C>
#include <top/G4_SensitiveDetectors.C>

R__LOAD_LIBRARY(libfun4all)
R__LOAD_LIBRARY(libPHPythia8)
R__LOAD_LIBRARY(libg4detectors)
R__LOAD_LIBRARY(libg4testbench)
R__LOAD_LIBRARY(libg4eval)
R__LOAD_LIBRARY(libg4dst)
R__LOAD_LIBRARY(libdptrigger)
R__LOAD_LIBRARY(libktracker)
R__LOAD_LIBRARY(libevt_filter)
R__LOAD_LIBRARY(libana_genbkg)

using namespace std;

int Fun4Sim_genbkg(const int nevent = 10)
{ 
  const bool pythia=true;
        const bool bkg=true;

  //! vtx gen flag
  const bool legacyVtxGen = true;

  const bool do_collimator = true;//true;
  const bool do_target     = true;
  const bool do_shielding  = true;//true;
  const bool do_fmag       = true;//true;
  const bool do_kmag       = true;
  const bool do_absorber   = true;

  const bool legacy_rec_container = true;

  const double collimator_pos_z = -602.36;
  const double target_coil_pos_z = -300.;
  const double target_l = 7.9; //cm
  const double target_z = (7.9-target_l)/2.; //cm

  const double FMAGSTR = -1.054;
  const double KMAGSTR = -0.951;

  recoConsts *rc = recoConsts::instance();
  rc->set_DoubleFlag("FMAGSTR", FMAGSTR);
  rc->set_DoubleFlag("KMAGSTR", KMAGSTR);
  //rc->set_IntFlag("RANDOMSEED", seed);
  rc->set_DoubleFlag("SIGX_BEAM", .3);
  rc->set_DoubleFlag("SIGY_BEAM", .3);
  if(legacyVtxGen)
  {
    rc->set_BoolFlag("TARGETONLY",false);
    rc->set_BoolFlag("DUMPONLY", false);
  }
  rc->Print();

  GeomSvc::UseDbSvc(true);
  GeomSvc* geom_svc = GeomSvc::instance();
  geom_svc->printTable();

  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(0);

  
    PHPythia8 *pythia8 = new PHPythia8();
    
      pythia8->set_config_file("support/phpythia8_bkg.cfg");

    if (legacyVtxGen) pythia8->enableLegacyVtxGen();
    else{
      pythia8->set_vertex_distribution_mean(0, 0, target_coil_pos_z, 0);

    }
    se->registerSubsystem(pythia8);
    HepMCNodeReader* hr = new HepMCNodeReader();
    hr->set_particle_filter_on(true);
    hr->insert_particle_filter_pid(13);
    hr->insert_particle_filter_pid(-13);
      hr->Set_bkg_mode();
      hr->Set_pxy2pz_rat(0.25);
    se->registerSubsystem(hr);


  // Fun4All G4 module
  PHG4Reco* g4Reco = new PHG4Reco();
  //PHG4Reco::G4Seed(123);
  g4Reco->set_field_map(
      rc->get_CharFlag("fMagFile")+" "+
      rc->get_CharFlag("kMagFile")+" "+
      Form("%f",FMAGSTR) + " " +
      Form("%f",KMAGSTR) + " " +
      "5.0",
      PHFieldConfig::RegionalConst);

  // size of the world - every detector has to fit in here
  g4Reco->SetWorldSizeX(1000);
  g4Reco->SetWorldSizeY(1000);
  g4Reco->SetWorldSizeZ(5000);
  // shape of our world - it is a box
  g4Reco->SetWorldShape("G4BOX");
  // this is what our world is filled with
  g4Reco->SetWorldMaterial("G4_AIR"); //G4_Galactic, G4_AIR
  // Geant4 Physics list to use
  g4Reco->SetPhysicsList("FTFP_BERT");

  SetupBeamline(g4Reco, do_collimator, collimator_pos_z);
  SetupTarget(g4Reco, target_coil_pos_z, target_l, target_z, 1);
  SetupInsensitiveVolumes(g4Reco, do_shielding, do_fmag, do_kmag, do_absorber);
  SetupSensitiveDetectors(g4Reco);

  se->registerSubsystem(g4Reco);

  // save truth info to the Node Tree
  PHG4TruthSubsystem* truth = new PHG4TruthSubsystem();
  g4Reco->registerSubsystem(truth);
	

  //Analysis module for track QA
  AnaBkg* genbkg = new AnaBkg();
  genbkg->set_out_name("bkg.root");
  se->registerSubsystem(genbkg);  

 

  // input - we need a dummy to drive the event loop
  Fun4AllInputManager *in = new Fun4AllDummyInputManager("DUMMY");
  se->registerInputManager(in);

  ///////////////////////////////////////////
  // Output
  ///////////////////////////////////////////
  // DST output manager, tunred off to save disk by default
/*  Fun4AllDstOutputManager* out = new Fun4AllDstOutputManager("DSTOUT","DST.root");
  se->registerOutputManager(out);
*/

 /* if(pythia) {
    Fun4AllHepMCOutputManager *out = new Fun4AllHepMCOutputManager("HEPMCOUT", "hepmcout.txt");
    out->set_embedding_id(1);
    se->registerOutputManager(out);
  }
*/


  se->run(nevent);
  PHGeomUtility::ExportGeomtry(se->topNode(), "geom.root");

  // finish job - close and save output files
  se->End();
  se->PrintTimer();
  std::cout << "All done" << std::endl;

  // cleanup - delete the server and exit
  delete se;
  gSystem->Exit(0);

  return 0;
}
