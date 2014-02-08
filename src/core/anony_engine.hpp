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
// $Id: anony_engine.hpp 983 2007-10-30 05:40:58Z byoo3 $
//
// This file contains the decls for the AnonyEngine & allied classes
// that are used to manage an anonymization.
//
//********************************************************************


#ifndef _ANONY_ENGINE_H
#define _ANONY_ENGINE_H

#define ANONYMIZE_SUCCESS 0 
#define ANONYMIZE_FAILURE 1


#include <stdexcept>
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <ctime>
#include <list>
#include <vector>
#include <typeinfo>
#include <stdlib.h>


// #include "md5.h"
#include "rijndael.h"
#include "panonymizer.h"
#include "value.h"
#include "value_types.h"
#include "record.h"
#include "flaim_core.hpp"

#include "anony_algos.hpp"

using namespace std;

//class for holding the anonymization information from the userPolicy
//this info is then passed to the AnonyEngine's constructor
class AnonyFields
{
  public:
    AnonyFields()
    { 
      field_name = "";	
      anony_name = "";
    }

    string field_name;
    string anony_name;
    vector<string> param_names;
    vector<string> param_values;      
};


//Anonymization engine class
class AnonyEngine
{
  public:
    AnonyEngine(vector<AnonyFields> & anonyFields);
    ~AnonyEngine();

    // function to encapsulate the main engine functionality
    int managerLoop();

  protected:
    map<string, AnonyAlg*> anonyMapTable;
    struct timespec timeout;
    
};

#endif
