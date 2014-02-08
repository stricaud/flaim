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
// $Id: user_policy.cpp 1002 2007-12-13 21:31:34Z byoo3 $
//
// This file is part of Policy Manager sub module
//
//********************************************************************

#include "user_policy.h"
#include <iostream>
#include <iterator>

using namespace::std;

/**
 * Parse a policy structure and get all the information..
 **/
UserPolicy::UserPolicy(Policy* policyStruct) {
    parsePolicyStructure(policyStruct);    
}

void UserPolicy::print_name() {
    cout << "Name: "<< policy_name << endl;
}

void UserPolicy::parsePolicyStructure(Policy* policy_struct) {
    //	printPolicy(policy_struct);
    
    //	printf("Number of fields: %d\n", policy_struct->numOfFields);
    
    fields = new vector<Field>(policy_struct->numOfFields);
    // Field* curr_field;
    for (int i = 0; i < policy_struct->numOfFields; i++) {
        printf("Parsing field: %d\n", i);
        
        // curr_field = new Field();
        
        (fields->at(i)).parseFieldStruct(policy_struct->fields[i]);
        // fields->assign(i, *curr_field);
        (fields->at(i)).printOut();
    }        
    
    fprintf(stderr, "Finished parsing, now printing..");
    printOut();
}

void UserPolicy::printOut() {
    vector<Field>::iterator vfptrIt;
    
    vfptrIt = fields->begin();
    
    while (vfptrIt != fields->end()) {
        (*vfptrIt).printOut();
        vfptrIt++;
    }        
}


void UserPolicy::printPolicy(struct Policy* policy) {
    printf("Policy\n");
    printf("Name: %s\n", policy->filename);
    
    int i;
    for (i=0;i<policy->numOfFields;i++) {
        printFieldAnony(policy->fields[i]);
        printf("\n");
    }
    
    printf("End policy..");
    
}

void UserPolicy::printFieldAnony(struct FieldAnony* field) {
    printf("Fieldname: %s\n", field->fieldname);
    printf("Anonymization algorithm: %s\n", field->anonyname);
    
    int notNull = 0;
    char* paramname;
    char* paramval;
    int i;
    for (i=0;i<field->numOfParms;i++) {
        paramname = field->param[i][0];
        paramval = field->param[i][1];
        printf("(%s,%s) pointer: (%d,%d)\n", paramname, paramval, paramname, paramval);
    }
}

/****************************************************************/

Field::Field() {
    param_names = NULL;
    param_values = NULL;
}

void Field::parseFieldStruct(FieldAnony* field_struct) {
    /*	cout << "Parsing field: "<<field_struct->fieldname << endl;
        cout << "Number of parameters: "<<field_struct->numOfParms<<endl;
     */
    field_name = field_struct->fieldname;
    anony_name = field_struct->anonyname;
    
    param_names = new vector<string>(field_struct->numOfParms);
    param_values = new vector<string>(field_struct->numOfParms);
    
    for (int i = 0; i < field_struct->numOfParms; i++) {
        (*param_names)[i] = field_struct->param[i][0];
        (*param_values)[i] = field_struct->param[i][1];
    }
}


void Field::printOut() {
    vector<string>::iterator nameIt  = param_names->begin();
    vector<string>::iterator valueIt = param_values->begin();
    
    cout<< "\nPrinting field: "<<field_name<<endl;
    cout<< "\nAnonymization algorithm: "<<anony_name<<endl;
    
    for (int i=0;i<param_names->size();i++) {
        cout<<"\tName: "<<(*param_names)[i]<<endl;
        cout<<"\tValue: "<<(*param_values)[i]<<endl;
    }
    
    /*	while (nameIt != param_names->end()){
                cout<<"\tName: "<<*nameIt<<endl;
                cout<<"\tValue: "<<*valueIt<<endl;
                nameIt++;
                valueIt++;
        }
     */
}
