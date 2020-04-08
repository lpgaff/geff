// A code to fit gamma-ray efficiency curves using
// multiple sources simulataneously and normalising
// them to each other to produce a single curve.
// Liam Gaffney - 28/11/2018

#ifndef __FitEff_cc__
#define __FitEff_cc__

#ifndef __FitEff_hh__
#include "FitEff.hh"
#endif

FitEff::FitEff( GlobalFitter gf, int Es, int Ee ) {
	
	// Assign fitter
	globalChi2 = &gf;
	
	// Set limits
	Estart = Es;
	Eend = Ee;
	
	// Check number of parameters in efficiency curve
	if( npoly > 10 ){
		
		cerr << "Why the hell would you want a 9th order polynomial?" << endl;
		return;
		
	}
	
}

FitEff::~FitEff(){

}

void FitEff::SetVariables( unsigned int n ) {
	
	// Set number of sources
	SetNsources( n );
	
	// Set starting parameters
	// default 5th order polynomial
	effpar.push_back( 2.75 );	// a
	effpar.push_back( -0.7 );	// b
	effpar.push_back( -0.04 );	// c
	effpar.push_back( 0.11 );	// d
	effpar.push_back( -0.04 );	// e
	
	// Check size of parameters etc
	npoly = effpar.size();
	neffpars = npoly + 1;
	nnormpars = nsources;
	npars = npoly + nnormpars;
	
	// Build arrays
	string letter;
	for( unsigned int i = 0; i < npoly; i++ ) {
		
		par0.push_back( effpar[i] );
		letter = 'a' + i;
		parname.push_back( letter );
		
	}
	
	for( unsigned int i = 0; i < nnormpars; i++ ) {
		
		par0.push_back( 1.0 );
		parname.push_back( "n_" + convertInt(i) );
		
	}
	
	// Result parameter arrays
	errArray.resize( npoly*neffpars+1 );
	parEffs.resize( neffpars );
	
	// Drawing things
	c1 = new TCanvas( "c1", "efficiency", 1200, 750 );
	mg = new TMultiGraph();
	leg = new TLegend( 0.7, 0.7, 0.9, 0.9 );
	
	return;
	
}


int FitEff::ReadData() {
	
	double a, b, c, d;

	// Efficiency data from file
	y.resize( nsources );
	x.resize( nsources );
	yerr.resize( nsources );
	xerr.resize( nsources );

	// Normalisation data (same size as efficiency data)
	norms.resize( nsources );
	normserr.resize( nsources );
	
	string line;
	stringstream line_ss;
	
	// Open and read efficiency files
	for( unsigned int i = 0; i < efiles.size(); i++ ) {
		
		ifile.open( efiles[i].c_str() );
		
		if( !ifile.is_open() ){
			
			cerr << "Could not open " << efiles[i] << endl;
			return 1;
			
		}
		
		else cout << "Opened efficiency file: " << efiles[i] << endl;
		
		getline( ifile, line );
		
		while( !ifile.eof() ){
			
			line_ss.str( line );
			
			if( line.substr( 0, 1) != "#" ) {
				
				line_ss >> a >> b >> c >> d;
				
				x[i].push_back( a );
				xerr[i].push_back( b );
				y[i].push_back( c );
				yerr[i].push_back( d );
				
			}
			
			getline( ifile, line );

		}
		
		ifile.close();
		
	}
	
	// Open and read normalisation files
	for( unsigned int i = 0; i < nfiles.size(); i++ ) {
		
		ifile.open( nfiles[i].c_str() );
		
		if( !ifile.is_open() ){
			
			cout << "Could not open " << nfiles[i] << endl;
			cout << "Assuming that you don't have any data for this source\n";
			
		}
		
		else cout << "Opened normalisation file: " << nfiles[i] << endl;
		
		ifile >> a >> b;
		
		while( !ifile.eof() ){
			
			norms[i].push_back( a );
			normserr[i].push_back( b );
			ifile >> a >> b;
			
		}
		
		ifile.close();
		
	}
	
	// If there are no normalisation files, fix to 1
	if( nfiles.size() == 0 ) {
		
		cout << "I didn't read any normalisation files\n";
		cout << "Fixing source 1 to have N=1 and contuining...\n";
		
		norms[0].push_back( 1.0 );
		normserr[0].push_back( 0.0 );

	}
	
	// Check consistency
	if( nsources == efiles.size() ) return 0;
	else {
		
		cerr << "Number of sources not equal to number of files\n";
		return 1;
	
	}
	
}

void FitEff::DoFit() {

	//////////////////////////
	// Perform a global fit //
	//////////////////////////

	// Define global chi2 function
	globalChi2->CopyData( x, xerr, y, yerr, norms, normserr );
	globalChi2->SetParameters( par0, parname );

	// Get fit result
	fitres = globalChi2->GetFitResult();

	// output to screen and file
	ofstream fitfile;
	fitfile.open( "fitresult.txt", ios::out );
	fitres.Print( std::cout );
	fitres.Print( fitfile );
	fitres.PrintCovMatrix( std::cout );
	fitres.PrintCovMatrix( fitfile );
	fitfile.close();

	// Get parameters and covariance
	// This ignores the normalisation constants
	for( unsigned int i = 0; i < npoly; i++ ) {
		
		parEffs[i] = fitres.Value(i);
		errArray[npoly*npoly+i] = fitres.Value(i);
		
		for( unsigned int j = 0; j < npoly; j++)
			errArray[i*npoly+j] = fitres.CovMatrix(i,j);

	}
	
	parEffs[npoly] = fitres.Value( npoly );
	errArray[npoly*neffpars] = fitres.Value( npoly );

	fEff = globalChi2->GetEffCurve( parEffs );
	fErr = globalChi2->GetErrCurve( errArray );
	
	return;
	
}

void FitEff::DrawResults( string outputfile ) {
	
	// Graphs for effiency function
	gFinal = new TGraph( fEff->GetXmax() - fEff->GetXmin() );
	gLow = new TGraph( fEff->GetXmax() - fEff->GetXmin() );
	gUpp = new TGraph( fEff->GetXmax() - fEff->GetXmin() );

	// Fill points on graphs
	double eff, err;
	cout << "E (keV)\tEff (%)\terror (%)\n";
	for( unsigned int i = fEff->GetXmin(); i < fEff->GetXmax(); i++ ) {
		
		eff = fEff->Eval( i ) * fitres.Value(npoly);
		err = fErr->Eval( i ) * fitres.Value(npoly);
		
		if( ( i % 100 == 0 && i < 800 ) || ( i % 500 == 0 && i >= 500 ) )
			cout << i << "\t" << eff << "\t" << err << endl;
		
		gFinal->SetPoint( i-fEff->GetXmin(), i, eff );
		gLow->SetPoint( i-fEff->GetXmin(), i, eff-err );
		gUpp->SetPoint( i-fEff->GetXmin(), i, eff+err );

	}
	
	// Graphs for the data
	gData.resize( nsources );
	for( unsigned int i = 0; i < nsources; i++ ) {
		
		gData[i] = new TGraphErrors( x[i].size() );
		
		gData[i]->SetLineColor(i+1);
		gData[i]->SetMarkerColor(i+1);
		gData[i]->SetMarkerStyle(24+i*2);
		gData[i]->SetMarkerSize(2);
		gData[i]->SetLineWidth(2);
		
		title = "source #" + convertInt(i+1);
		leg->AddEntry( gData[i], title.c_str(), "lep" );
		
		mg->Add( gData[i], "P" );
		
	}

	// Fill data points in graphs
	double scale;
	for( unsigned  int i = 0; i < nsources; i++ ) {
		
		for ( unsigned int j = 0; j < x[i].size(); j++ ) {
			
			scale = fitres.Value(npoly+i);
			gData[i]->SetPoint( j, x[i][j], y[i][j] * scale );
			gData[i]->SetPointError( j, xerr[i][j], yerr[i][j] * scale );
			
		}
		
	}
	
	// Some tidying up
	gFinal->SetLineColor(nsources+1);
	gFinal->SetLineWidth(2);
	gLow->SetLineStyle(10);
	gLow->SetLineColorAlpha(1,0.3);
	gLow->SetLineWidth(2);
	gUpp->SetLineStyle(10);
	gUpp->SetLineColorAlpha(1,0.3);
	gUpp->SetLineWidth(2);

	leg->AddEntry( gFinal, "Fit results", "l" );
	leg->AddEntry( gLow, "Upper/lower boundaries", "l" );

	mg->Add(gLow,"C");
	mg->Add(gUpp,"C");
	mg->Add(gFinal,"C");
	mg->Draw("A");
	//c1->SetLogx();
	mg->GetXaxis()->SetRangeUser( Estart, Eend );
	//c1->SetLogy();
	mg->GetYaxis()->SetRangeUser( gFinal->Eval(Estart) * 0.9, gFinal->GetMaximum() * 1.1 );
	//mg->GetYaxis()->UnZoom();
	c1->SetGridy();
	c1->SetGridx();

	leg->Draw("same");

	// Labels etc
	mg->GetXaxis()->SetTitle("Energy (keV)");
	mg->GetYaxis()->SetTitle("Efficiency (%)");
	mg->GetXaxis()->SetTickLength(0.015);
	mg->GetYaxis()->SetTickLength(0.015);
	mg->GetXaxis()->SetTitleSize(0.045);
	mg->GetYaxis()->SetTitleSize(0.045);
	mg->GetYaxis()->SetTitleOffset(0.75);
	
	c1->Update();

	c1->SaveAs( outputfile.c_str() );
	
	return;
	
}
#endif
