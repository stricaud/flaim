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
// $Id: core_globals.h 664 2007-02-27 19:20:39Z xluo1 $
//
// Header file which declares the core list of records and the 
// reader-writer lock that protects them. This file MUST be included 
// by everyone who intends // to use this list, including the 
// preprocessor, postprocessor, and anonymization manager!
//
//********************************************************************


#ifndef _CORE_GLOBALS_H
#define _CORE_GLOBALS_H

#include <pthread.h>

#include "preprocessor.h"
#include "postprocessor.h"
#include "listed_record.h"
#include "core_list.h"
#include "flaim_core.hpp"

// STL list which is used to contain records that are being anonymized
// The preprocessor adds to the end of this list, and the postprocessor
// removes elements from it (presumably from the head)
extern CoreList coreList;


// global preprocessor pointer which is filled in when the preprocessor is
// created and initialized. This provides base class functionality which allows
// other threads (in particular, the anonymization manager) to ask the
// preprocessor if it is done preprocessing, i.e., no more records are incoming
extern Preprocessor *pre;

// global postprocessor pointer which is filled in when the postprocessor
// is created and initialized
extern Postprocessor *post;

// condition variable used to signal a sleeping postprocessor
extern pthread_cond_t postRecordsAvail;
extern pthread_mutex_t postRecordsAvailMutex;

//predicate for checking on the postRecordsAvail
extern bool postRecordsBool;

// condition variable used to tell anonymization algorithms that more data is available
extern pthread_cond_t preRecordsAvail;
extern pthread_mutex_t preRecordsAvailMutex;

//predicate for checking on the preRecordsAvail
extern bool preRecordsBool;


//the number of records put into incoming queue by the module each time 
extern const int CACHE_RECORD_SIZE;

//the time interval for waiting on a condition variable before timeout  
extern const long WAIT_TIME_SECONDS; 

#endif
