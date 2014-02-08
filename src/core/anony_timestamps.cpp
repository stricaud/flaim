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
// $Id: anony_timestamps.cpp 1002 2007-12-13 21:31:34Z byoo3 $
//
// This file contains the defs of the time stamp based algos defined
// in anony_algos.hpp
//
//********************************************************************


#include "anony_algos.hpp"
#include "core_globals.h"
#include "time_enum.h"
#include <stdint.h>
#include <cassert>

// if an iterator is invalid because it is not part of a collection (i.e.,
// dereferenceable or past-the-end), its increment/decrement operations are
// ill-defined; in particular, --(++i) == i is true iff i is dereferenceable
inline bool iterIsValid(std::list<listedRecord>::iterator i)
{
    std::list<listedRecord>::iterator tmp = i;
    --(++i);
    return i == tmp;
}


const char* const TSAnonyAlg::secondParam = "secondaryField";
const char* const TimeUnitAnnihilation::timeFieldParam = "timeField";


TimeUnitAnnihilation::TimeUnitAnnihilation(vector<string> & param_names, 
    vector<string> & param_values) : TSAnonyAlg(param_names, param_values)
{
  if (verbose) cerr << "flaim: Constructor of TimeUnitAnnihilation. " << endl;
 
   for (int i = 0; i < 6 ; i++)
     trunc[i] = T_INVALID; 

  if( (param_names.size() < 1 ) || (param_names.size() > 2) ||
      (param_names.size() != param_values.size() ) )
  {
    cerr << "flaim: Incorrect number of parameters for TimeUnitAnnihilation" <<endl;
    exit(1);
  }

   cerr << "Time Unit to be truncated: ";

   string::size_type loc;
   int cnt = 0;
 
  loc = param_values[0].find( "seconds", 0 );
   if( loc != string::npos ){
     trunc[0] = T_SECS;
     cnt++;
     cerr << " second ";   
   }

   loc = param_values[0].find( "minutes", 0 );
   if( loc != string::npos ){    
     trunc[1] = T_MINS;
     cnt++;  
     cerr << " minute ";    
   }

   loc = param_values[0].find( "hours", 0 );
   if( loc != string::npos ){
     trunc[2] = T_HRS;
     cnt++;     
     cerr << " hour ";    
   }
   
   loc = param_values[0].find( "days", 0 );
   if( loc != string::npos ){
     trunc[3] = T_DAYS;
     cnt++;    
     cerr << " day ";    
   }  

   loc = param_values[0].find( "months", 0 );
   if( loc != string::npos ){
     trunc[4] = T_MONTHS;
     cnt++;   
     cerr << " month ";    
   }

   loc = param_values[0].find( "years", 0 );
   if( loc != string::npos ){
     trunc[5] = T_YRS; 
     cnt++;    
     cerr << " year ";    
   } 

   cerr << endl;

  if(cnt == 0) {
    cerr <<"flaim: No valid params for TimeUnitAnnihilation"<<endl;
    exit(1);
  } 
  
  lastID = 0;
}


TimeUnitAnnihilation::~TimeUnitAnnihilation()
{
  if (verbose) cerr << "flaim: Destructor of TimeUnitAnnihilation. " << endl;  
}

int TimeUnitAnnihilation::anonymizeValue(Value *v) 
{
  uint32_t *z = Value::to_uint32(v);
  time_t secs = (time_t)(*z);
  tm tdata, *t;
  t = &tdata;
  localtime_r(&secs,&tdata);

  //now blank out the relevant field from it
  if (trunc[0] == T_SECS)
     t->tm_sec = 0;

  if (trunc[1] == T_MINS)
     t->tm_min = 0;
  
  if (trunc[2]== T_HRS)
   t->tm_hour = 0;

  if (trunc[3] == T_DAYS)
   t->tm_mday = 1; ///RESET it to the first of day the month!!

  if (trunc[4] == T_MONTHS)
   t->tm_mon = 0; ///RESET it to the first month january

  if (trunc[5] == T_YRS)
   t->tm_year = 70; ///RESET it to the first year i.e. 1970

  secs = mktime(t);
  uint32_t i = (uint32_t)secs;
  (*z) = i;

  return 0;
}


const char* const RandTimeShift::tShift_upperParam = "upperTimeShiftLimit";
const char* const RandTimeShift::tShift_lowerParam = "lowerTimeShiftLimit";

RandTimeShift::RandTimeShift(vector<string> & param_names, 
    vector<string> & param_values) : TSAnonyAlg(param_names, param_values)
{
  if (verbose) cerr << "flaim: Constructor of RandTimeShift. " << endl;

  upTimeShiftLimit = 0;
  lowTimeShiftLimit = 0;

  if( (param_names.size() <2 ) || (param_names.size() > 3) ||
      (param_names.size() != param_values.size() ) )
  {
    cerr << "flaim: Incorrect number of parameters for RandomTimeShift" << endl;
    exit(1);
  }

  for (int i = 0; i < param_names.size(); i++)
  {
    if (param_names[i] == RandTimeShift::tShift_upperParam)
    {
      upTimeShiftLimit = atoi(param_values[i].c_str());
    }
    else if (param_names[i] == RandTimeShift::tShift_lowerParam)
    {
      lowTimeShiftLimit = atoi(param_values[i].c_str());
    }
  }

  if (upTimeShiftLimit < lowTimeShiftLimit)
  {
    cerr << "flaim: Illegal parameters values for RandomTimeShift" << endl;
    exit(1);
  }

  unsigned int randNum;
  double frac;

       if((fp = fopen("/dev/random","rb"))!=NULL
	    ||(fp = fopen("/dev/urandom","rb"))!=NULL)
       {       
             if(fread(&randNum,sizeof(unsigned int),1,fp)!=1)
             {
               cerr << " Failed to get the randNum from /dev/random or /dev/urandom." << endl;
               exit(1);
            }
            else
            {
                if (verbose)
	         cerr << "randNum = " << (unsigned int)randNum << endl;  
             }
        }
        else{
            cerr << " Failed to open /dev/random or /dev/urandom." << endl;
               exit(1);
        }
 

   frac = (double)randNum/(double)UINT_MAX;

  if (upTimeShiftLimit != lowTimeShiftLimit)
  {
    time_t range = upTimeShiftLimit - lowTimeShiftLimit;
    timeShift = lowTimeShiftLimit + (time_t)(range * frac);
  }
  else 
  {
    timeShift = upTimeShiftLimit;
  }

  cerr <<"flaim: RandomTimeShift value = " << timeShift <<endl;

  lastID = 0;
}


RandTimeShift::~RandTimeShift()
{
  if (verbose) cerr << "flaim: Destructor of RandTimeShift. " << endl;  
  
  if (fp != NULL)
    fclose(fp);
}

int RandTimeShift::anonymizeValue(Value *v)
{
  uint32_t *t = Value::to_uint32(v);
  *t = *t + timeShift;
  return 0;
}







const char* const TimeEnumerate::buffer_sizeParam = "bufferSize";
const char* const TimeEnumerate::interval_lengthParam = "intervalSize";
const char* const TimeEnumerate::baseTimeParam = "baseTime";
const uint32_t TimeEnumerate::default_buffer_size = 30;


TimeEnumerate::TimeEnumerate(vector<string> & param_names, 
    vector<string> & param_values) : TSAnonyAlg(param_names, param_values)
{
	

  if (verbose) cerr << "flaim: Constructor of TimeEnumerate. " << endl;
	numTimesRanCount = 0;
	lastReadID = 0;
  bufferSize = default_buffer_size;
  timeInterval = 0;
  lastBack = 0;
  randEpoch = 0;



  if ( (param_names.size() < 2) || (param_names.size() > 3) ||
      (param_names.size() != param_values.size()) )
  {
    cerr << "flaim: Incorrect number of parameters for TimeEnumerate" << endl;
    exit(1);
  }
  for (int i = 0; i < param_names.size(); i++)
  {
    if (param_names[i] == TimeEnumerate::buffer_sizeParam)
    {
      bufferSize = atoi(param_values[i].c_str());
    }
    else if (param_names[i] == TimeEnumerate::interval_lengthParam)
    {
      timeInterval = atoi(param_values[i].c_str());
    }
    else if (param_names[i] == TimeEnumerate::baseTimeParam)
    {
      randEpoch = atoi(param_values[i].c_str());
    }
  }
 
  if (verbose) cerr << "flaim: randEpoch aka baseTime = " << randEpoch << endl;
  lastID = 0;
  // Create a new buffer list.
	buff_list = new BufferList(randEpoch, timeInterval);
  
}


TimeEnumerate::~TimeEnumerate()
{
  if (verbose) cerr << "flaim: Destructor of TimeEnumerate. " << endl;
  delete buff_list;
}


/////////////////////////////////////////////////////
//
////////////////////////////////////////////////////
int TimeEnumerate::anonymize(string fieldName) {

	buff_list->setFieldName(fieldName);
	buff_list->setCurrIndex(lastReadID);
		
	//lastID points to the position (inclusive) beyond which records havent
	//been anonymized, starts with lastID = 0 & ends with one greater than max-size
	bool goAhead = false;
//	clist_index_t stopID;
	bool lastBatch = false; // True if there are no more records to be read in. 
	clist_index_t lastID = coreList.back().id;

	int ret;
	
	// This means that we need more records before we can anonymise (buff list not 
	// long enough), AND we don't have anymore records on the core list currently 
	// (lastReadID > lastID). So all we can do is set the moreRecords needed flag
	// and return!
	if ((buff_list->size() < bufferSize) && (lastReadID > lastID) && (!pre->preprocessFinished())) {
		if (verbose) 
                    cerr << "TimeEnumerate: lastReadID > lastID, no TimeEnumerate can be done!"<<endl;
	         
		return(-1);	
	}
	
	while (!lastBatch) {
		 if (verbose) {
			cerr << endl << "front(cl):" << coreList.front().id << " back(cl):" 
	  			<< coreList.back().id<<" size(cl):"<<coreList.size() << " lastID:"
	  			<< lastID << " lastReadID: "
	  			<< lastReadID<<" goAhead:" << " buff_list->size(): "<<buff_list->size()
	  			<<endl; //" numTimes: "<<++numTimesRanCount<<"\n";
//			buff_list->toString();
		 }
		ret = fillBuffer(buff_list, &lastReadID);
		
		switch (ret) {
			case -1: 
				assert(ret != -1);
				break;
			case 0: 
                                if (verbose)
                                   cerr << "TimeEnumerate: running enumerate (not last batch)!" << endl;
				buff_list->enumerate();
				break;
				
			case 1: 
				if (verbose)	
					cerr << "TimeEnumerate:running enumerate all!" <<endl;
				lastBatch = true;
				buff_list->enumerateAll();
				break;
				
			case 2: 
				if (verbose)	
					cerr << "TimeEnumerate:running enumerate all!" <<endl;
				lastBatch = true;

				buff_list->enumerateAll();
				break;
			case 3: 
				if (verbose)	
					cerr << "TimeEnumerate:Don't have enough records yet, so let's spin!!" <<endl;
				return(-1); // spin..
				break;
		}
		
           //set flag "postRecordsBool" to true, if the refCount decreases to 0
            if (coreList[coreList.front().id].refCount == 0){
               if (verbose) 
                   cerr << "TimeEnumerate: set postRecordsBool to true" <<endl;

                 //set the predicate
                 pthread_mutex_lock(&postRecordsAvailMutex);  
                  postRecordsBool = true;
                pthread_mutex_unlock(&postRecordsAvailMutex);  
             }
		
	}



}
/////////////////////////////////////////////////////
// fillBuffer()
// This method fills the buffer with elements from the core list. 
//  Importantly, this method makes sure that each record that is addedd to the
// core list has the field that needs to be anonymized.
// ptr_buflst - pointer to the buffer list
// ptr_currentID - pointer to the current id of the element to be next read. 
/////////////////////////////////////////////////////
int TimeEnumerate::fillBuffer(BufferList* ptr_buflst, clist_index_t* currentID) {
	
	Value val;
  	Value val2;
  	Value* v = &val;
  	Value* v2 = &val2;
	
	// Seq. number of the element at the back of the core list. 
	clist_index_t lastID = coreList.back().id;

	if (ptr_buflst->size() > bufferSize) {
		if (verbose) {
			cerr << "fillBuffer: ERROR How did the bufferlist get more than bufferSize records? "<<endl;
			cerr << "fillBuffer: ERROR buflist size: "<<ptr_buflst->size()<<endl;	
		}
		return(-1);
	}
	
	
	// if ready is true, means we are ready to anonymize.
	// If false, means we need to get more records.  
	bool ready = (ptr_buflst->size() == bufferSize);


	bool endOfRecords = (*currentID > lastID); // end of records?
	
	// allRecordsDone indicates that there are no more records left to be read from the log
	// file..
	bool allRecordsDone = pre->preprocessFinished();


	// If size of *ptr_buflst < threshold, let us iterate through the core list
	if (ptr_buflst->size() < bufferSize) {
		// Continue until we hit end of core list or else the buffer is 
		// filled. 
		while ((!endOfRecords) && (!ready)) {
			// Call add bufferList.
			ptr_buflst->addRecord(&coreList[*currentID]);
			// Check size of buffer list.
			if (ptr_buflst->size() == bufferSize) {
				ready = true;
			} else {
				ready = false;
			}
			
			
			(*currentID)++;

			if ((*currentID) > lastID) {
				endOfRecords = true;
			}
		}
	} 
	

	if (ready) {
		// Means we have enough records to enumerate
		if (endOfRecords && allRecordsDone) {
			return(1); // anonymize all	
		} else if (endOfRecords && !allRecordsDone) {
			return(0); // come back later for more records..	
		} else {
			return(0); // come back later for more records.	
		}
	} else {
		// Okay, we didn't get enough records. now see if  we are all done or not.
		
		if (endOfRecords && allRecordsDone) {
			// okay, just anonymize all..
			return(2);	
		} else if (endOfRecords && !allRecordsDone) {
			// Okay, we can get more records in a bit, so need to spin
			return(3);
		} else {
			// Why are we here??? this shouldn't happen..
			if (verbose) 
				cerr << "!!! ERROR ERROR! We shouldn't be here!!!"<<endl;
			return(-1);
		}
		
	}
}
