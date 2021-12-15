#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
#include <TSystem.h>
#include <TRandom1.h>
#include <top/G4_Beamline.C>
#include <top/G4_Target.C>
#include <top/G4_InsensitiveVolumes.C>
#include <top/G4_SensitiveDetectors.C>

R__LOAD_LIBRARY(libfun4all)
R__LOAD_LIBRARY(libPHPythia8)
R__LOAD_LIBRARY(libg4detectors)
R__LOAD_LIBRARY(libg4testbench)
R__LOAD_LIBRARY(libg4eval)
R__LOAD_LIBRARY(libdptrigger)
R__LOAD_LIBRARY(libembedding)
R__LOAD_LIBRARY(libevt_filter)
R__LOAD_LIBRARY(libktracker)
R__LOAD_LIBRARY(libSQPrimaryGen)
R__LOAD_LIBRARY(libana_eloss)
#endif

using namespace std;

int Fun4Sim_ana_eloss(
		      const int nevent = 1
		      )
{

  //*****************************
  const double FMAGSTR = -0.;
  const double KMAGSTR = -0.; 

  const bool gen_gun      = true;
  const bool read_hepmc = false;

  recoConsts *rc = recoConsts::instance();
  rc->set_DoubleFlag("FMAGSTR", FMAGSTR);
  rc->set_DoubleFlag("KMAGSTR", KMAGSTR);
  rc->Print();


  GeomSvc::UseDbSvc(true);
  GeomSvc *geom_svc = GeomSvc::instance();
  geom_svc->printTable();

  ///////////////////////////////////////////
  // Make the Server
  //////////////////////////////////////////
  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(0);

  PHG4SimpleEventGenerator* gen = new PHG4SimpleEventGenerator("MUP");
  gen->add_particles("mu+", 1);  // mu+,e+,proton,pi+,Upsilon
  // gen->set_vertex_distribution_function(PHG4SimpleEventGenerator::Uniform, PHG4SimpleEventGenerator::Uniform, PHG4SimpleEventGenerator::Uniform);
  gen->set_vertex_distribution_mean(-0.1, -0.1, -0.1);
  gen->set_vertex_distribution_width(0.0, 0.0, 0.0);
  //gen->set_vertex_size_function(PHG4SimpleEventGenerator::Uniform);
  //gen->set_vertex_size_parameters(0.0, 0.0);
  gen->set_pxpypz_range(0., 0., 0. ,0., 10., 100.);
  se->registerSubsystem(gen);



  // Fun4All G4 module
  PHG4Reco *g4Reco = new PHG4Reco();
  //PHG4Reco::G4Seed(123);
  g4Reco->set_field(0);
  // size of the world - every detector has to fit in here
  g4Reco->SetWorldSizeX(1000);
  g4Reco->SetWorldSizeY(1000);
  g4Reco->SetWorldSizeZ(5000);
  // shape of our world - it is a tube
  g4Reco->SetWorldShape("G4BOX");
  // this is what our world is filled with
  g4Reco->SetWorldMaterial("G4_AIR"); //G4_Galactic, G4_AIR
  // Geant4 Physics list to use
  g4Reco->SetPhysicsList("FTFP_BERT");
  //g4Reco->SetPhysicsList("QGSP_BERT");
 

  se->registerSubsystem(g4Reco);

  // //size in cm
  // *********************************************** 
  double xsize = 43.2;  
  double ysize = 160.;  
  double zsize = 503.0;//503.;  
  //double zsize = 165.00;
  PHG4BlockSubsystem *fmag = NULL;
  fmag = new PHG4BlockSubsystem("fmag",0);
  fmag->set_double_param("place_x",0.);
  fmag->set_double_param("place_y",0.);
  fmag->set_double_param("place_z",zsize/2.-0.0001);
  fmag->set_double_param("size_x",xsize);
  fmag->set_double_param("size_y",ysize);
  fmag->set_double_param("size_z",zsize);
  fmag->set_string_param("material","G4_Fe"); //material of box
  //fmag->set_string_param("material","G4_CONCRETE"); // material of box
  fmag->SetActive();
  g4Reco->registerSubsystem(fmag);


  PHG4BlockSubsystem *det = NULL;
  det = new PHG4BlockSubsystem("det",0);
  det->set_double_param("place_x",0.);
  det->set_double_param("place_y",0.);
  det->set_double_param("place_z",505.0);
  det->set_double_param("size_x",xsize);
  det->set_double_param("size_y",ysize);
  det->set_double_param("size_z",0.50);
  det->set_string_param("material","G4_Fe"); //material of box
  //fmag->set_string_param("material","G4_CONCRETE"); // material of box
  det->SetActive();
  g4Reco->registerSubsystem(det);

  
 
  // save truth info to the Node Tree
  PHG4TruthSubsystem *truth = new PHG4TruthSubsystem();
  g4Reco->registerSubsystem(truth);

  //ana Eloss ....  
  AnaEloss *ana_eloss = new AnaEloss();
  se->registerSubsystem(ana_eloss);


  // input - we need a dummy to drive the event loop
  if(read_hepmc) {
    Fun4AllHepMCInputManager *in = new Fun4AllHepMCInputManager("HEPMCIN");
    in->Verbosity(10);
    in->set_vertex_distribution_mean(0,0,0,0);
    se->registerInputManager(in);
    in->fileopen("hepmcout.txt");
  } else {
    Fun4AllInputManager *in = new Fun4AllDummyInputManager("DUMMY");
    se->registerInputManager(in);
  }

  ///////////////////////////////////////////
  // Output
  ///////////////////////////////////////////

  // DST output manager, tunred off to save disk by default
  //Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", "DST.root");
  //se->registerOutputManager(out);

  if (nevent >= 0)
    {
      se->run(nevent);

      PHGeomUtility::ExportGeomtry(se->topNode(),"geom.root");

      // finish job - close and save output files
      se->End();
      se->PrintTimer();
      std::cout << "All done" << std::endl;

      // cleanup - delete the server and exit
      delete se;
      gSystem->Exit(0);
    } else { // TEve event display
    // gROOT->LoadMacro("EventDisplay.C");
    // EventDisplay(nevent);
  } 

  return 0;
}
