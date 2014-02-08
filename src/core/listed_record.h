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
// $Id: listed_record.h 633 2007-01-18 19:50:56Z xluo1 $
//
// Basic header which defines the listedRecord structure
//
//********************************************************************


#ifndef _LISTED_RECORD_H
#define _LISTED_RECORD_H

#include "record.h"
#include <pthread.h>

typedef uint32_t clist_index_t;

typedef struct listedRecord
{	
    // Pointer to record
    Record * theRecord;

    // the number of anonymization algorithms applied for this record
    int refCount;
    
    // Lock on the refCount variable
    pthread_mutex_t countLock;
    
    // This is the sequence number of a record - the unique identifier. The core list is accessed through
    // this id and only this id.
    clist_index_t id;
} listedRecord;

#endif // _LISTED_RECORD_H
