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
// $Id: flaim.hpp 925 2007-08-16 20:28:43Z byoo3 $
//
// Definition of general parser module API for flaim. It is an 
// abstract class. The specific parser module needs to provide all 
// the API functions for using flaim.               
//
//********************************************************************


#ifndef _FLAIM_HPP
#define _FLAIM_HPP

#include <iostream>
#include <string>
#include <list>
#include <dlfcn.h>  //for dynamic loading

#include "record.h"

using namespace std;

// This class defines the flaim Module API which should be implemented
// by the dynammically linked modules
class LogParser  //abstract base class
{

public:
  //constructor
  LogParser()  {;}
 
  virtual ~LogParser(){;}  


  //get the location  module schema file 
  virtual int getModuleSchemaFile(string & schemaFile) = 0;
  
  //get one log record 
  virtual Record * getRecord() = 0;

  //put one log record
  virtual int putRecord(Record * r) = 0;

  //test if it reaches the end of log
  virtual int isEndofLog() = 0;

  //get the sequence number of the current record
  virtual int getRecordSeq(int & rcdSeq) = 0;

  //reset the file pointer to the beginning of log
  virtual int resetFilePointer() = 0;

  int returnCode;
};


//the types of the class factories
typedef LogParser* create_t(string srcLog, string destLog, string errOutLog, string anonyField);
typedef void destroy_t(LogParser*);

#endif
