#include "bkg_subtr_includes.h"
#include "pdfs.h"

using namespace std;

float fit_jpsik(TChain* data, TChain* MC, const string cut, const vector<float> sig_m_range){


	auto full_pdf = jpsik_pdf("jpsik", "bkmm_jpsimc_mass");
	auto w=full_pdf.w;

	//Convert the MC to a RooFit format
	TH1F* hMCmass = new TH1F("hMCmass", "MC mass histogram", 100, 5.15, 5.42);
	MC->Draw("bkmm_jpsimc_mass>>hMCmass", cut.c_str(), "goff");
	RooDataHist rooMC("rooMC", "rooMC", RooArgSet(*w->var("bkmm_jpsimc_mass")), hMCmass); //this creates a histogram, so we will go for a binned fit. RooDataSet would create an unbinned dataset.
	cout<<"Entries in MC dataset: "<<rooMC.sumEntries()<<endl;

	//take the signal-only pdf and fit it to the MC
	auto signal_only_pdf = w->pdf("signalModel_jpsik");
	signal_only_pdf->fitTo(rooMC);

	//let's plot stuff. I suggest move this to a separate function.
	TCanvas cForMC("MCcanvas");
	auto frameForMC = w->var("bkmm_jpsimc_mass")->frame(); //this makes the RooFit frame
	rooMC.plotOn(frameForMC);
	signal_only_pdf->plotOn(frameForMC);
	frameForMC->Draw();
	cForMC.SaveAs("plots/MC_mass_fit.pdf");
	////////

	//ok, now we got the signal shape. Let's fix its parameters and apply this shape to fit the data
	full_pdf.freezeSignal(); //go check in pdfs.cpp what this does.


	//now we can cleanup the memory and load the actual data
	rooMC.Delete();

	//define the fit and integration limits
	const float m_min_fit = 5.07;
	const float m_max_fit = 5.65;
	w->var("bkmm_jpsimc_mass")->setRange("sigRange", sig_m_range[0], sig_m_range[1]);
	w->var("bkmm_jpsimc_mass")->setMin(m_min_fit);
	w->var("bkmm_jpsimc_mass")->setMax(m_max_fit);
	w->var("bkmm_jpsimc_mass")->setRange("fitRange", m_min_fit, m_max_fit);

	//find out how much background there is under the signal by fitting the data
	TH1F* hdatamass = new TH1F("hdatamass", "data mass histogram", 100, m_min_fit, m_max_fit);

	data->Draw("bkmm_jpsimc_mass>>hdatamass", cut.c_str(), "goff"); //the cut is already applied on this dataset, but it never hurts...
	RooDataHist roodata("roodata", "roodata", RooArgSet(*w->var("bkmm_jpsimc_mass")), hdatamass); //this creates a histogram, so we will go for a binned fit. RooDataSet would create an unbinned dataset.
	cout<<"Entries in data dataset: "<<roodata.sumEntries()<<endl;

	//take the signal-only pdf and fit it to the MC
	auto SB_pdf = w->pdf("jpsik");
	SB_pdf->fitTo(roodata);

	//let's plot stuff. I suggest move this to a separate function.
	TCanvas cForData("datacanvas");
	auto frameForData = w->var("bkmm_jpsimc_mass")->frame(); //this makes the RooFit frame
	roodata.plotOn(frameForData);
	SB_pdf->plotOn(frameForData, RooFit::NormRange("fitRange"));
	frameForData->Draw();
	cForData.SaveAs("plots/data_mass_fit.pdf");
	// make the plotting better! We need each pdf component (signal, exponential background, and PR background) in a different color, and a legend!
	////////
	auto mass_var = w->var("bkmm_jpsimc_mass");

	float sig_int = (w->var("N_sig_jpsik")->getVal())*(w->pdf("signalModel_jpsik")->createIntegral(RooArgSet(*mass_var),RooFit::NormSet(*mass_var),RooFit::Range("sigRange"))->getVal());
	float bkg_int = (w->var("N_bkg_jpsik")->getVal())*(w->pdf("bkgModel_jpsik")->createIntegral(RooArgSet(*mass_var),RooFit::NormSet(*mass_var),RooFit::Range("sigRange"))->getVal());
	cout<<"signal integral in signal mass range = "<<sig_int<<endl;
	cout<<"background integral in signal mass range = "<<bkg_int<<endl;
	float tot_int = (roodata.sumEntries())*(w->pdf("jpsik")->createIntegral(RooArgSet(*mass_var),RooFit::NormSet(*mass_var),RooFit::Range("sigRange"))->getVal());
	cout<<"Cross-check. Does the some of the two make the total, within good approximation? N_bkg+N_sig-Ntot is = "<<bkg_int+sig_int-tot_int<<endl;

	return sig_int/(sig_int+bkg_int);
} 
