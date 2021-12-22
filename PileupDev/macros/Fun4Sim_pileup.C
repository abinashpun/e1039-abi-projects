#include <top/G4_Beamline.C>
#include <top/G4_Target.C>
#include <top/G4_InsensitiveVolumes.C>
#include <top/G4_SensitiveDetectors.C>
R__LOAD_LIBRARY(libana_pileup)

using namespace std;

/*
  Macro to run pileup (still in development)
*/

int Fun4Sim_pileup(const int nevent = 10)
{

  ///choose from one of the pileup method; histogram input, function input or set fixed size of bucket if both false 
  const  bool hist_pileup = true;
  const bool function_pileup = false;
  const int fixedBucketSize = 5000;

  const bool legacy_rec_container = true;

  const bool do_collimator = true;
  const bool do_target     = true;
  const bool do_shielding  = true;
  const bool do_fmag       = true;
  const bool do_kmag       = true;
  const bool do_absorber   = true;

  const double collimator_pos_z = -602.36;
  const double target_coil_pos_z = -130.;//-300.;
  const double target_l = 7.9; //cm
  const double target_z = (7.9-target_l)/2.; //cm

  const double FMAGSTR = -1.054;
  const double KMAGSTR = -0.951;


  recoConsts *rc = recoConsts::instance();
  rc->set_DoubleFlag("FMAGSTR", FMAGSTR);
  rc->set_DoubleFlag("KMAGSTR", KMAGSTR);
  rc->set_DoubleFlag("Z_UPSTREAM", -800.);
  rc->Print();

  GeomSvc* geom_svc = GeomSvc::instance();
  geom_svc->printTable();

  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(0);

  


  ///Pile up process
  SQPileupGen* extgen = new SQPileupGen();

  if(function_pileup)
    {
      TF1* intensity_profile = new TF1("intensity_profile", "[0]*pow(x,[1])*exp(-[2]*x+exp(-[3]*x))+[4]", 0, 5000);
      intensity_profile->SetParameters(6.35,1.38,4.9e-3,4.7e-3,178.8);//Set your function and parameters
      extgen->set_beam_intensity_profile(intensity_profile);
    }
  else if(hist_pileup)
    {  
      TFile *f = new TFile("e906_rf00.root");// e906_rf00.root (NIM3 beam intesnity profile from all RS 67)
      TH1D* hprof = (TH1D*)f->Get("h1_rf00");

      //loop over the bins	
      int nbin = hprof->GetNbinsX();
      for(int ibin=1; ibin<=nbin; ++ibin) 
	{

	  double c = hprof->GetBinContent(ibin);
	  double w = hprof->GetBinWidth(ibin);
	  hprof->SetBinContent(ibin,c*w);
	}
      extgen->set_inhibit_threshold(5000);//3000-5000
      extgen->set_proton_coeff(0.038);//For Run 15361-15370, thredhold = 3000 and coeff = 0.033
      extgen->set_beam_intensity_profile_histo(hprof);
    }
  else
    {
      extgen->setBucketSize(fixedBucketSize);//manual pileup
    }
  extgen->setExtInputFile("/pnfs/e1039/persistent/users/apun/bkg_study/e1039pythiaGen_17Nov21/1_bkge1039_pythia_17Nov21_100M.root");
  //extgen->setExtInputFile("bkg.root");//uncomment for gridjobs
  se->registerSubsystem(extgen);



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

  // digitizer
  SQDigitizer* digitizer = new SQDigitizer("Digitizer", 0);
  //digitizer->Verbosity(99);
  se->registerSubsystem(digitizer);


  se->registerSubsystem(new TruthNodeMaker());

  /// Trigger emulator
  DPTriggerAnalyzer* dptrigger = new DPTriggerAnalyzer();
  dptrigger->set_road_set_file_name("$E1039_RESOURCE/trigger/trigger_67.txt");//trigger_67.txt(E906 data), trigger_78.txt(~E1039), trigger_103.txt (J/Psi)
  se->registerSubsystem(dptrigger);

  // trakcing module
  SQReco* reco = new SQReco();
  reco->Verbosity(0);
  reco->set_legacy_rec_container(legacy_rec_container);
  //reco->set_geom_file_name("support/geom.root"); //not needed as it's created on the fly
  reco->set_enable_KF(true);           //Kalman filter not needed for the track finding, disabling KF saves a lot of initialization time
  reco->setInputTy(SQReco::E1039);     //options are SQReco::E906 and SQReco::E1039
  reco->setFitterTy(SQReco::KFREF);    //not relavant for the track finding
  reco->set_evt_reducer_opt("none");   //if not provided, event reducer will be using JobOptsSvc to intialize; to turn off, set it to "none", for normal tracking, set to something like "aoc"
  reco->set_enable_eval(false);          //set to true to generate evaluation file which includes final track candidates 
  //reco->set_eval_file_name("eval.root");
  reco->set_enable_eval_dst(false);     //set to true to include final track cnadidates in the DST tree
  //if(cosmic) reco->add_eval_list(3);    //output of cosmic reco is contained in the eval output for now
  //reco->add_eval_list(3);             //include back partial tracks in eval tree for debuging
  //reco->add_eval_list(2);             //include station-3+/- in eval tree for debuging
  //reco->add_eval_list(1);             //include station-2 in eval tree for debugging
  se->registerSubsystem(reco);

  TruthNodeMaker* truthMaker = new TruthNodeMaker();
  truthMaker->set_legacy_rec_container(legacy_rec_container);
  se->registerSubsystem(truthMaker);

  VertexFit* vertexing = new VertexFit();
  se->registerSubsystem(vertexing);

  //Analysis module for Pileup
  AnaPileup* ana_pileup = new AnaPileup();
  ana_pileup->set_out_name("pileup.root");
  ana_pileup->set_legacy_rec_container(legacy_rec_container);
  se->registerSubsystem(ana_pileup);  

  // input - we need a dummy to drive the event loop
  Fun4AllInputManager *in = new Fun4AllDummyInputManager("DUMMY");
  se->registerInputManager(in);

  ///////////////////////////////////////////
  // Output
  ///////////////////////////////////////////
  // DST output manager, tunred off to save disk by default
  //Fun4AllDstOutputManager* out = new Fun4AllDstOutputManager("DSTOUT","DST.root");
  //se->registerOutputManager(out);

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
