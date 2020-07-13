#include "pdfs.h"
using namespace std;


jpsik_pdf::~jpsik_pdf(){ //desctructor
	w->Delete();
};

jpsik_pdf::jpsik_pdf(string name, string mass_var_name, string tau_var_name){

	jpsik_pdf::name = name;
			// create workspace
	w = new RooWorkspace("dpworkspace", "dpworkspace");

			// create observable variable
	RooRealVar* mass = new RooRealVar(mass_var_name.c_str(), "observable (mass)", 4.5, 6.0);
	RooRealVar* t = new RooRealVar(tau_var_name.c_str(), "observable (time)", 0, 10e-12);
	w->import(*mass);
	w->import(*t);

			//add signal model
	RooAddPdf* signal_mass_model = jpsik_pdf::make_signal_mass_model((name).c_str(), mass);
	RooDecay* signal_tau_model = jpsik_pdf::make_signal_tau_model((name).c_str(), t);
	RooProdPdf* signalModel_m_tau = new RooProdPdf(("signalModel_m_t_"+name).c_str(), ("signalModel_m_t_"+name).c_str(), RooArgList(*signal_mass_model, *signal_tau_model));
			//add bkg model
	RooAddPdf* bkg_mass_model = jpsik_pdf::make_bkg_mass_model((name).c_str(), mass);
	RooDecay* bkg_tau_model = jpsik_pdf::make_bkg_tau_model((name).c_str(), t);
	RooProdPdf* bkgModel_m_tau = new RooProdPdf(("bkgModel_m_t_"+name).c_str(), ("bkgModel_m_t_"+name).c_str(), RooArgList(*bkg_mass_model, *bkg_tau_model));
			//total model
	RooRealVar* N_sig = new RooRealVar(("N_sig_"+name).c_str(), "N_sig", 1e3, 10, 1e9);
	RooRealVar* N_bkg = new RooRealVar(("N_bkg_"+name).c_str(), "N_bkg", 1e2, 10, 1e9);
	RooAddPdf* model_m = new RooAddPdf((name+"_m").c_str(), (name+"_m").c_str(), RooArgList(*signal_mass_model, *bkg_mass_model), RooArgList(*N_sig, *N_bkg));
	RooAddPdf* model_m_tau = new RooAddPdf(name.c_str(), name.c_str(), RooArgList(*signalModel_m_tau, *bkgModel_m_tau), RooArgList(*N_sig, *N_bkg));

	w->import(*model_m_tau);
	w->import(*model_m, RooFit::RecycleConflictNodes());

	w->Print();
};

RooAddPdf* jpsik_pdf::make_signal_mass_model(string name, RooRealVar* mass){

	//let's try a model with three fully-independent gaussians
	//note: unfortunately, as it happens in ROOT, the memory is bound to the name you give to the variable, not to the variable itself

	RooRealVar* M = new RooRealVar(("M_"+name).c_str()      , ("M_"+name).c_str(), 5.28, 5.2, 5.35); //common mass mean

	RooRealVar* M2_shift = new RooRealVar(("M2_shift_"+name).c_str(), ("M2_shift_"+name).c_str(), 0, -0.1, 0.1); //shift of each gaussian wrt to common mean
	RooRealVar* M3_shift = new RooRealVar(("M3_shift_"+name).c_str(), ("M3_shift_"+name).c_str(), 0, -0.1, 0.1); //shift of each gaussian wrt to common mean

	RooFormulaVar* M2 = new RooFormulaVar(("M2_"+name).c_str(), "@0+@1", RooArgList(*M, *M2_shift));
	RooFormulaVar* M3 = new RooFormulaVar(("M3_"+name).c_str(), "@0+@1", RooArgList(*M, *M3_shift));

	RooRealVar* sigma1 = new RooRealVar(("sigma1_"+name).c_str(), ("sigma1_"+name).c_str(), 0.01, 0.002, 0.1);
	RooRealVar* sigma2 = new RooRealVar(("sigma2_"+name).c_str(), ("sigma2_"+name).c_str(), 0.03, 0.002, 0.1);
	RooRealVar* sigma3 = new RooRealVar(("sigma3_"+name).c_str(), ("sigma3_"+name).c_str(), 0.05, 0.002, 0.1);

	RooGaussian* gau1 = new RooGaussian(("gau1_"+name).c_str(), ("gau1_"+name).c_str(), *mass, *M,*sigma1);
	RooGaussian* gau2 = new RooGaussian(("gau2_"+name).c_str(), ("gau2_"+name).c_str(), *mass, *M2,*sigma2);
	RooGaussian* gau3 = new RooGaussian(("gau3_"+name).c_str(), ("gau3_"+name).c_str(), *mass, *M3,*sigma3);

	RooRealVar* frac_gau1 = new RooRealVar(("frac_gau1_"+name).c_str(), ("frac_gau1_"+name).c_str(), 0.5,0,1);
	RooRealVar* frac_gau2 = new RooRealVar(("frac_gau2_"+name).c_str(), ("frac_gau2_"+name).c_str(), 0.5,0,1);
	RooFormulaVar* rel_frac_gau2 = new RooFormulaVar(("rel_frac_gau2"+name).c_str(), "@0*(1-@1)", RooArgList(*frac_gau2, *frac_gau1));

	RooAddPdf* signalModel = new RooAddPdf(("signalModel_m_"+name).c_str(), ("Mass PDF for "+name).c_str(), RooArgList(*gau1, *gau2, *gau3), RooArgList(*frac_gau1, *rel_frac_gau2));
	return signalModel;
};

RooDecay* jpsik_pdf::make_signal_tau_model(string name, RooRealVar* t){

	//lifetime model
	RooRealVar* tau = new RooRealVar(("tau_"+name).c_str(), ("Lifetime of "+name).c_str(), 1.638e-12, 1.4e-12, 1.9e-12);

	RooRealVar* reso_mean = new RooRealVar(("reso_mean_"+name).c_str(), ("reso_mean_"+name).c_str(), 0);
	RooRealVar* reso_sigma = new RooRealVar(("reso_sigma_"+name).c_str(), ("reso_sigma_"+name).c_str(), 0.2e-12);
	RooResolutionModel* tau_reso_model = new RooGaussModel(("tau_reso_model_"+name).c_str(), ("Resolution model for "+name).c_str(), *t, *reso_mean, *reso_sigma);//fake resolution model for now. Replace with actual one, from MC!
	RooDecay* decay_expo = new RooDecay(("signalModel_t_"+name).c_str(), ("Exponential decay PDF for "+name).c_str(), *t, *tau, *tau_reso_model, RooDecay::SingleSided);
	return decay_expo;
};

RooAddPdf* jpsik_pdf::make_bkg_mass_model(string name, RooRealVar* mass){

			//exponential shape for combinatorial background
	RooRealVar* lambda = new RooRealVar(("expo_lambda_"+name).c_str(), ("expo_lambda_"+name).c_str(), -0.1, -0.5, 0.5);
	RooExponential* expo = new RooExponential(("expo_"+name).c_str(), ("expo_"+name).c_str(), *mass, *lambda);

			//partially-reconstructed background. We'll model it with an error function
	RooRealVar* t = new RooRealVar (("erf_t_"+name).c_str(), ("erf_t_"+name).c_str(), 5.1, 5.05, 5.2);
	RooRealVar* s = new RooRealVar (("erf_s_"+name).c_str(), ("erf_s_"+name).c_str(), 0.02, 0.01, 0.04);

	RooGenericPdf* erf_pdf = new RooGenericPdf(("erf_"+name).c_str(), "1+TMath::Erf((@1-@0)/@2)", RooArgList(*mass,*t,*s));

			// we should add a shape for b->J/psi pi...
	RooRealVar* frac_partreco = new RooRealVar(("frac_partreco"+name).c_str(), "frac_partreco", 0.75,0.3,1);

			//add all up
	RooAddPdf* bkgModel = new RooAddPdf(("bkgModel_m_"+name).c_str(), ("Mass PDF for "+name).c_str(), RooArgList(*expo, *erf_pdf), RooArgList(*frac_partreco));
	return bkgModel;
};

RooDecay* jpsik_pdf::make_bkg_tau_model(string name, RooRealVar* t){

	//lifetime model
	RooRealVar* tau = new RooRealVar(("tau_"+name).c_str(), ("Lifetime of "+name).c_str(), 0.2e-12, 0e-12, 10e-12);

	RooRealVar* reso_mean = new RooRealVar(("reso_mean_"+name).c_str(), ("reso_mean_"+name).c_str(), 0);
	RooRealVar* reso_sigma = new RooRealVar(("reso_sigma_"+name).c_str(), ("reso_sigma_"+name).c_str(), 1e-12);
	RooResolutionModel* tau_reso_model = new RooGaussModel(("tau_reso_model_"+name).c_str(), ("Resolution model for "+name).c_str(), *t, *reso_mean, *reso_sigma);//fake resolution model for now. Replace with actual one, from MC!
	RooDecay* decay_expo = new RooDecay(("bkgModel_t_"+name).c_str(), ("Exponential decay PDF for "+name).c_str(), *t, *tau, *tau_reso_model, RooDecay::SingleSided);
	return decay_expo;
};

void jpsik_pdf::freezeSignal(){

	w->var(("M2_shift_"+jpsik_pdf::name).c_str())->setConstant(true);
	w->var(("M3_shift_"+jpsik_pdf::name).c_str())->setConstant(true);

	w->var(("sigma1_"+jpsik_pdf::name).c_str())->setConstant(true);
	w->var(("sigma2_"+jpsik_pdf::name).c_str())->setConstant(true); 
	w->var(("sigma3_"+jpsik_pdf::name).c_str())->setConstant(true);

	w->var(("frac_gau1_"+jpsik_pdf::name).c_str())->setConstant(true);
	w->var(("frac_gau2_"+jpsik_pdf::name).c_str())->setConstant(true);
};

