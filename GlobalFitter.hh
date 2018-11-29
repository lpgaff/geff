// Fitter for the efficiency curves

#ifndef __GlobalFitter_hh__
#define __GlobalFitter_hh__

#include "Fit/Fitter.h"
#include "Fit/BinData.h"
#include "Fit/Chi2FCN.h"
#include "Math/WrappedMultiTF1.h"
#include "Fit/FitResult.h"
#include "TF1.h"
#include "TMath.h"

#ifndef __convert__
#include "convert.hh"
#endif

#include <string>
#include <vector>

using namespace std;

class GlobalFitter {
	
public:
	
	GlobalFitter( double _E0 = 350., int _Estart = 1, int _Eend = 4500 ){
		
		E0 = _E0;
		Estart = _Estart;
		Eend = _Eend;
		
	};
	virtual ~GlobalFitter(){;};
	
	void CopyData( vector< vector<double> > _x,
				  vector< vector<double> > _xerr,
				  vector< vector<double> > _y,
				  vector< vector<double> > _yerr,
				  vector< vector<double> > _norms,
				  vector< vector<double> > _normserr );
	
	void BinData();
	void SetParameters( vector<double> _par, vector<string> _parname );
	void CreateIndividualFits();

	inline unsigned long GetDataSize(){ return data_size; };
	
	TF1* GetEffCurve( vector<double> _par );
	TF1* GetErrCurve( vector<double> _par );
	
	ROOT::Fit::FitResult GetFitResult();
	
private:
	
	// Vectors for holding in data
	vector< vector<double> > y;
	vector< vector<double> > x;
	vector< vector<double> > yerr;
	vector< vector<double> > xerr;
	
	// Data for normalisation
	vector< vector<double> > norms;
	vector< vector<double> > normserr;
	
	// Binned data
	ROOT::Fit::DataOptions opt;
	ROOT::Fit::DataRange range;
	vector< shared_ptr< ROOT::Fit::BinData > > effi_data;
	vector< shared_ptr< ROOT::Fit::BinData > > norm_data;
	int data_size;
	
	// parameters
	vector<double> par0;
	vector<string> parname;
	vector<double> effpar;
	unsigned int npars;
	unsigned int nsources;
	unsigned int neffpars;
	unsigned int npoly;

	// Default variables
	double E0;
	int Estart;
	int Eend;
	
	// Fit functions
	TF1 *fEff, *fErr;
	vector< shared_ptr< TF1 > > fEffi;
	vector< shared_ptr< TF1 > > fNorm;
	vector< shared_ptr< ROOT::Math::WrappedMultiTF1 > > wEffi;
	vector< shared_ptr< ROOT::Math::WrappedMultiTF1 > > wNorm;
	vector< ROOT::Fit::Chi2Function* > effi_fcn;
	vector< ROOT::Fit::Chi2Function* > norm_fcn;

	struct Chi2Fit {
		
		Chi2Fit( vector< ROOT::Fit::Chi2Function* > & effi_inp,
				vector< ROOT::Fit::Chi2Function* > & norm_inp,
				unsigned int _nsources, unsigned int _npars ) {
			
			for( unsigned int i = 0; i < effi_inp.size(); i++ ) {

				effi_vec.push_back( effi_inp[i] );
				cout << "source #" << i << " has " << effi_vec[i]->NPoints();
				cout << " efficiency data points and " << effi_vec[i]->NDim();
				cout << " free parameters\n";

			}
			
			for( unsigned int i = 0; i < norm_inp.size(); i++ ) {
				
				norm_vec.push_back( norm_inp[i] );
				cout << "source #" << i << " has " << norm_vec[i]->NPoints();
				cout << " normalisation data points\n";

			}
			
			nsources = _nsources;
			npars = _npars;
			
		}
		
		double operator()( const double* p ){
			
			chisq = 0;
			npoly = npars - nsources;
			
			vector< vector<double> > pf;
			vector< vector<double> > pn;

			vector<double> dummy_pf;
			vector<double> dummy_pn;

			for( unsigned int i = 0; i < nsources; i++ ) {
				
				for( unsigned int j = 0; j < npoly; j++ )
					dummy_pf.push_back( p[j] );
				
				dummy_pf.push_back( p[npoly+i] );
				dummy_pn.push_back( p[npoly+i] );

				pf.push_back( dummy_pf );
				dummy_pf.clear();

				pn.push_back( dummy_pn );
				dummy_pn.clear();

			}
			
			// Calculate chisq
			for( unsigned int i = 0; i < effi_vec.size(); i++ )
				chisq += ( *effi_vec[i] )( pf[i].data() );
			
			for( unsigned int i = 0; i < norm_vec.size(); i++ )
				chisq += ( *norm_vec[i] )( pn[i].data() );
			
			return chisq;
			
		};
		
		vector< const ROOT::Fit::Chi2Function* > effi_vec;
		vector< const ROOT::Fit::Chi2Function* > norm_vec;
		
		double EvalChi2( const double* p ){ return (*this)(p); };

		unsigned int nsources;
		unsigned int npars;
		unsigned int npoly;
		double chisq;

	};
		
	// Function classes
	class ExpFit {
		
	public:
		
		ExpFit( double _E0_, unsigned int _neffpars_ ){
			_E0 = _E0_;
			_neffpars = _neffpars_;
		};
		double operator()( double *x, double *par );
		double Eval( double *x, double *par ){ return (*this)( x, par ); };

	private:
		
		double _E0;
		unsigned int _neffpars;

	};
	
	class ExpFitErr {
		
	public:
		
		ExpFitErr( double _E0_, unsigned int _neffpars_ ){
			_E0 = _E0_;
			_neffpars = _neffpars_;
		};
		double operator()( double *x, double *par );
		double Eval( double *x, double *par ){ return (*this)( x, par ); };

	private:
		
		double _E0;
		unsigned int _neffpars;
		
	};
	
	class NormFunc {
		
	public:
		
		NormFunc(){;};
		double operator()( double *x, double *par );
		double Eval( double *x, double *par ){ return (*this)( x, par ); };
		
	};
	
	// Function classes
	ExpFit *eff_func;
	ExpFitErr *err_func;
	NormFunc *norm_func;
	
};
#endif
