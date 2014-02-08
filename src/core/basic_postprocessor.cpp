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
// $Id: basic_postprocessor.cpp 869 2007-07-06 06:43:50Z xluo1 $
//
// Implementation of a no-sorting postprocessor for FLAIM-Core
//
//********************************************************************


#include "basic_postprocessor.h"
#include "flaim.hpp"
#include "listed_record.h"
#include "core_globals.h"
#include <ctime>
#include <list>
#include <pthread.h>


//#define DEBUG // comment this out if you don't want verbose messages for
	      // debugging

// BasicPostprocessor(LogParser *mod)
// Constructor
//
// Parameters: mod, a pointer to the module being used
// Returns: Nothing
// Modifies: Nothing
BasicPostprocessor::BasicPostprocessor(LogParser *mod) : module(mod)
{
    cerr << "*** Postprocessor created." << endl;
}

// ~BasicPostprocessor()
// Destructor
BasicPostprocessor::~BasicPostprocessor() {}


// int postprocessLoop()
// Main program loop for a postprocessing thread
//
// Parameters: None
// Returns: 0 on successful completion, nonzero otherwise
// Modifies: Removes data from the core list and writes it back out to module
// Notes: Only one thread should be executing this function at a time
int BasicPostprocessor::postprocessLoop()
{    
      fprintf(stderr,"thread: postprocessor: Starting postprocessor.\n");

    int status;
    int theCount = -1;
    int rcdCount = 0;


    while(1)  //postprocessor thread loop
    {
       if (verbose) 
         fprintf(stderr,"thread: postprocessor: getting the CPU to run.\n");

	// if there are still records in the outgoing queue, send them first
	if(!outgoing.empty())
	{
	    int cntr = 0;
	    std::list<listedRecord>::iterator iter = outgoing.begin();

	    // put all the records
	    while(iter != outgoing.end())
	    {
#ifdef DEBUG
              cerr << "postprocessor: so far have dumped "<< cntr<< "records from outgoing queue, leaving - "
              <<outgoing.size()<<" records remaining" << endl;
#endif

		if(module->putRecord((*iter).theRecord))
		{
		   if (verbose) 
                      fprintf(stderr, "thread: postprocessor: could'nt putRecord. \n");

		    return 1;
		}

		++iter;
		cntr++;
                rcdCount++;
            // cerr << "rcdCount = " << rcdCount << endl;
	    }

	    // clear the list
#ifdef DEBUG
	    cerr << "Clearing outgoing queue....\n";
#endif
	    outgoing.clear();
	} // end of if 

      
           //first, check if all the work is done, if so exit

          if (verbose) fprintf(stderr,"thread: postprocessor:  Trying to get wr lock\n");
	 //   coreList.wrlock();
   
           if(0 != coreList.trywrlock()) {
	    	if (verbose) fprintf(stderr,"thread: postprocessor: UNSUCCESSFULL trywrlock()\n");
			continue;
	    }
         

           if(coreList.empty()){
                         
                 if (verbose) fprintf(stderr,"thread: postprocessor:  Trying to get unlock\n");
	    	
	          coreList.unlock();

                                        
                  // cerr << "thread: postprocessor: rcdCount = " << rcdCount << endl;

              if (pre->preprocessFinished()) //all the work is done
	      {	                      	              
                 if(verbose) 
                   fprintf(stderr,"thread: postprocessor:  all the work is done.\n");

                 if(verbose)
                   cerr << "thread: postprocessor: rcdCount = " << rcdCount << endl;

		    break; // exit from the postProcessor thread loop
	      }
              else{    //possibility of more records
	                                      

               //wait on the condition variable if there are no new records
	       if (verbose) 
                  fprintf(stderr,"thread: postprocessor:  waiting on postRecordsAvail\n");
	
	        pthread_mutex_lock(&postRecordsAvailMutex);

                //set the timeout for condtion wait
            //   timeout.tv_sec = time(NULL) + WAIT_TIME_SECONDS;
             //  timeout.tv_nsec = 0;

	       while (postRecordsBool != true){
                   status = pthread_cond_wait(&postRecordsAvail,&postRecordsAvailMutex);
                  // status = pthread_cond_timedwait(&postRecordsAvail,&postRecordsAvailMutex, &timeout);
                   
                   //error happened
                    if (status){
                         fprintf(stderr,"thread: postprocessor:  error in wait on postRecordsAvail!!\n");
                         pthread_mutex_unlock(&postRecordsAvailMutex);
                         return 1;
                    } 
              }

                if (verbose) 
                  fprintf(stderr,"thread: postprocessor:  RETURNING from wait on postRecordsAvail!!\n");

                 // retest the predicate
                 if (postRecordsBool == true) {
                   if (verbose) 
                      fprintf(stderr,"thread: postprocessor:  postRecordsAvail is true.\n");                       
                 }
                 else{
                    if (verbose)
                       fprintf(stderr,"thread: postprocessor:  postRecordsAvail is false (spurious wakeup).\n");             
                 }

                 //unlock the associated mutex
                 pthread_mutex_unlock(&postRecordsAvailMutex); 
                 continue;
                            
              }
         }
         else { // the coreList is not empty

                if (verbose) {                 
                    if (postRecordsBool == true) 
                       fprintf(stderr,"thread: postprocessor:  postRecordsAvail is true.\n");
                    else
                       fprintf(stderr,"thread: postprocessor:  postRecordsAvail is false.\n");             
                 }

   
               pthread_mutex_lock(&postRecordsAvailMutex);

                //set the timeout for condtion wait
              // timeout.tv_sec = time(NULL) + WAIT_TIME_SECONDS;
               //timeout.tv_nsec = 0;

               if(postRecordsBool != true) {

                  if (verbose) fprintf(stderr,"thread: postprocessor:  Trying to get unlock\n");
                      coreList.unlock(); 
                 
                  if (verbose)
                    fprintf(stderr,"thread: postprocessor:  waiting on postRecordsAvail\n");
            

	          while (postRecordsBool != true){

                   status = pthread_cond_wait(&postRecordsAvail,&postRecordsAvailMutex);
                  // status = pthread_cond_timedwait(&postRecordsAvail,&postRecordsAvailMutex, &timeout);
                   
                   //timeout or error happened
                    if (status){
                         fprintf(stderr,"thread: postprocessor:  error in wait on postRecordsAvail!!\n");
                         pthread_mutex_unlock(&postRecordsAvailMutex);
                         return 1;
                    } 
                  }

               if (verbose) 
                   fprintf(stderr,"thread: postprocessor:  RETURNING from wait on postRecordsAvail!!\n");

                 // retest the predicate
                 if (postRecordsBool == true) {
                   if (verbose) 
                      fprintf(stderr,"thread: postprocessor:  postRecordsAvail is true.\n");
         

                    if (verbose) fprintf(stderr,"thread: postprocessor:  Trying to get wr lock\n");
    	                //coreList.wrlock();
                    
                       if(0 != coreList.trywrlock()) {
	    	         if (verbose) fprintf(stderr,"thread: postprocessor: UNSUCCESSFULL trywrlock()\n");

                        //unlock the associated mutex
                         pthread_mutex_unlock(&postRecordsAvailMutex);      
			  continue;
         	     }
         
                      postRecordsBool == false;   //reset the predicate                                            
                 }
                 else{
                   if (verbose) 
                       fprintf(stderr,"thread: postprocessor:  postRecordsAvail is false (spurious wakeup).\n"); 
            
                   //unlock the associated mutex
                     pthread_mutex_unlock(&postRecordsAvailMutex); 
                     continue;            
                 }
              
               }         

               //unlock the associated mutex
              pthread_mutex_unlock(&postRecordsAvailMutex);                
       
         // we are assuming linear dequeuing; therefore, once we find
		// the first record that has a nonzero reference count, we can stop

         if(verbose)
               cerr << "thread: postprocessor: copy into outgoing from coreList begin. "
                    <<"coreList:" << coreList.size() << " outgoing:" << outgoing.size() <<endl;
       
             theCount = coreList.front().refCount;              
            
#ifdef DEBUG
           if (verbose)
	    cerr << "postprocessor: got reference count from front of list ("<< theCount << ")" << endl;
#endif
	           
	     while(!coreList.empty() && theCount == 0)
	     {

                //put the records into the outgoing queue
	         outgoing.push_back(coreList.front());
		 coreList.pop_front();

#ifdef DEBUG
               if (verbose)
		 cerr << "postprocessor popped from coreList, size is " << coreList.size() << endl;
#endif

                 if (!coreList.empty()) {
            	    theCount = coreList.front().refCount;                                
#ifdef DEBUG
                if (verbose)
 		     cerr << "postprocessor: coreList.front().refCount is " << theCount << endl;
#endif
                }
	     }


           if(verbose)
               cerr << "thread: postprocessor: copy into outgoing from coreList finished. "
                    <<"coreList:" << coreList.size() << " outgoing:" << outgoing.size() <<endl;


	    // release the lock
		if (verbose) fprintf(stderr,"thread: postprocessor:  Trying to unlock\n");

	      coreList.unlock();
		
               if (verbose) fprintf(stderr,"thread: postprocessor:  Returning from trying to unlock\n");

      } //coreList not empty

      //release the CPU after working
      if(verbose)  
            fprintf(stderr, "thread: postprocessor: Release the CPU after working for a while. \n");


        sched_yield();
       
    }//postprocessor loop

    return 0;
}
