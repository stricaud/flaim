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
// $Id: basic_postprocessor.h 237 2006-07-27 10:31:56Z vdhar $
//
// Header file which defines an implementation class for a basic 
// postprocessor which does no sorting
//
//********************************************************************


#ifndef _BASIC_POSTPROCESSOR_H
#define _BASIC_POST_PROCESSOR_H

#include "postprocessor.h"
#include "flaim.hpp"
#include <list>
#include "listed_record.h"
#include <ctime>

class BasicPostprocessor : public Postprocessor
{
    public:
	BasicPostprocessor(LogParser *mod);
	~BasicPostprocessor();

	int postprocessLoop();

    private:
	std::list<listedRecord> outgoing;
	LogParser *module;
	struct timespec timeout;
};

#endif // _BASIC_POSTPROCESSOR_H
