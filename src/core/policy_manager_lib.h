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
// $Id: policy_manager_lib.h 237 2006-07-27 10:31:56Z vdhar $
//
// This file is part of the Policy Manager Sub Module
//
//********************************************************************


#ifndef POLICYMANAGERLIB_H
#define	POLICYMANAGERLIB_H

#include "anonyparse.h"


#if defined __cplusplus
extern "C" {
#endif


  struct TestPassing {
    char* foo1;
    char* foo2;
    char* foo3;
  };


  int validateFLAIM(xmlDocPtr, const char*);
  int validateModule(xmlDocPtr, const char*, const char*, const char*);
  struct Policy* validateAndParse(const char* instance, const char* flaimSchema, const char* moduleSchema, const char* schematronStylesheet, const char* MSLtoStronStylesheet);
  struct Policy* parse(xmlDocPtr);
  struct TestPassing* sendCharstarstar();

#if defined __cplusplus
}
#endif	

#endif
