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
// $Id: basic_preprocessor.cpp 1002 2007-12-13 21:31:34Z byoo3 $
//
// Implementation of a basic FLAIM-Core preprocessor which has no
// support for sorting algorithms
//
//********************************************************************


#include "basic_preprocessor.h"
#include "listed_record.h"
#include "flaim.hpp"
#include "core_globals.h"
#include <list>
#include <pthread.h>

// #define DEBUG // comment this out to remove debugging messages

// parameter which gives the number of records to get at a time, if possible

// BasicPreprocessor(LogParser *mod, int algos)
// Constructor
//
// Parameters: mod, a pointer to the module being used; algos, the number of
// algorithms to be applied
// Returns: Nothing
// Modifies: Nothing
BasicPreprocessor::BasicPreprocessor(LogParser *mod, int algos) :
    numAlgos(algos), preprocessFlag(false), module(mod), currentID(0) {
        // initializer list used; no code
        cerr << "*** Preprocessor created: " << numAlgos << " algorithms in use" << endl;
    }
    
// ~BasicPreprocessor()
// Destructor
    BasicPreprocessor::~BasicPreprocessor() {
        // nothing to destroy
    }
    
// int preprocessLoop()
// Performs preprocessing duties; should be invoked by the main thread
//
// Parameters: None
// Returns: 0 on successful exit, 1 on failure
// Modifies: Reads data from the module and passes it to the core list
// Notes: Only one thread should be executing this function at a time
    int BasicPreprocessor::preprocessLoop() {
        // if (verbose)
        
        fprintf(stderr, "thread: preprocessor: Starting preprocessor.\n");
        
        Record* r;
        listedRecord l;
        int rcdCount=0, rcdNum;
        
        // while there are still records remaining...
        while (1) {
            if (verbose)
                fprintf(stderr, "thread: preprocessor: getting the CPU to run.\n");
            
            module->getRecordSeq(rcdNum);
            if(module->isEndofLog() && (rcdNum == rcdCount))  {
                break;
            }
            
            // get records until rcdsToGet records have been cached or the
            // end of the log has been reached
            for (int i = 0; !module->isEndofLog() && i < CACHE_RECORD_SIZE 
                    && incoming.size() < 10 * CACHE_RECORD_SIZE; i++) {
                if (NULL == (r = module->getRecord())) {
                    fprintf(stderr, "thread: preprocessor: stopping preprocessor due to abnormal data.\n");
                    preprocessFlag = true;
                    module->returnCode = 1;
                    return 0;
                }
                
                // fill in the listedRecord structure, initializing its reference count
                l.theRecord = r;
                l.refCount = numAlgos;
                l.id = currentID;
                
                // append the structure to the "incoming" list
                incoming.push_back(l);
                
                ++currentID;
            }
            
            if (verbose)
                fprintf(stderr, "thread: preprocessor: incoming buffered %d records\n", incoming.size());
            if (verbose)
                fprintf(stderr, "thread: preprocessor: Current record number: %d\n", currentID);
            
            // once we have some records, append them to the core list
            
            // First try to lock the read/write lock for reading.. w/o blocking if unavailable.
            if (verbose) fprintf(stderr, "thread: preprocessor -  Trying WR lock\n");
            
            if (0 != coreList.tryrdlock()) //not get the rdlock
            {
                if (verbose) fprintf(stderr, "thread: preprocessor: UNSUCCESSFULL tryrdlock()\n");
                
                // try to lock the read/write lock for writing....
                if (verbose) fprintf(stderr, "thread: preprocessor -  Trying WR lock\n");
                
                if (0 != coreList.trywrlock()) {
                    if (verbose) fprintf(stderr, "thread: preprocessor: UNSUCCESSFULL trywrlock()\n");
                    continue;   //not get the lock
                }
                
                if (verbose) fprintf(stderr, "thread: preprocessor - THIS CAN NEVER HAPPEN!\n");
                
            }
            else //get the rdlock
            {
                if (verbose) fprintf(stderr, "thread: preprocessor: successfull tryrdlock()\n");
                
                coreList.unlock();
                
                if (verbose) fprintf(stderr, "thread: preprocessor: trying wrlock - hopefully will get the lock!\n");
                coreList.wrlock();
                
                if (verbose) fprintf(stderr, "thread: preprocessor:  GOT THE WR LOCK!\n");
            }
            
            /*
             * if (0 != coreList.trywrlock()) {
             * if (verbose) fprintf(stderr,"thread: preprocessor: UNSUCCESSFULL trywrlock()\n");
             * continue;
             * } */         
            
            if (verbose)
                fprintf(stderr, "thread: preprocessor: coreList BEFORE incoming dumping - %d records\n", coreList.size());
            
            // put the incoming list onto the end of the core list, clearing
            // the incoming list in the process
            while (!incoming.empty() && coreList.size() < 10 * CACHE_RECORD_SIZE) {
                coreList.push_back(incoming.front());
                incoming.pop_front();
                rcdCount++;
            }
            
            if (verbose)
                fprintf(stderr, "thread: preprocessor: coreList AFTER incoming dumping - %d records\n", coreList.size());
            
            // release the lock and inform the anonyEngine that new records were put in the coreList
            if (verbose) fprintf(stderr, "thread: preprocessor:  trying to unlock!\n");
            coreList.unlock();            
            
            if (verbose) fprintf(stderr, "thread: preprocessor:  signaling preRecordsAvail!!\n");
            
            //setting the predicate preRecordsBool and signalling the anonyEngine thread
            pthread_mutex_lock(&preRecordsAvailMutex);
            preRecordsBool = true;
            pthread_cond_signal(&preRecordsAvail);
            pthread_mutex_unlock(&preRecordsAvailMutex);
            
#ifdef DEBUG
cerr << "preprocessor: just dumped records, coreList size is " << coreList.size() << endl;
#endif

if (verbose)
    fprintf(stderr, "thread: preprocessor: Release the CPU after working for a while. \n");

            // pthread_yield();
            sched_yield();

        } // while records available
        
        if(verbose)
            cerr << "thread: preprocessor: rcdCount = " << rcdCount << endl;
        
        if (verbose)
            cerr << "thread: preprocessor: setting preprocessFlag to true" << endl;
        
        // set the internal 'preprocessor thread finished' flag before exit
        preprocessFlag = true;
        
        return 0;
    }
    
    
// bool preprocessFinished()
// Inform anyone who asks whether preprocessing is finished
//
// parameters: none
// Returns: true if preprocessing is done, false otherwise
// Modifies: Nothing
    bool BasicPreprocessor::preprocessFinished() {
        return preprocessFlag;
    }
