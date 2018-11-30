// Fit gamma-ray efficiency with multiple sources
// Liam Gaffney (liam.gaffney) - 28/11/2018

#ifndef __FitEff_hh__
#define __FitEff_hh__

#include "TH1.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TMath.h"
#include "TRandom.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TMatrixTSym.h"
#include "TFile.h"
#include "TMultiGraph.h"
#include "Fit/FitResult.h"
#include "TLegend.h"

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

#ifndef __convert__
#include "convert.hh"
#endif

#ifndef __GlobalFitter_hh__
#include "GlobalFitter.hh"
#endif

using namespace std;

class FitEff {
	
public:
	
	// Initialisation functions
	FitEff( GlobalFitter gf, int Es, int Ee );
	~FitEff();
	
	// Setup functions
	inline void AddEfile( string filename ){
		efiles.push_back( filename );
		return;
	};
	
	inline void AddNfile( string filename ){
		nfiles.push_back( filename );
		return;
	};

	void SetVariables( unsigned int n );
	
	inline void SetNsources( unsigned int n ){
		nsources = n;
		return;
	};
	
	// Read data
	int ReadData();
	
	// Do fitting
	void DoFit();
	
	// Draw things
	void DrawResults( string outputfile );

private:
	
	// Files for efficiency and normalisation
	ifstream ifile;
	vector<string> efiles, nfiles;
	
	// Vectors for reading in data
	vector< vector<double> > y;
	vector< vector<double> > x;
	vector< vector<double> > yerr;
	vector< vector<double> > xerr;

	// Data for normalisation
	vector< vector<double> > norms;
	vector< vector<double> > normserr;

	// Number of parameters
	unsigned int nsources;
	unsigned int neffpars;
	unsigned int npoly;
	unsigned int nnormpars;
	unsigned int npars;
	
	// Efficiency curves
	TF1 *fEff, *fErr;

	// Default variables
	int Estart;
	int Eend;

	// Efficiency parameters
	vector<double> effpar;
	vector<double> par0;
	vector<string> parname;
	vector<double> errArray;
	vector<double> parEffs;

	// Fit results
	GlobalFitter *globalChi2;
	ROOT::Fit::FitResult fitres;

	// Drawing things
	TCanvas *c1;
	vector< TGraphErrors* > gData;
	TGraph *gFinal, *gLow, *gUpp;
	TMultiGraph *mg;
	TLegend* leg;
	string title;

};
#endif
