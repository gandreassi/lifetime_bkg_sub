#include "bkg_subtr_includes.h"
#include <string>

using namespace std;

int main (int argc, char *argv[]) {

	const vector<float> sig_m_range{5.15, 5.4};
	const vector<float> bkg_m_range{5.4, 5.6};

	//load data and MC
	TChain* data = new TChain("Candidates");
	data->Add("/mnt/hadoop/scratch/gandreas/NanoAOD/505/Charmonium+Run2018A-17Sep2018-v1+MINIAOD/Bukmm_flat/0*.root");
	TChain* MC = new TChain("Candidates");
	MC->Add("/mnt/hadoop/scratch/gandreas/NanoAOD/505/BuToJpsiK_BMuonFilter_SoftQCDnonD_TuneCP5_13TeV-pythia8-evtgen+RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v2+MINIAODSIM/MCmatch_flat_Bukmm/*.root");

	//define a cut
	const string cut = "Muon1_softId && "\
				    "abs(Muon1_eta)<1.4 && "\
				    "Muon1_pt>4 && "\
				    "Muon2_softId && "\
				    "abs(Muon2_eta)<1.4 && "\
				    "Muon2_pt>4 && "\
				    "abs(bkmm_jpsimc_mass-5.3)<0.5 && "\
				    "bkmm_jpsimc_sl3d>4 && "\
				    "bkmm_jpsimc_vtx_chi2dof<5 && "\
				    "HLT_DoubleMu4_3_Jpsi";

	float sig_frac = fit_jpsik(data, MC, cut, sig_m_range); //this function will fit jpsik on data and MC, and return the signal fraction in a given mass interval
	// now plot with background subtraction
	plot_bkg_sub_tau(data, MC, cut, sig_m_range, bkg_m_range, sig_frac);

	//now let's do the 2D unbinned fit to m and tau
	fit_MC_mass_lifetime(MC, cut);

	return 0;
}
