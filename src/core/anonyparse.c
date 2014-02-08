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
// $Id: anonyparse.c 1002 2007-12-13 21:31:34Z byoo3 $
//
// This file is part of the Policy Manager sub module
//
//********************************************************************


#include<libxml/xmlstring.h>
#include "anonyparse.h"

/**
 * This method parses a xml node that represents a field node in the user policy
 *
 * We assume that all the parameters will not have a sub tree, and will only have some text.
 **/
struct FieldAnony* parseFieldElement(xmlNodePtr felement, xmlDocPtr doc) {
    int paramNum = 0;
    struct FieldAnony* field;
    
    field = (struct FieldAnony*) malloc(sizeof(struct FieldAnony));
    if (field == NULL) {
        fprintf(stderr, "out of memory\n");
        return(NULL);
    }
    /*	printf("Size of struct FieldAnony: %d\n",sizeof(struct FieldAnony));
        printf("sizeof: char: %d, char*: %d\n", sizeof(char), sizeof(char*));
   
        printf("---- <parseFieldElement>--------\n");
     */
    // What is the name of the node?
    //	printf("Parsing node with name: %s\n",felement->name);
    
    if (xmlStrcmp(felement->name, "text") == 0) {
        printf("Why is the node text? stringvalue: %s\n", xmlNodeListGetString(doc, felement, 1));
    }
    
    // Now first figure out the value of the field attribute and set field.fieldname
    field->fieldname = xmlGetProp(felement, "name");
    
    //	printf("Field name: %s\n",field->fieldname);
    
    /*	xmlChar* key;
        key = xmlNodeListGetString(doc, felement->xmlChildrenNode, 1);
        printf("key: %s\n",key);
        xmlFree(key);
     */
    
    // Loop through the child elements looking for the elements that are type 1 (an xml element rather than
    // a text node or something. Use NodeListGetString to get those contents and print it out.
    xmlNodePtr fieldChildren = felement->xmlChildrenNode;
    xmlChar* paramName;
    
    //	xmlNodePtr temp;
    
    while (fieldChildren != NULL) {
        // printf("\tchild name: %s,type: %d\n",fieldChildren->name,fieldChildren->type);
        if (fieldChildren->type == 1) {
            // Huzzah this is an anonymization algorithm!
            // printf("\tAnonymization algorithm: %s\n",fieldChildren->name);
            
            field->anonyname = malloc(sizeof(char)*(xmlStrlen(fieldChildren->name)+1));
            strcpy(field->anonyname, fieldChildren->name);
            // printf("\tAnonymization algorithm: %s\n",field->anonyname);
            
            /* 
             * Now loop through the children of the anonymization algorithm. For each child (parameter)        
             * print out the name and the value
             */
            xmlNodePtr anonyAlgParametersPtr = fieldChildren->xmlChildrenNode;
                        
            while (anonyAlgParametersPtr != NULL) {
                // printf("\t\tanonyAlgParametersPtr child name: %s,type: %d\n",anonyAlgParametersPtr->name,anonyAlgParametersPtr->type);
                
                if (anonyAlgParametersPtr->type == 1) {
                    paramName = anonyAlgParametersPtr->name;

                    /*					
                     * temp = anonyAlgParametersPtr->xmlChildrenNode;                        
                     * while (temp != NULL) {                            
                     *     printf("\t\t temp name: %s,type: %d\n",temp->name,temp->type);
                     *     temp = temp->next;
                     * }
                     */					
                    xmlChar* paramValue;
                    paramValue = xmlNodeListGetString(doc, anonyAlgParametersPtr->xmlChildrenNode, 1);
                    if (paramValue == NULL) {
                        fprintf(stderr, "ParamValue is null..");
                    }

                    // printf("\tParameter name:%s  Value: %s AllocSize:%d\n",paramName, paramValue, (sizeof(char)*xmlStrlen(paramName)) );
                    field->param[paramNum][0] = malloc(sizeof(char)*(xmlStrlen(paramName)+1));
                    strcpy(field->param[paramNum][0], paramName);

                    field->param[paramNum][1] = malloc(sizeof(char)*(xmlStrlen(paramValue)+1));
                    strcpy(field->param[paramNum][1], paramValue);
                    paramNum++;
                    xmlFree(paramValue);                     
                }
                anonyAlgParametersPtr = anonyAlgParametersPtr->next;
            }
        }
        fieldChildren = fieldChildren->next;
    }
    
    // Set the last element in the param's field to be NULL and value to be NULL to show that we terminated
    
    field->param[paramNum][0] = "NULL";
    field->param[paramNum][1] = "NULL";
    field->numOfParms = paramNum;
    //	printf("----</parseFieldElement>--------\n");
    
    return(field);
}

int printPolicy(struct Policy* policy) {
    printf("Policy\n");
    printf("Name: %s\n", policy->filename);
    
    int i;
    for (i=0;i<policy->numOfFields;i++) {
        printFieldAnony(policy->fields[i]);
        printf("\n");
    }
    
    printf("End policy..");    
}

int printFieldAnony(struct FieldAnony* field) {
    printf("Fieldname: %s\n", field->fieldname);
    printf("Anonymization algorithm: %s\n", field->anonyname);
    
    int notNull = 0;
    char* paramname;
    char* paramval;
    int i;
    int j;
    for (i;i<field->numOfParms;i++) {
        paramname = field->param[i][0];
        paramval = field->param[i][1];
        printf("(%s,%s) pointervalues(%d,%d)\n", paramname, paramval, paramname, paramval);
    }
}
