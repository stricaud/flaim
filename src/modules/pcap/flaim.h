/*****************************************************************
*                                                                *
*  flaim.hpp                                                     *
*                                                                *
*  Definition of general parser module API for flaim. It is an   *   
*  an abstract class. The specific parser module needs to        * 
*  provide all the API functions for using flaim.                *
*                                                                *
*  Written Mar 2006 by Xiaolin Luo                               *
*                                                                *
*    - changed by Xiaolin Luo May 2006                           *
*                                                                *
*                                                                *
*****************************************************************/

#include <iostream>
#include <string>
#include <list>
#include <dlfcn.h>  //for dynamic loading

#include "record.h"

using namespace std;

//defined in core to indicate that verbose messages should be printed
//extern bool verbose;

class LogParser  //abstract base class
{

public:
  //constructor
  LogParser()  {;}
 
  virtual ~LogParser(){;}  

 //Module API 
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
