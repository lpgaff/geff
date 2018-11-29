// Header file to convert integers etc to strings
// Liam Gaffney (liam.gaffney@cern.ch) - 28/11/2018

#ifndef __convert__
#define __convert__

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

inline std::string convertInt( int number ) {
	
	std::stringstream ss;
	ss << number;
	return ss.str();
	
}

inline std::string convertFloat( float number, int precision = 3 ) {
	
	std::stringstream ss;
	ss << std::setprecision( precision ) << number;
	return ss.str();
	
}
#endif
