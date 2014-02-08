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
// $Id: user_policy.h 237 2006-07-27 10:31:56Z vdhar $
//
// This file is part of Policy Manager sub module
// The class to store the user's policy.
//
//********************************************************************


#ifndef _USER_POLICY_H
#define _USER_POLICY_H

#include <string>
#include "policy_manager_lib.h"
#include <vector>

using namespace::std;

class Field {	

  public:
    string field_name;
    string anony_name;

    vector<string>* param_names;
    vector<string>* param_values;

    Field();
    Field(string name);
    void parseFieldStruct(FieldAnony*);
    void printOut();
};

/*****************************************************************/
class UserPolicy {

  public:

    string policy_name;
    vector<Field>* fields;

    void printPolicy(Policy*);
    void printFieldAnony(FieldAnony*);
    UserPolicy(Policy*);
    void print_name();
    void parsePolicyStructure(Policy*);
    void printOut();
};

#endif
