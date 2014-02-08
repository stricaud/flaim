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
// $Id: policy_manager_lib.c 1002 2007-12-13 21:31:34Z byoo3 $
//
// This file is part of the Policy Manager Sub Module
//
//********************************************************************


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/relaxng.h>

#ifdef BUILD_ON_BSD
  #include <xsltutils.h>
  #include <xsltInternals.h>
#else
  #include <libxslt/xsltutils.h>
  #include <libxslt/xsltInternals.h>
#endif
#include <libxml/xmlstring.h>
#include "anonyparse.h"


#include "policy_manager_lib.h"

#if defined EXEC_VERSION
int main (int argc, const char * argv[]) {
  // insert code here...
  printf("Hello, World!\n");

  if (argc < 5) {
    fprintf(stderr, "usage: %s instance_file FLAIM_schema module_schema schematron_stylesheet\n",argv[0]);
    return(-1);
  }
  validateAndParse(argv[1],argv[2],argv[3],argv[4]);
  fprintf(stderr,"Returning...\n");

  return(ret) ;
}

#endif


/***********************************************************************
  This method is the method that checks to make sure the instance file and the schema file are valid
  and can be parsed
 **/
struct Policy* validateAndParse(const char* instance, const char* flaimSchema, 
    const char* moduleSchema, const char* schematronStylesheet,
    const char* MSLtoStronStylesheet) 
{
  xmlDocPtr instance_doc;

  instance_doc = xmlParseFile(instance);
  if (instance_doc == NULL ) {
    fprintf(stderr,"plcymgr: User policy not parsed successfully.\n");
    xmlFreeDoc(instance_doc);
    exit(2);
  }

  int flaim_ret = validateFLAIM(instance_doc, flaimSchema);

	if (flaim_ret != 0) {
		fprintf(stderr, "Instance %s failed to be validated by FLAIM Schema: %s\n",instance, flaimSchema);
		exit(2);
	}
  
        fprintf(stderr,"module schema: %s\n", moduleSchema);
	int module_ret = validateModule(instance_doc, moduleSchema, schematronStylesheet, MSLtoStronStylesheet);
	
	if (module_ret < 0) {
		fprintf(stderr, "Instance %s failed to be validated by Module Schema: %s\n",instance, moduleSchema);
		exit(2);
	}
	
	
	struct Policy* apolicy = parse(instance_doc);

	fprintf(stderr, "Freeing resources..\n");
	xmlFreeDoc(instance_doc);
	xmlCleanupParser();
	xmlMemoryDump();
	return(apolicy);
		
}

struct TestPassing* sendCharstarstar() {
	
	
/*	char* foo = "hello world!";
	return(foo);
*/	
	struct TestPassing* t = (struct TestPassing*) malloc(sizeof(struct TestPassing));
	t->foo1 = "hello1";
	t->foo2 = "hello2";
	t->foo3 = "hello3";
	
	fprintf(stderr,"In PolicyManagerLib\n");
	fprintf(stderr,"foo1: %s\n",t->foo1);
	fprintf(stderr,"foo2: %s\n",t->foo2);
	fprintf(stderr,"foo3: %s\n", t->foo3);
	
	return(t);
	
/*	foo = malloc(10*sizeof(char*));
	int i;
	for (i=0;i<5;i++) {
		
		foo[i][0] = malloc(sizeof(char)*4);
		strcpy(foo[i][0],"hell");

		foo[i][1] = malloc(sizeof(char)*1);
		strcpy(foo[i][0],"l");
	}
	fprintf(stderr, "C: %d char***pointer..\n", foo);
	return(foo); */
}


/**
This method prints out the policy..
 Later on we will modify this method to populate an internal data structure for the policy.
 **/
struct Policy* parse(xmlDocPtr instance_doc) {

    struct Policy* policy = malloc(sizeof(struct Policy));

    policy->filename = instance_doc->name;
    int fieldNum = 0;

    xmlNodePtr root_node = xmlDocGetRootElement(instance_doc);
    fprintf(stderr, "Root element name: %s\n",root_node->name);

    // Once I have the policy node, it's children should be the field nodes
    xmlNodePtr childrenNodes = root_node->xmlChildrenNode;

    // for each field node, call the parseFieldElement function.
    /*
     libxml makes a node for the comments and some text nodes.
     I don't know what the text nodes represent..
     So here i will only call parseFieldElement on the 
     nodes which are field nodes.
     */
    while (childrenNodes != NULL) {
        if (xmlStrcmp(childrenNodes->name,"field") == 0) {
            policy->fields[fieldNum] = parseFieldElement(childrenNodes, instance_doc);
            fieldNum++;
        }
        childrenNodes = childrenNodes->next;
    }

    policy->numOfFields = fieldNum;
    // printPolicy(policy);

    return(policy);
}


/*****************************************************************
Validate an instance document according to the FLAIM policy
*****************************************************************/
int validateFLAIM(xmlDocPtr instance_doc, const char* flaimSchema) {
	
	xmlRelaxNGParserCtxtPtr rngpcp;
	xmlRelaxNGPtr rng;
	xmlRelaxNGValidCtxtPtr rngValidCtxt;

	
	int isValid;
	
	/**
		Create a context for the relax ng XML schema file.
	 use xmlRelaxNGNewParserCtxt
	 **/	
	
	rngpcp = xmlRelaxNGNewParserCtxt(flaimSchema);
    if (rngpcp == NULL) 
    {
      fprintf(stderr,"plcymgr: FLAIM RelaxNG parser context was not created successfully.\n");
      xmlRelaxNGFreeParserCtxt(rngpcp);
      exit(2);
    }

	
	/**
		Parse the relaxNG schema
	 using: xmlRelaxNGParse
	 **/
	
	rng = xmlRelaxNGParse(rngpcp);
    if (rng == NULL) 
    {
      fprintf(stderr,"plcymgr: FLAIM RelaxNG schema was not parsed successfully.\n");
      xmlRelaxNGFree(rng);
      exit(2);
    }
	
	/**
		Create a validation context
	 using: xmlRelaxNGNewValidCtxt
	 **/
	
	rngValidCtxt = xmlRelaxNGNewValidCtxt(rng);
    if (rngValidCtxt == NULL)
    {
      fprintf(stderr,"plcymgr: RelaxNG Validation context was not created successfully.\n");
      xmlRelaxNGFreeValidCtxt(rngValidCtxt);
      exit(2);
    }
	
	
	
	/**
		Validate using the relax ng validation context
	 using: relaxNGValidateDoc
	 **/
	isValid = xmlRelaxNGValidateDoc(rngValidCtxt, instance_doc);
	
	
	if (isValid != 0) 
	{
	  fprintf(stderr, "Document is not valid!(schema: %s)\n", flaimSchema);
	} else {
	  fprintf(stderr, "Document is valid!(schema %s)\n", flaimSchema);
	}
	
	xmlRelaxNGFree(rng);
	xmlRelaxNGFreeValidCtxt(rngValidCtxt);
	xmlRelaxNGFreeParserCtxt(rngpcp);
	return(isValid);
}

/***********************************************
* This method does a couple things:
* 1) Parse the instance doc and module schema file
* 2) Check if module schema file is a schematron file or a module schema language file
* 3) If a MSL file, convert to a schematron file.
* 4) Apply the schematron stylesheet to the schematron file.
* 5) Apply the resulting stylesheet to the instance doc
* 6) Check for errors.
************************************************/
int validateModule(xmlDocPtr instance_doc , const char* moduleSchema, 
				   const char* schematronStylesheet, const char* msltostronstylesheet)  {
	const char *params[1];
	params[0] = NULL;
	
	
	// Instance document, output document, schematron schema as doc file 
	// (to apply on instance), 
	// and the schematron schema as a stylesheet as a doc.
	// module_doc will be where the module schema is initially loaded - 
	// to see if it is an MSL file or a Schematron file.
	xmlDocPtr output_doc, stron_schema_doc, stron_schema_stylesheet_doc, module_doc;
	xmlNodePtr module_doc_children;
	
	// the schematron style file (e.g. schematron basic),  
	// schematron style file as a stylefile pointer.
	xsltStylesheetPtr stron_stylesheet_sty, stron_schema_stylesheet_sty, msltostron_stylesheet_sty;
	
	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue = 1;
	
	// First parse the schematron style file.
	stron_stylesheet_sty = 
	    xsltParseStylesheetFile((const xmlChar *)schematronStylesheet);
	
	fprintf(stderr,"Parse the schematron stylesheet.\n");
	if (stron_stylesheet_sty == NULL) 
	{
	  fprintf(stderr,"plcymgr: stron_stylesheet_sty is NULL\n");
          xsltFreeStylesheet(stron_stylesheet_sty);
          exit(2);
	}
	
	/*****
	* Load the module schema and check to see whether it is of 
	* Module Schema language or Schematron..
	*****/
	
	// fprintf(stderr,"module schema: %s\n", moduleSchema);
	// Now parse the schematron schema
	module_doc = xmlParseFile(moduleSchema);
        if (module_doc == NULL ) {
          fprintf(stderr,"plcymgr: Module Schema: %s not parsed successfully. Check to make sure file is in correct location!\n",moduleSchema);
          xmlFreeDoc(module_doc);
          exit(2);
        }

	
	module_doc_children = module_doc->children;
	fprintf(stderr, "First childs' name in module schema: %s\n",
		module_doc_children->name);
        
	if (xmlStrEqual("modulepolicy",module_doc_children->name)) {
		/**
		* The module schema is in MSL. need to convert to schematron.
		*/
		fprintf(stderr, "Converting %s to schematron using %s\n", 
			moduleSchema, msltostronstylesheet);
		msltostron_stylesheet_sty = 
		    xsltParseStylesheetFile
			((const xmlChar *) msltostronstylesheet);
		if (msltostron_stylesheet_sty == NULL) {
			fprintf(stderr, "error in parsing msltostron stylesheet");
		}
		
		stron_schema_doc = xsltApplyStylesheet(msltostron_stylesheet_sty, module_doc, (const char**)params);
			
	} else {
		stron_schema_doc = module_doc;		
	}
	
	
	
	fprintf(stderr,"Parse the schematron schema.\n");
	if (stron_schema_doc == NULL) {
		fprintf(stderr,"stron_schema_doc");
	}
	
	
	// Now apply the schematron stylesheet onto the schematron schema. store in 
	stron_schema_stylesheet_doc = xsltApplyStylesheet(stron_stylesheet_sty, stron_schema_doc, (const char**)params);
	fprintf(stderr,"Apply stron stylesheet to stron schema.\n");
	if (stron_schema_stylesheet_doc == NULL) {
		fprintf(stderr,"*** stron_schema_stylesheet_doc is NULL\n");
	}
	

	
	// Now transform the strong_schema_stylesheet_doc from a xmlDocPtr to a xsltStylesheetPtr!
	stron_schema_stylesheet_sty = xsltParseStylesheetDoc(stron_schema_stylesheet_doc);
	fprintf(stderr,"Transform the stron schema stylesheet from doc into stylesheet pointer.\n");
	if (stron_schema_stylesheet_doc == NULL) {
		fprintf(stderr,"*** stron_schema_stylesheet_doc is NULL\n");
	}
	
	
	// Now apply the schematron schema stylessheet to the instance document.
	output_doc = xsltApplyStylesheet(stron_schema_stylesheet_sty, instance_doc, params);
	fprintf(stderr,"Apply on the output document.\n");
	if (output_doc == NULL) {
		fprintf(stderr,"output_doc: ");
		fprintf(stderr,"Failed to get a non-null output document!\n");
		return -1;
	}

	
	xmlChar *key;
	key = xmlNodeListGetString(output_doc,output_doc->children,1);
	// Check to see if a substring is present in the string.
	// Now check to see if the key word "{ERROR}" is in the output
	fprintf(stderr, "Output from validation: \n%s\n",key);

	xmlChar* rescomp;
	const xmlChar* search = "{ERROR}";
	rescomp = xmlStrstr(key,search);

	int isValid;
	if (rescomp == NULL) {
		isValid = 0;
		fprintf(stderr, "Couldn't find %s\n",search);
	} else {
		isValid = -1;
		fprintf(stderr, "Found an error!");
	}
	
	xmlFree(key);
	
	xsltFreeStylesheet(msltostron_stylesheet_sty);
	xsltFreeStylesheet(stron_stylesheet_sty);
	xsltFreeStylesheet(stron_schema_stylesheet_sty);
	
        xmlFreeDoc(module_doc);
	xmlFreeDoc(output_doc);
	xmlFreeDoc(stron_schema_doc);
	
	return(isValid);
}
