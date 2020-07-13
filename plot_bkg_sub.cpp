#include "bkg_subtr_includes.h"
using namespace std;


void plot_bkg_sub_tau(TChain* data, TChain* MC, const string cut, const vector<float> sig_m_range, const vector<float> bkg_m_range, float sig_frac){

	string full_cut_sig = cut+" && bkmm_jpsimc_mass>"+to_string(sig_m_range[0])+" && bkmm_jpsimc_mass<"+to_string(sig_m_range[1]);//we should look in the signal mass window
	string full_cut_bkg = cut+" && bkmm_jpsimc_mass>"+to_string(bkg_m_range[0])+" && bkmm_jpsimc_mass<"+to_string(bkg_m_range[1]);//we should look in the signal mass window

	TH1F* h_SB = new TH1F("h_SB", "S+B histogram", 50, 0,8);
	TH1F* h_B = new TH1F("h_B", "B histogram", 50, 0,8);
	TH1F* h_MC = new TH1F("h_MC", "MC (S) histogram", 50, 0,8);
	data->Draw("1e12*bkmm_jpsimc_tau>>h_SB", full_cut_sig.c_str(), "goff");
	data->Draw("1e12*bkmm_jpsimc_tau>>h_B", full_cut_bkg.c_str(), "goff");
	MC->Draw("1e12*bkmm_jpsimc_tau>>h_MC", full_cut_sig.c_str(), "goff");

	//normalize!
	h_SB->Scale(1./h_SB->Integral());
	h_B->Scale(sig_frac/h_B->Integral());

	TH1F* h_S = (TH1F*)h_SB->Clone("h_S");
	h_S->Add(h_B, -1);//subtract the background from the S+B
	h_MC->Scale(h_S->Integral()/h_MC->Integral());

	//now draw the three in the same canvas
	TCanvas c("canvas");

	h_SB->SetMarkerStyle(8);
	h_SB->SetMarkerSize(0.7);
	h_SB->SetMarkerColor(1);
	h_SB->SetLineColor(1);
	h_SB->SetLineWidth(1);
	h_SB->GetXaxis()->SetTitle("decay time [ps]");
	h_SB->GetYaxis()->SetTitle("a.u.");
	h_SB->Draw("");

	h_MC->SetLineColor(1);
	h_MC->SetFillColor(16);
	h_MC->SetFillStyle(1001);
	h_MC->Draw("same&hist");

	h_B->SetMarkerStyle(8);
	h_B->SetMarkerSize(0.7);
	h_B->SetMarkerColor(2);
	h_B->SetLineColor(2);
	h_B->SetLineWidth(1);
	h_B->Draw("same");

	h_S->SetMarkerStyle(8);
	h_S->SetMarkerSize(0.7);
	h_S->SetMarkerColor(9);
	h_S->SetLineColor(9);
	h_S->SetLineWidth(1);
	h_S->Draw("same");

	//add a legend
	TLegend leg(0.7, 0.75, 0.9, 0.9);
	leg.AddEntry(h_MC, "signal MC", "f");
	leg.AddEntry(h_SB, "data (S+B)","lp");
	leg.AddEntry(h_B,  "data (B)",  "lp");
	leg.AddEntry(h_S,  "data (S)",  "lp");

	leg.Draw();

	// //other cosmetic changes
	gStyle->SetOptTitle(0);
	gStyle->SetOptStat(false);
	c.SetLogy();
	c.SaveAs("plots/lifetimeSB.pdf");
}