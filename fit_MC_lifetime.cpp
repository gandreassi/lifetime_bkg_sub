#include "bkg_subtr_includes.h"
#include "pdfs.h"

using namespace std;

void fit_MC_mass_lifetime(TChain* MC, const string cut){

	const float m_min = 5.15;
	const float m_max = 5.42;
	const float t_min = 0;
	const float t_max = 9e-12;

	auto full_pdf = jpsik_pdf("jpsik", "bkmm_jpsimc_mass", "bkmm_jpsimc_tau");
	auto w=full_pdf.w;

	cout<<"Copying MC tree with cuts. This may take a while..."<<endl;

	//turn off some branches so the copy operation will be more efficient
	MC->SetBranchStatus("*",0);
	MC->SetBranchStatus("Muon*_eta",1);
	MC->SetBranchStatus("Muon*_pt",1);
	MC->SetBranchStatus("Muon*Id",1);
	MC->SetBranchStatus("bkmm_jpsimc*",1);
	MC->SetBranchStatus("HLT_DoubleMu4_3_Jpsi*",1);

	ostringstream m_t_ranges_cut;
	TTree* MC_cut = MC->CopyTree(cut.c_str());
	cout<<"...done!"<<endl;

	//this time we will make an unbinned fit
	w->var("bkmm_jpsimc_mass")->setMin(m_min);
	w->var("bkmm_jpsimc_mass")->setMax(m_max);
	w->var("bkmm_jpsimc_tau")->setMin(t_min);
	w->var("bkmm_jpsimc_tau")->setMax(t_max);
	m_t_ranges_cut<<"(bkmm_jpsimc_mass>"<<m_min<<" && bkmm_jpsimc_mass<"<<m_max<<" && bkmm_jpsimc_tau>"<<t_min<<" && bkmm_jpsimc_tau<"<<t_max<<")";

	RooDataSet rooMC("rooMC", "rooMC", MC_cut, RooArgSet(*w->var("bkmm_jpsimc_mass"), *w->var("bkmm_jpsimc_tau")), +m_t_ranges_cut.str().c_str());
	cout<<"Entries in RooDataSet: "<<rooMC.sumEntries()<<endl;
	auto model = w->pdf("signalModel_m_t_jpsik");

	model->fitTo(rooMC);

	//plot mass projection
	auto mframe = w->var("bkmm_jpsimc_mass")->frame();
	rooMC.plotOn(mframe);
	model->plotOn(mframe);
	TCanvas mc("canvas for mass projection");
	mframe->Draw();
	mc.SaveAs("plots/MC_2Dfit_m_proj.pdf");

	//plot lifetime projection
	auto tframe = w->var("bkmm_jpsimc_tau")->frame();
	rooMC.plotOn(tframe);
	model->plotOn(tframe);
	TCanvas tc("canvas for mass projection");
	tframe->Draw();
	tc.SaveAs("plots/MC_2Dfit_t_proj.pdf");
}