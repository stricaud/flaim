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
// $Id: policy_manager.cpp 1002 2007-12-13 21:31:34Z byoo3 $
//
// The entry point into the policy manager sub module
//
//********************************************************************


#include "policy_manager.h"

#include <iostream>
#include <string>
#include <fstream>   // file I/O
#include <iomanip>

using namespace std;

//config file param names
const char * const flaim_schema = "flaimSchemaRng";
const char * const stron_xsl = "schematronXsl";
const char * const mslStron_xsl = "mslToSchematronXsl";

UserPolicy * PolicyManager(map<string, char*> &configData, char* userpolicy, char* moduleschema) {
    
    // fprintf(stderr, "1configData[mod_schema]: %s\n", moduleschema);
    // fprintf(stderr, "1configData[mod_schema]: %s\n", userpolicy);
    
    char* flaimschema        = configData[flaim_schema];
    char* schematronstyle    = configData[stron_xsl   ];
    char* msltoschematronsty = configData[mslStron_xsl];
    
    // fprintf(stderr, "2configData[mod_schema]: %s\n", moduleschema);
    // fprintf(stderr, "2configData[mod_schema]: %s\n", userpolicy);
    
    if (!(flaimschema && moduleschema && userpolicy && schematronstyle && msltoschematronsty)) {
        fprintf(stderr, "plcymgr: Incomplete configuration data.\n");
        fprintf(stderr, "plcymgr: The following are compulsory:\n");
        fprintf(stderr, "plcymgr: flaimSchemaRng moduleSchemaXml userPolicyXml schematronXsl mslToSchematronXsl\n");  //FIXME: add the schematron1.5 thingy here... but its implicit ??
        exit(2);
    }
    
    // fprintf(stderr, "3configData[mod_schema]: %s\n", moduleSchema.c_str());
    
    Policy* apolicy = validateAndParse(userpolicy, flaimschema, moduleschema, schematronstyle, msltoschematronsty);
    //printf("Number of fields in policy: %d\n",apolicy->numOfFields);
    
    UserPolicy *upolicy =  new UserPolicy(apolicy);
    for (int i = 0; i < ((struct Policy*)apolicy)->numOfFields; i++) {
        for (int j = 0; j < apolicy->fields[i]->numOfParms; j++) {            
            free(apolicy->fields[i]->param[j][0]);
            free(apolicy->fields[i]->param[j][1]);
        }
        free(apolicy->fields[i]->anonyname);
        xmlFree(apolicy->fields[i]->fieldname);
        free(((struct FieldAnony*)apolicy->fields[i]));
    }
    free(apolicy);
    
    return upolicy;
}
