// Fitter for the efficiency curves

#ifndef __GlobalFitter_cc__
#define __GlobalFitter_cc__

#ifndef __GlobalFitter_hh__
#include "GlobalFitter.hh"
#endif

#include "TCanvas.h"

void GlobalFitter::CopyData( vector< vector<double> > _x,
							vector< vector<double> > _xerr,
							vector< vector<double> > _y,
							vector< vector<double> > _yerr,
							vector< vector<double> > _norms,
							vector< vector<double> > _normserr ) {
	
	x = _x;
	xerr = _xerr;
	y = _y;
	yerr = _yerr;
	
	norms = _norms;
	normserr = _normserr;

	nsources = x.size();
	
	BinData();

	return;
	
}

TF1* GlobalFitter::GetErrCurve( vector<double> _par ) {

	fErr->SetParameters( _par.data() );
	
	return fErr;
	
}

TF1* GlobalFitter::GetEffCurve( vector<double> _par ) {
	
	fEff->SetParameters( _par.data() );
	
	return fEff;
	
}

void GlobalFitter::BinData() {
	
	// Data initialisers and filling
	range.SetRange( Estart, Eend );
	effi_data.resize( nsources );
	norm_data.resize( nsources );

	for ( unsigned int i = 0; i < nsources; i++ ) {
		
		effi_data[i] = make_shared< ROOT::Fit::BinData >( opt, range,
						x[i].size(), 1, ROOT::Fit::BinData::kCoordError );
		
		for ( unsigned int j = 0; j < x[i].size(); j++ ) {
			
			effi_data[i]->Add( x[i][j], y[i][j], xerr[i][j], yerr[i][j] );
			
		}
		
	}
	
	for ( unsigned int i = 0; i < nsources; i++ ) {
		
		norm_data[i] = make_shared< ROOT::Fit::BinData >( opt, range,
						norms[i].size(), 1, ROOT::Fit::BinData::kValueError );
		
		for ( unsigned int j = 0; j < norms[i].size(); j++ ) {
			
			norm_data[i]->Add( 1.0, norms[i][j], normserr[i][j] );
			
		}
		
	}
	
	// Get data size
	data_size = 0;
	for( unsigned int i = 0; i < nsources; i++ ) {
		
		data_size += effi_data[i]->Size();
		data_size += norm_data[i]->Size();
		
	}
	
	return;
	
}

void GlobalFitter::SetParameters( vector<double> _par, vector<string> _parname ) {
	
	par0 = _par;
	parname = _parname;
	
	npars = par0.size();
	npoly = npars - nsources;
	neffpars = npoly + 1;
	
	// write the efficiency parameters
	for( unsigned int i = 0; i < neffpars; i++ )
		effpar.push_back( par0[i] );
	
	// adjust the normalisation
	for( unsigned int i = npoly; i < npars; i++ )
		par0[i] = norms[0][0];
	
	// Make individual fits
	CreateIndividualFits();
	
	return;
	
}

void GlobalFitter::CreateIndividualFits() {
	
	// Function classes
	eff_func = new ExpFit( E0, neffpars );
	err_func = new ExpFitErr( E0, neffpars );
	norm_func = new NormFunc();
	
	// Resize vectors
	fEffi.resize( nsources );
	wEffi.resize( nsources );
	fNorm.resize( nsources );
	wNorm.resize( nsources );
	effi_fcn.resize( nsources );
	norm_fcn.resize( nsources );
	
	// Fit functions for all data
	string name;
	for( unsigned int i = 0; i < nsources; i++ ) {
		
		name = "fEffi_" + convertInt(i+1);
		fEffi[i] = make_shared< TF1 >( name.c_str(), eff_func, Estart, Eend, neffpars );
		wEffi[i] = make_shared< ROOT::Math::WrappedMultiTF1 >( *fEffi[i], 1 );
		
		name = "fNorm_" + convertInt(i+1);
		fNorm[i] = make_shared< TF1 >( name.c_str(), norm_func, -1e6, 1e6, 1 );
		wNorm[i] = make_shared< ROOT::Math::WrappedMultiTF1 >( *fNorm[i], 1 );

	}
	
	// Chi2 functions for all data
	for( unsigned int i = 0; i < nsources; i++ ) {
		
		effi_fcn[i] = new ROOT::Fit::Chi2Function( effi_data[i], wEffi[i] );
		norm_fcn[i] = new ROOT::Fit::Chi2Function( norm_data[i], wNorm[i] );
		
	}
	
	// Efficiency and error functions
	fEff = new TF1( "fEff", eff_func, Estart, Eend, neffpars );
	fErr = new TF1( "fErr", err_func, Estart, Eend, npoly*neffpars+1 );
	
	return;
	
}

ROOT::Fit::FitResult GlobalFitter::GetFitResult() {
	
	// Define fitter
	Chi2Fit chi2fitter = Chi2Fit( effi_fcn, norm_fcn, nsources, npars );
	ROOT::Fit::Fitter fitter;
	fitter.Config().SetParamsSettings( npars, par0.data() );
	
	// Get initial chisq
	double chisq0 = chi2fitter.EvalChi2( par0.data() );
	cout << "Initial chisq = " << chisq0 << endl;

	// set parameter names
	for( unsigned int i = 0; i < npars; i++ ) {
		
		fitter.Config().ParSettings(i).SetName( parname[i].c_str() );
		
	}
	
	// Fitter options
	fitter.Config().SetMinimizer( "Minuit2", "Migrad" );
	//fitter.Config().MinimizerOptions().SetPrintLevel(1);
	//fitter.Config().SetMinosErrors( true ); // Perform MINOS error analysis, i.e. correlations to all parameters
	//fitter.Config().MinimizerOptions().SetMaxIterations(1);
	//fitter.Config().MinimizerOptions().SetMaxFunctionCalls(1);


	// Do fit of global chi2 fucntion
	fitter.FitFCN( npars, chi2fitter, 0, data_size, true );

	// normalise the errors to chi2/NDF = 1
	ROOT::Fit::FitResult fitres = fitter.Result();
	//fitres.NormalizeErrors();
	
	return fitres;
	
}

double GlobalFitter::ExpFit::operator()( double *x, double *par ) {
	
	unsigned int _npoly = _neffpars - 1;
	
	Double_t f = par[0];
	
	for( unsigned int i = 1; i < _npoly; i++ )
		f += par[i] * TMath::Power( TMath::Log( x[0] / _E0 ), (double)i );
	
	f = TMath::Exp(f);
	
	return f / par[_npoly];
	
}

double GlobalFitter::ExpFitErr::operator()( double *x, double *par ) {
	
	unsigned int _npoly = _neffpars - 1;
	
	ExpFit expfcn( _E0, _neffpars );
	vector<double> _effpar;
	_effpar.resize( _neffpars );
	for( unsigned int m = 0; m < _neffpars; m++ )
		_effpar[m] = par[_npoly*_npoly+m];
	
	Double_t f = 0, g = 0;
	Double_t h = TMath::Power( expfcn( x, _effpar.data() ), 2.0 );
	
	for( unsigned int m = 0; m < _npoly; m++ ) {
		
		for( unsigned int n = 0; n < _npoly; n++ ) {
			
			g = h * TMath::Power( TMath::Log( x[0] / _E0 ), (double)m );
			g *= TMath::Power( TMath::Log( x[0] / _E0 ), (double)n );
			g *= par[ m*_npoly + n ];
			f += g;
			
		}
		
	}
	
	return TMath::Sqrt(f);
	
}

double GlobalFitter::NormFunc::operator()( double *x, double *par ) {
	
	return par[0];
	
}
#endif
