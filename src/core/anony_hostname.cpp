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
// $Id: anony_hostname.cpp 981 2007-10-30 05:19:22Z byoo3 $
//
// This file contains the defs for the hostname specific algos
// defined in anony_algos.hpp
//
//********************************************************************


#include "anony_algos.hpp"
#include "core_globals.h"



const char* const HostBlackMarker::typeParam = "type"; //options: FullName,HostOnly
const char* const HostBlackMarker::hostAltParam = "hostReplacement";
const char* const HostBlackMarker::domainAltParam = "domainReplacement";

//Hostname specific BlackMarker
HostBlackMarker::HostBlackMarker(vector<string> & param_names,vector<string> & param_values) 
: AnonyAlg(param_names, param_values)
{
  if (verbose) cerr << "flaim: Constructor of HostBlackMarker." << endl;

  if( (param_names.size() != 3) || (param_values.size() != 3) ) {
    cerr << "flaim: Incorrect number of parameters for HostBlackMarker" <<endl;
    exit(1);
  }

  isFullBlackMarker = true;
  hostAlt = "";
  domainAlt = "";

  for (int i = 0; i < param_names.size(); i++)
  {
    //if (verbose) cout << param_names[i] <<"="<<param_values[i] << endl;
    if (param_names[i] == HostBlackMarker::typeParam)
    {
      if(param_values[i] == "FullName") {
        isFullBlackMarker = true;
      } 
      else if(param_values[i] == "HostOnly") {
        isFullBlackMarker = false;
      } 
    }
    else if (param_names[i] == HostBlackMarker::hostAltParam) 
    {
      hostAlt = param_values[i];
    }
    else if (param_names[i] == HostBlackMarker::domainAltParam) 
    {
      domainAlt = param_values[i];
    }
  }

  if ((hostAlt == "") || (domainAlt == "")) {
    cerr << "flaim: Invalid/Incomplete parameter specification for HostBlackMarker" <<endl;
    exit(1);
  }

  if (verbose) cerr << "flaim: HostBlackMarker, replacement = \"" << hostAlt <<"."<< domainAlt <<"\""<< endl;

  lastID = 0;
}



HostBlackMarker::~HostBlackMarker()
{
  if (verbose) cerr << "flaim: Destructor of HostBlackMarker." << endl;
}

//    bool isFullBlackMarker;
//    string hostAlt;
//    string domainAlt;

//This aLgo only works with STRING data-type currently
int HostBlackMarker::anonymizeValue(Value *v)
{
  if (v->Type() != VAL_TYPE_STRING) {
    fprintf(stderr, "flaim: Non-STRING types not supported by HostBlackMarker yet\n");
    exit(1);
  }
  string *s = Value::to_string(v);
  if (isFullBlackMarker) 
  {
    //check for . separators in the value
    if(s->find_first_of(".") != string::npos) {
      //the field is a FQDN
      *s = hostAlt + "." + domainAlt; 
    } else {
      //the field only has the hostname
      *s = hostAlt; 
    }
  } else
  {
    //check for . separators in the value
    if(s->find_first_of(".") != string::npos) {
      //the field is a FQDN only replace the host part
      string str_tmp = hostAlt + s->substr(s->find_first_of("."));
      *s = str_tmp;
    } else {
      //the field only has the hostname replace the entire thing
      *s = hostAlt; 
    }
  }
  return 0;
}








//note: in and out are assumed non null
void to_hexstr(unsigned char*in, char *out) 
{
  //if(verbose) cerr << "IN:"<<in<<endl;
  for(int x=0,y=0; x < 16; x++) 
  {
    uint32_t n = 0; 
    //chew the lower nibble
    n = (in[x] & 0xF0) >> 4;
    out[y++] = "0123456789abcdef"[n];
    if(verbose) fprintf(stderr,"%c",out[y-1]);
    //bite the upper nibble
    n = (in[x] & 0x0F);
    out[y++] = "0123456789abcdef"[n];
    if(verbose) fprintf(stderr,"%c",out[y-1]);
    n = in[x];
    if(verbose) fprintf(stderr,"(%x) ",n);
  }
  if(verbose) cerr << endl;
}


const char* const HostHash::typeParam = "type"; //options: MD5

//Full Hostname Hashing - this needs to be figured out
HostHash::HostHash(vector<string> & param_names,vector<string> & param_values)
: AnonyAlg(param_names, param_values)
{
  if (verbose) cerr << "flaim: Constructor of HostHash." << endl;
  if( (param_names.size() != 1) || (param_values.size() != 1) ) {
    cerr << "flaim: Incorrect number of parameters for HostHash" <<endl;
    exit(1);
  }
  isMD5Digest = false;
  if (param_names[0] == HostHash::typeParam)
  {
    if(param_values[0] == "MD5") {
      isMD5Digest = true;
    } else {
      cerr <<"flaim: Invalid param value for HostHash" <<endl;
      exit(1);
    }
  } else {
    cerr <<"flaim: Invalid param name for HostHash" <<endl;
    exit(1);
  }
}

int HostHash::anonymizeValue(Value *v)
{
  if (v->Type() != VAL_TYPE_STRING) 
  {
    fprintf(stderr, "flaim: Non-STRING types not supported by HostBlackMarker yet\n");
    exit(1);
  }
  string *s = Value::to_string(v);
  unsigned char md5_digest[17] = {0};
  //if(verbose) cerr << "flaim: Before - "<<*s<<endl; 
  if(isMD5Digest) 
  {
    // md5Hash( (char*)s->c_str(), md5_digest); //incorrect but still safe 
    MD5((const unsigned char*)s->c_str(), strlen(s->c_str()), md5_digest);
    char temp[33] = {0};
    to_hexstr(md5_digest, temp);
    
    *s = string(temp);
    //if(verbose) cerr << "flaim: After  - "<<*s<<endl; 
    //remember to fix the size in value
  } else {
    //add other hasing algos here
  }
  return 0;
}

HostHash::~HostHash() 
{
}



