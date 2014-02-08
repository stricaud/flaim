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
// $Id: anony_macaddr.cpp 909 2007-07-25 21:44:46Z xluo1 $
//
// This file contains the defs for the mac address specific algos 
// defined in anony_algos.hpp
//
//********************************************************************


#include "anony_algos.hpp"
#include "core_globals.h"
#include <stdint.h>


//class for MAC Adrr random permutation
MacPermute::MacPermute(vector<string> & param_names, vector<string> & param_values)
: AnonyAlg(param_names, param_values)
{
  if (verbose) cerr << "flaim: Constructor of MacPermute." << endl;

  macMapTable.clear();
  macEntryTable.clear();
  mapCount = 0;	        

  lastID = 0;

   if((fp = fopen("/dev/urandom","rb"))==NULL)
   {
       cerr << " Failed to open /dev/urandom." << endl;
       exit(1);               
   }
}

MacPermute::~MacPermute()
{
  if (verbose) cerr << "flaim: Destructor of MacPermute. " << endl;  

  macMapTable.clear();
  macEntryTable.clear();
  mapCount = 0;

  if (fp != NULL)
    fclose(fp);
}

//anonymization function
//Supported Types: STRING and CHAR_BUF
//Size constraint: 6 bytes
int MacPermute::anonymizeValue(Value *v) 
{
  const int macBytes = 6;
  const char* c;

  //perform initial size and type checks
  if (v->Type() == VAL_TYPE_CHAR_PTR) {
    if (v->Bytes() != macBytes) {
      fprintf(stderr, "flaim: given field size unsupported by MacPermute\n");
      exit(1);
    } else {
      c = Value::to_char_ptr(v);
    }
  } 
  else if (v->Type() == VAL_TYPE_STRING) {
    string *s = Value::to_string(v);
    if (s->size() != macBytes) {
      fprintf(stderr, "flaim: given field size unsupported by MacPermute\n");
      exit(1);
    } else {
      //dont forget to set the correct size within the string if the module
      //forgets
      v->setBytes(macBytes);
      c = s->c_str();
    }
  }
  else {
    fprintf(stderr, "flaim: given field type unsupported by MacPermute\n");
    exit(1);
  }
  uint64_t mac = 8192; 
  uint64_t *macAddr = &mac;
  mac = 
    (((uint64_t)c[0])<<0) + (((uint64_t)c[1])<<8)  + (((uint64_t)c[2])<<16) +
    (((uint64_t)c[3])<<24) + (((uint64_t)c[4])<<32) + (((uint64_t)c[5])<<40) ;
  //fprintf(stderr,"flaim: %llX-%llX-%llX-%llX-%llX-%llX\n",
  //    ((uint64_t)c[0])<<0 , ((uint64_t)c[1])<<8 , ((uint64_t)c[2])<<16 ,
  //    ((uint64_t)c[3])<<24 , ((uint64_t)c[4])<<32 , ((uint64_t)c[5])<<40 );
  if (verbose) fprintf(stderr,"flaim: orig macAddr = %llX\n",mac);


  macKey = *macAddr;
  //check if the MAC is in the hashtable
  if (macMapTable.find(macKey) != macMapTable.end())
  {
    //use the existed mapping
    (*macAddr) = macMapTable[macKey];
  }
  else //create a new mapping
  {
    do
    {           
       /* get the permuteEntry from /dev/random or /dev/urandom */
    
          if(fread(&macEntry,sizeof(uint64_t),1,fp)!=1)
            {
               cerr << " Failed to get the permuteEntry from /dev/urandom." << endl;
               exit(1);
            }           
               
       //the generated MAC has been used
    }while (macEntryTable.find(macEntry) != macEntryTable.end()); 

    if (verbose)
	  cerr << "macEntry = " << (uint64_t)macEntry << endl;  

    //add the new pair to the mapping table and mark it in the entry table
    macMapTable.insert(intPair(macKey, macEntry));
    macEntryTable.insert(intPair(macEntry, ++mapCount));
    (*macAddr) = macEntry;
  }
  if (verbose) fprintf(stderr,"flaim: modded macAddr = %llX\n",mac);
  char newMac[8] = {0};

  newMac[0] = (uint8_t)(mac & (0xFFLLU));
  newMac[1] = (uint8_t)((mac & (0xFF00LLU))>>8);
  newMac[2] = (uint8_t)((mac & (0xFF0000LLU))>>16);
  newMac[3] = (uint8_t)((mac & (0xFF000000LLU))>>24);
  newMac[4] = (uint8_t)((mac & (0xFF00000000LLU))>>32);
  newMac[5] = (uint8_t)((mac & (0xFF0000000000LLU))>>40);
  
  if (v->Type() == VAL_TYPE_CHAR_PTR) {
    memcpy(v->Ptr(), ((void*)(macAddr)), macBytes); 
  }
  else if (v->Type() == VAL_TYPE_STRING) {
    string *s = Value::to_string(v);
    *s = newMac;
  }
  return 0;
}

