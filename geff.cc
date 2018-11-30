// A code to fit gamma-ray efficiency curves using
// multiple sources simulataneously and normalising
// them to each other to produce a single curve.
// Liam Gaffney - 28/11/2018

#ifndef CXXOPTS_HPP_INCLUDED
#include "cxxopts.hh"
#endif

#ifndef __convert__
#include "convert.hh"
#endif

#ifndef __FitEff_hh__
#include "FitEff.hh"
#endif

#ifndef __GlobalFitter_hh__
#include "GlobalFitter.hh"
#endif

void PrintUsage( char* progname ) {
	
	cout << "\nUsage: \n" << progname;
	cout << " -e <eff1.dat> -n <norm1.dat> ... -e <effX.dat> - n<normX.dat>\n";
	cout << "\n effX.dat is the file containing the energy and efficiency\n";
	cout << " values for source number X. The format is 4 columns:\n";
	cout << "  Energy (keV) | Error (keV) | Efficiency (arb.) | Error (arb.)\n";
	cout << "\n normX.dat is the file containing experimentally determined\n";
	cout << " normalisation constants from arbitrary units to absolute %\n";
	cout << " for source number X. The format is 2 columns:\n";
	cout << "  Normalisation (%/arb.) | Error (%/arb.)\n\n";
	cout << " The efficiency curve is determined by a simultaneous fit to all\n";
	cout << " experimental data. In the case that no normalisation data are\n";
	cout << " given at all, then it is assumed to be equal to 1 for the first\n";
	cout << " source and the other sources are fitted to this. In case multiple\n";
	cout << " normalisations are given for a source, the global fit will minimise\n";
	cout << " using all these values plus the constraints from the efficiency\n";
	cout << " data of the other sources and their experimental normalisations.\n";
	cout << " If source data given under -e doesn't have experimental values\n";
	cout << " of normalisation constants, it's sufficient to simply type a\n";
	cout << " dummy filename, i.e it doesn't have to exist. However, the\n";
	cout << " ordering of the sources under -n must match those under -e.\n";
	
	cout << "\n" << progname << " --help\tfor help!\n\n\n";
	
	return;
	
}

int main( int argc, char* argv[] ) {
	
	// If the number of arguments are wrong, exit with usage
	if( argc < 2 ) {
		
		PrintUsage( argv[0] );
		return 0;
		
	}
	
	// Some variables
	string outputfile = "efficiency.pdf";
	
	// Options parser
	try {
		
		cxxopts::Options options( "geff",
								 "Program to fit gamma-ray efficiency curves with multiple sources" );
		
		options.add_options()
		( "e,eff", "efficiency file for source #X (repeat for each source)",
		 cxxopts::value<std::vector<std::string>>(), "<effX.dat>" )
		( "n,norm", "normalisation file for source #X (repeat for each source)",
		 cxxopts::value<std::vector<std::string>>(), "<normX.dat>" )
		( "o,out", "output filename, filetype taken from extension (pdf, png, svg, eps, root, C, etc)",
		 cxxopts::value<std::string>(), "<efficiency.pdf>" )
		( "h,help", "Print this help" )
		;
		
		auto optresult = options.parse(argc, argv);
		
		// Do help
		if( optresult.count( "help" ) ) {
			
			cout << options.help({""}) << endl;
			PrintUsage( argv[0] );
			return 0;
			
		}
		
		// Check for consistency
		if( optresult.count("e") == 0 ) {
			
			cerr << "I am going to need some data before I can fit it!\n";
			return 1;
			
		}
		
		else if( optresult.count("e") < optresult.count("n") ) {
			
			cerr << "Too many normalisation files\n";
			return 1;
			
		}
		
		else if( optresult.count("e") > optresult.count("n") ) {
			
			cout << "Not enough normalisation files\n";
			cout << "Continuing and assuming there are no normalisation data\n";
			cout << "for the sources that do not have files specified.\n";
			
		}
		
		// Check for output filename (use default if not)
		if( optresult.count("o") )
			outputfile = optresult["o"].as<std::string>();

		
		// If we get this far, create the FitEff and GlobalFitter instances
		GlobalFitter gf( 350., 1, 4500 );
		FitEff fe( gf );

		// Initialise with the number of sources
		fe.SetVariables( optresult.count("e") );
		
		// Add efficiency files
		for( unsigned int i = 0; i < optresult.count("e"); i++ )
			fe.AddEfile( optresult["e"].as<std::vector<std::string>>().at(i) );
		
		// Add normalisation files
		for( unsigned int i = 0; i < optresult.count("n"); i++ )
			fe.AddNfile( optresult["n"].as<std::vector<std::string>>().at(i) );

		// Read the data
		int readresult = fe.ReadData();
		if( readresult > 0 ) return readresult;
		
		// Run the fitting
		fe.DoFit();
		
		// Draw the results
		fe.DrawResults( outputfile );
		
	}
	
	// catch an error of parsing
	catch ( const cxxopts::OptionException& e ) {
		
		cerr << "error parsing options: " << e.what() << endl;
		return 1;
		
	}
	
	return 0;
	
}
