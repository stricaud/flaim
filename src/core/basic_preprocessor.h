//********************************************************************
//        ___           ___       ___                       ___     
//       /\  \         /\__\     /\  \          ___        /\__\    
//      /::\  \       /:/  /    /::\  \        /\  \      /::|  |   
//     /:/\:\  \     /:/  /    /:/\:\  \       \:\  \    /:|:|  |   
//    /::\~\:\  \   /:/  /    /::\~\:\  \      /::\__\  /:/|:|__|__ 
//   /:/\:\ \:\__\ /:/__/    /:/\:\ \:\__\  __/:/\/__/ /:/ |::::\__\
//   \/__\:\ \/__/ \:\  \    \/__\:\/:/  / /\/:/  /    \/__/~~/:/  /
//        \:\__\    \:\  \        \::/  /  \::/__/           /:/  / 
//         \/__/     \:\  \       /:/  /    \:\__\          /:/  /  
//                    \:\__\     /:/  /      \/__/         /:/  /   
//                     \/__/     \/__/                     \/__/    
//  
//
// Copyright © 2005-2006
// The Board of Trustees of the University of Illinois.
// All rights reserved.
// 
// Developed by:
//   LAIM Working Group
//   National Center for Supercomputing Applications
//   University of Illinois
// 
// Webpage:
//   http://flaim.ncsa.uiuc.edu/
// 
// ASCII Art generated using: http://www.network-science.de/ascii/
// 
//********************************************************************
// $Id: basic_preprocessor.h 534 2006-12-05 04:57:50Z kiran $
//
// Header file which defines an implementation class for a basic 
// preprocessor which does no sorting
//
//********************************************************************


#ifndef _BASIC_PREPROCESSOR_H
#define _BASIC_PREPROCESSOR_H

#include "preprocessor.h"
#include "flaim.hpp"
#include <list>
#include "listed_record.h"

class BasicPreprocessor : public Preprocessor
{
    public:
	BasicPreprocessor(LogParser *mod, int algos);
	~BasicPreprocessor();

	int preprocessLoop();
	bool preprocessFinished();

	// a PUBLIC MEMBER VARIABLE?!
	// what does this variable do????
	// An internal 'preprocessing finished' flag. Set to true when no more records are available.
	// False otherwise
	bool preprocessFlag;

    private:
	std::list<listedRecord> incoming;
	// bool preprocessFlag;
	int numAlgos;
	LogParser *module;
	clist_index_t currentID;

};

#endif // _BASIC_PREPROCESSOR_H
