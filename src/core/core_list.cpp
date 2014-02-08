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
// $Id: core_list.cpp 792 2007-05-21 20:42:09Z byoo3 $
//
// Implementation file for the FLAIM core list class
//
//********************************************************************


#include <cassert>
#include <vector>
#include <iostream>

#include <pthread.h>

#include "core_list.h"
#include "listed_record.h"
#include <stdint.h>

extern bool verbose;

//#define DEBUG // comment this out to remove verbose debugging messages

// CoreList()
// Constructor
CoreList::CoreList()
{
    //initialize the listLock
    int errnum = pthread_rwlock_init(&listLock, NULL);
    if (errnum)
    { 
      //return string describing the errnum
      cerr << "CoreList: failed to initialize the listLock: " << strerror(errnum ) << endl;
      exit(2);
    }
}


// ~CoreList()
// Destructor
CoreList::~CoreList()
{
    //destroy the listLock 
   int errnum = pthread_rwlock_destroy(&listLock);
   if (errnum)
   {
      //return string describing the errnum
      cerr << "CoreList: failed to destroy the listLock: " << strerror(errnum )  << endl;
   }
}

// operator[]
// Return the listedRecord with the given ID number
//
// parameters: idx, the ID number to find
// returns: a reference to the appropriate listedRecord
// modifies: nothing
// notes: asserts if ID number is out of range
listedRecord & CoreList::operator[](clist_index_t idx)
{
#ifdef DEBUG
    cerr << "CoreList::operator[]: theList[0].id == " << theList[0].id << endl;
#endif

    clist_index_t realIndex = idx + (~theList[0].id + 1);

#ifdef DEBUG
    cerr << "CoreList::operator[]: realIndex == " << realIndex << endl;
#endif

   if (realIndex < 0 ||  realIndex >= theList.size()){
      cerr << "CoreList size: " <<  theList.size() << endl;
      cerr << "CoreList::operator[]: realIndex = " << realIndex << endl;
   }

    assert(realIndex >= 0 && realIndex < (theList.size()));

    return theList[realIndex];
}

// int size()
// gateway to call theList.size()
int CoreList::size() const
{
    return theList.size();
}

// bool empty()
// gateway to call the List.empty()
bool CoreList::empty() const
{
    return theList.empty();
}

// listedRecord & front()
// returns a reference to the first element of coreList
listedRecord & CoreList::front()
{
   return theList.front();
}

// listedRecord & back()
// returns a reference to the last element of coreList
listedRecord & CoreList::back()
{
    return theList.back();
}

// void push_front(const listedRecord & in)
// pushes the parameter onto the front of the list
void CoreList::push_front(const listedRecord & in)
{
    theList.push_front(in);
}

// void pop_front()
// removes the first element in the list
void CoreList::pop_front()
{
    theList.pop_front();
}

// void push_back(const listedRecord & in)
// gateway to call theList.push_back
void CoreList::push_back(const listedRecord & in)
{
    theList.push_back(in);
}

// void pop_back()
// gateway to call theList.pop_back()
void CoreList::pop_back()
{
    theList.pop_back();
}

// int rdlock()
// gateway to lock listLock with a read lock
int CoreList::rdlock()
{
   int errnum = pthread_rwlock_rdlock(&listLock);

   if (errnum)
   {
      //return string describing the errnum
    if (verbose) 
      cerr << "CoreList: failed to get rdlock: " << strerror(errnum )  << endl;
   }

    return errnum;
}

// int wrlock()
// gateway to grab a write lock
int CoreList::wrlock()
{
   int errnum = pthread_rwlock_wrlock(&listLock);;

   if (errnum)
   {
      //return string describing the errnum
    if (verbose) 
      cerr << "CoreList: failed to get wrlock: " << strerror(errnum )  << endl;
   }

    return errnum;
}

// int unlock()
// gateway to unlock the list
int CoreList::unlock()
{
   int errnum = pthread_rwlock_unlock(&listLock);

   if (errnum)
   {
      //return string describing the errnum
     if (verbose) 
      cerr << "CoreList: failed to unlock the listLock: " << strerror(errnum )  << endl;
   }

    return errnum;
}


// int tryrdlock()
// gateway to trying a read lock
int CoreList::tryrdlock()
{
   int errnum = pthread_rwlock_tryrdlock(&listLock);

   if (errnum)
   {
      //return string describing the errnum
     if (verbose) 
      cerr << "CoreList: failed to get tryrdlock: " << strerror(errnum )  << endl;
   }

    return errnum;
}


// int trywrlock()
// gateway to trying a write lock
int CoreList::trywrlock()
{
   int errnum = pthread_rwlock_trywrlock(&listLock);

   if (errnum)
   {
      //return string describing the errnum
    if (verbose) 
      cerr << "CoreList: failed to get trywrlock: " << strerror(errnum )  << endl;
   }

   return errnum;
}
