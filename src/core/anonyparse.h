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
// $Id: anonyparse.h 237 2006-07-27 10:31:56Z vdhar $
//
// This file is part of the Policy Manager sub module and will 
// contain the headers for parsing methods for various 
// anonymization routines.
//
//********************************************************************


#ifndef ANONYPARSE_H
#define ANONYPARSE_H


#include <libxml/tree.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAXNUMPARMS 100
#define MAXNUMFIELDS 50

#if defined __cplusplus
extern "C" {
#endif


  struct FieldAnony {
    int numOfParms;
    char* fieldname;
    char* anonyname;
    char* param[MAXNUMPARMS][2];
  };

  struct Policy {
    char* filename;
    int numOfFields;
    struct FieldAnony* fields[MAXNUMFIELDS];
  };
  struct FieldAnony* parseFieldElement(xmlNodePtr felement, xmlDocPtr doc);

#if defined __cplusplus
}
#endif

#endif
