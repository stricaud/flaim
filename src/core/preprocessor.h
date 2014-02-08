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
// $Id: preprocessor.h 237 2006-07-27 10:31:56Z vdhar $
//
// Header file which defines the core functionality for a preprocessor for
// use with FLAIM-Core
//
//********************************************************************


#ifndef _PREPROCESSOR_H
#define _PREPROCESSOR_H

#include "flaim.hpp"
#include "listed_record.h"
#include <list>
#include <stdint.h>

class Preprocessor
{
    public:
	Preprocessor() {preprocessFlag = false;}
	Preprocessor(LogParser *mod, int algos) : module(mod), numAlgos(algos), currentID(0)
	{preprocessFlag = false;}
	virtual ~Preprocessor() {};

	virtual int preprocessLoop() = 0;
	virtual bool preprocessFinished() = 0;

	bool preprocessFlag;
	
    private:
	std::list<listedRecord> incoming;
	// bool preprocessFlag;
	LogParser *module;
	int numAlgos;

    protected:
	clist_index_t currentID;
};

#endif // _PREPROCESSOR_H
