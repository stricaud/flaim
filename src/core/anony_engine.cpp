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
// $Id: anony_engine.cpp 1042 2008-01-30 21:46:17Z byoo3 $
//
// This file contains the defs for the AnonyEngine methods, some of
// the important ones being the constructor and the managerLoop
//
//********************************************************************


#include "anony_engine.hpp"
#include "core_globals.h"


using namespace std;


//constructor
AnonyEngine::AnonyEngine(vector<AnonyFields> & anonyFields)
{
  
   cerr << "*** AnonyEngine created." << endl;

   if (verbose) cerr << "flaim: Within AnonyEngine constructor... initializing anony-algo objects"<< endl;
 
  //initialize the anonymization algorithm objects for each data field

  //For manipulating anonymization map table
  vector<AnonyFields>::iterator anonyFieldsIter;

  anonyFieldsIter = anonyFields.begin();
  while (anonyFieldsIter != anonyFields.end()) 
  {
    if (verbose)
      cerr << "flaim: Field-name: "<<(*anonyFieldsIter).field_name
        << ", Anony-algo: " << (*anonyFieldsIter).anony_name<<endl;

    //insert the anony alg object into the anonyMapTable for the corresponding field
    if (anonyMapTable.find((*anonyFieldsIter).field_name) == anonyMapTable.end())  
    {
      if(verbose) {
        for (unsigned int i=0;i<(*anonyFieldsIter).param_names.size();i++) 
        {
          cerr <<"flaim:   \\____ Param Name: "<< (*anonyFieldsIter).param_names[i]
            <<", Param Value: "<<(*anonyFieldsIter).param_values[i]<<endl;		                     		  
        }
      }

      //Create the object for performing the anonymization 
      //FIXME: abstract out the algo name !!!!!
      AnonyAlg * anonyAlg;
      typedef pair <string, AnonyAlg*> anonyPair;
      
      /*if ( (*anonyFieldsIter).anony_name == "IPv4PrefixPreserving")
      {
        // anonyAlg =  new IpPrefixPresv((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
		   anonyAlg =  new BinaryPrefixPresv((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      } */
	  if ( (*anonyFieldsIter).anony_name == "BinaryPrefixPreserving")
      {
       	   anonyAlg =  new BinaryPrefixPresv((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      }
      else if ( (*anonyFieldsIter).anony_name == "RandomTimeShift")
      {        
        anonyAlg =  new RandTimeShift((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      }
      else if ( (*anonyFieldsIter).anony_name == "TimeUnitAnnihilation")
      {
        anonyAlg =  new TimeUnitAnnihilation((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      }
      else if ( (*anonyFieldsIter).anony_name == "NumericTruncation")
      {
        anonyAlg =  new NumericTruncate((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      }
      else if ( (*anonyFieldsIter).anony_name == "StringTruncation")
      {
        anonyAlg =  new StringTruncate((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      }
      else if ( (*anonyFieldsIter).anony_name == "BytesTruncation")
      {
        anonyAlg =  new BytesTruncate((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      }
      else if ( (*anonyFieldsIter).anony_name == "BinaryTruncation")
      {
        anonyAlg =  new BytesTruncate((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      }
      else if ( (*anonyFieldsIter).anony_name == "BinaryBlackMarker")
      {
        anonyAlg =  new BinaryBlackMarker((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      }
      else if ( (*anonyFieldsIter).anony_name == "StringBlackMarker")
      {
        anonyAlg =  new StringBlackMarker((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      }
      else if ( (*anonyFieldsIter).anony_name == "Annihilation")
      {
        anonyAlg =  new Annihilation((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      }
      /*else if( (*anonyFieldsIter).anony_name == "RandomPermutation")
      {                      
        anonyAlg =  new RandPermute((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);        
      }*/
      else if( (*anonyFieldsIter).anony_name == "BinaryRandomPermutation")
      {                             
         anonyAlg =  new BinaryRandPermute((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      }
      else if( (*anonyFieldsIter).anony_name == "TimeRandomPermutation")
      {                             
         anonyAlg =  new BinaryRandPermute((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      }
      else if( (*anonyFieldsIter).anony_name == "Classify")
      {                      
        anonyAlg =  new Classify((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      }
      else if( (*anonyFieldsIter).anony_name == "TimeEnumeration")
      {                      
        anonyAlg =  new TimeEnumerate((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      }
    /*  else if( (*anonyFieldsIter).anony_name == "MacRandomPermutation")
      {                      
         anonyAlg =  new MacPermute((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
	  } */ 
      else if ( (*anonyFieldsIter).anony_name == "HostBlackMarker")
      {
        anonyAlg =  new HostBlackMarker((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      }
      else if ( (*anonyFieldsIter).anony_name == "HostHash")
      {
        anonyAlg =  new HostHash((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      }
      else if ( (*anonyFieldsIter).anony_name == "Hash")
      {
        anonyAlg =  new Hash((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      }
      else if ( (*anonyFieldsIter).anony_name == "Substitution")
      {
        anonyAlg =  new Substitute((*anonyFieldsIter).param_names,(*anonyFieldsIter).param_values);
      }
      ///////////////////////////////////
      // Add other new algorithms here
      ///////////////////////////////////
      else {
        anonyFieldsIter++;
        continue;
      }

      //insert the pair of fieldName and anonymization object into anonyMapTable
      anonyMapTable.insert(anonyPair((*anonyFieldsIter).field_name,  anonyAlg)); 
      //Avoid any possible buffer overflows
      strncpy(anonyAlg->algoName, (*anonyFieldsIter).anony_name.c_str(), 99);
      anonyAlg->algoName[99] = '\0';
    }

    anonyFieldsIter++;
  }
}


//destructor
AnonyEngine::~AnonyEngine()
{
  if (verbose) cerr << "flaim: AnonyEngine destructor is called." << endl;
  
  //For manipulating anonymization map table
  vector<AnonyFields>::iterator anonyFieldsIter;
  map<string, AnonyAlg*>::iterator anonyMapTableIter;
  AnonyAlg * anonyAlg;

  anonyMapTableIter = anonyMapTable.begin();
  while (anonyMapTableIter!= anonyMapTable.end())
  { 
    anonyAlg = (*anonyMapTableIter).second;

    if (verbose) cerr << "flaim: Deleting object of " << (*anonyMapTableIter).first << endl;

    delete anonyAlg;
    anonyMapTableIter++;
  } 

  //delete the object that is in the anonyMapTable
  if(!anonyMapTable.empty())
    anonyMapTable.clear(); 
}


// int managerLoop()
// Main loop which handles control flow and anonymization algorithm invocation
// for the anonymization thread
//
// Parameters: None
// Returns: 0 on successful termination, 1 otherwise
// Modifies: Performs operations on the core list and the records contained
//           in it
int AnonyEngine::managerLoop()
{
    
      fprintf(stderr,"thread: anony: Starting anonyEngine.\n");

    map<string, AnonyAlg*>::iterator tableIter;
    int status;

    while(1)
    {
       if (verbose) 
         fprintf(stderr,"thread: anony: getting the CPU to run.\n");

	tableIter = anonyMapTable.begin();       

	// get a write lock so the list can be safely traversed

       if (verbose) fprintf(stderr,"thread: anony:  Trying to get wrlock\n");

	//coreList.wrlock();

       if(0 != coreList.trywrlock()) {
	     if (verbose) 
                fprintf(stderr,"thread: anony: UNSUCCESSFULL trywrlock()\n");
             continue;
       }

	// if corelist is empty 
	if(coreList.empty())
	{
             //unlock the corelist
           if (verbose) fprintf(stderr,"thread: anony:  Trying to get unlock\n");
             coreList.unlock();
    
            
	    if(pre->preprocessFinished())
	    {
	    	// no more records from preprocessor and empty core list.
              if(verbose)  
                 fprintf(stderr, "thread: anony: all the work is done! \n");
		  break; //get out of the anonyEngine threading loop
	    }
	    else  //wait for more records
	    {
	    	// So core list is empty, but possibility of more records.           
            
	        if (verbose) 
                   fprintf(stderr,"thread: anony:  starting wait on preRecordsAvail!!\n");

                //lock the mutex associated with the condition variable
		pthread_mutex_lock(&preRecordsAvailMutex);

                //set the timeout for condtion wait
                //timeout.tv_sec = time(NULL)+WAIT_TIME_SECONDS;
                //timeout.tv_nsec = 0;

               //wait on the condition variable
	       while (preRecordsBool != true){
                   status = pthread_cond_wait(&preRecordsAvail,&preRecordsAvailMutex);
                   //status = pthread_cond_timedwait(&preRecordsAvail,&preRecordsAvailMutex, &timeout);
                   
                   //timeout or error happened
                   if (status){
                        fprintf(stderr,"thread: anony:  error in wait on preRecordsAvail!!\n");
                         pthread_mutex_unlock(&preRecordsAvailMutex);
                         return 1;
                    } 
              }

                if (verbose) 
                  fprintf(stderr,"thread: anony:  RETURNING from wait on preRecordsAvail!!\n");

                 // retest the predicate
                 if (preRecordsBool == true) {
                   if (verbose) 
                       fprintf(stderr,"thread: anony:  preRecordsAvail is true.\n");
                     preRecordsBool == false;   //reset the predicate                 
                 }
                 else{
                   if (verbose) 
                       fprintf(stderr,"thread: anony:  preRecordsAvail is false (spurious wakeup).\n");             
                 }

                 //unlock the associated mutex
                 pthread_mutex_unlock(&preRecordsAvailMutex); 
                continue;                                                     
	    }
	} 

	// iterate over all the algorithms
	while(tableIter != anonyMapTable.end())
	{
	    if (verbose) 
               fprintf(stderr, "anony_engine: anonymizing field %s\n", (tableIter->first).c_str());
            if (verbose) 
               fprintf (stderr, "anony_engine: calling anonymize for algo - %s\n", 
                  tableIter->second->algoName);
	    
            // call the anonymization function, which uses its access to the
	    // list to determine if it can run, running if this is possible
	    tableIter->second->anonymize(tableIter->first);
	    // move to the next anony object in the table
	    ++tableIter;
	}

	if (verbose) fprintf(stderr,"thread: anony: unlocking corelist\n");
	 coreList.unlock();


       // signaling the postprocessor, when postRecordsBool is set to true by the anony functions
        pthread_mutex_lock(&postRecordsAvailMutex);         		
        if (postRecordsBool){
           if(verbose)  
            fprintf(stderr, "thread: anony: signalling the postprocessor that records available for output. \n");       
            
           pthread_cond_signal(&postRecordsAvail);
	}
   	pthread_mutex_unlock(&postRecordsAvailMutex);


      if(verbose)  
         fprintf(stderr, "thread: anony: Release the CPU after working for a while. \n");

       //release the CPU after working for a while          
       // pthread_yield();
          sched_yield();
    }

    return 0;
}
	    

	
