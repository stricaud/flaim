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
// $Id: core_list.h 652 2007-02-15 20:06:27Z xluo1 $
//
// Header file defining a class that encapsulates a list of 
// listedRecords and gives the necessary functionality for working 
// with the collection
//
//********************************************************************


#ifndef _CORE_LIST_H
#define _CORE_LIST_H

#include <deque>
#include <pthread.h>
#include <stdint.h>

#include "listed_record.h"

class CoreList
{
    public:
	CoreList();
        ~CoreList();
	
	listedRecord & operator[](clist_index_t idx);

	int size() const;
	bool empty() const;

	listedRecord & front();
	listedRecord & back();
	
	void push_front(const listedRecord & in);
	void pop_front();
	void push_back(const listedRecord & in);
	void pop_back();

	int rdlock();
	int wrlock();
	int unlock();
	int tryrdlock();
	int trywrlock();

    private:

	std::deque<listedRecord> theList;
	pthread_rwlock_t listLock;  // reader-writer lock which ensures that the core list is accessed by one thread at 
                                    // each time

};

#endif // _CORE_LIST_H
