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
// $Id: anony_ipaddr.cpp 983 2007-10-30 05:40:58Z byoo3 $
//
// This file contains the defs for the ip address specific algos
// defined in anony_algos.hpp
//
//********************************************************************


#include "anony_algos.hpp"
#include "core_globals.h"
#include <stdint.h>


const static unsigned int IPMAX = ~0;
const static unsigned int MACMAX = (1<<24) - 1;
const static unsigned int PORTMAX = 65536;




/*
//IP Prefix-Preserving 

//virtual constructor
IpPrefixPresv::IpPrefixPresv(vector<string> & param_names, vector<string> & param_values)
: AnonyAlg(param_names, param_values)
{
  if (verbose) cerr << "flaim: Constructor of IpPrefixPresv." << endl;
  if ((param_names.size() != 1) || (param_names.size() != 1)) {
    cerr <<"flaim: incorrect num of params for "<<algoName<< endl;
    exit(1);
  }
  if(param_names[0]== "passphrase")
  { 	   
    //if (verbose) cerr  <<"flaim: (params) "<< param_names[0] << ": " <<  param_values[0] << endl;
    for(int i=0;i<16;i++) my_pad[i] = 0; 
      
    //generate the secret key for prefix-preserving
    md5Hash((char *) param_values[0].c_str(), my_key);
    memcpy(key,(const unsigned char *) my_key, sizeof(my_key));
    md5Hash((char *) my_key, my_pad); // pad = md5Hash(key)
    memcpy(key+16,(const unsigned char *) my_pad, sizeof(my_pad));

    // construct an object of PAnonymizer with the key
    my_anonymizer.reset( new PAnonymizer(my_key));
  }
  else
  {
    //FIXME: isnt this being checked via schema validation ???
    cerr << "flaim: Invalid parameter for IP Prefix-Preserving." << endl;      
    exit(1);
  }

  lastID = 0;
}

//virtual destructor
IpPrefixPresv::~IpPrefixPresv()
{
  if (verbose) cerr << "flaim: Destructor of IpPrefixPresv. " << endl;  
}


//anonymization function 
int IpPrefixPresv::anonymizeValue(Value *v) 
{
      uint32_t *ipAddr = Value::to_uint32(v);
       uint32_t ret;
       ret = my_anonymizer->anonymize((*ipAddr));
       (*ipAddr) = ret;

  return 0;
}
*/



/*Binary Prefix-Preserving */

//virtual constructor
BinaryPrefixPresv::BinaryPrefixPresv(vector<string> & param_names, vector<string> & param_values)
: AnonyAlg(param_names, param_values)
{
  if (verbose) cerr << "flaim: Constructor of BinaryPrefixPresv." << endl;
  if ((param_names.size() != 1) || (param_names.size() != 1)) {
    cerr <<"flaim: incorrect num of params for "<<algoName<< endl;
    exit(1);
  }
  if(param_names[0]== "passphrase")
  { 	   
    //if (verbose) cerr  <<"flaim: (params) "<< param_names[0] << ": " <<  param_values[0] << endl;
    for(int i=0;i<16;i++) my_pad[i] = 0; 
      
    //generate the secret key for prefix-preserving
    // md5Hash((char *) param_values[0].c_str(), my_key);
    MD5((const unsigned char*)param_values[0].c_str(), strlen(param_values[0].c_str()), my_key);
    memcpy(key,(const unsigned char *) my_key, sizeof(my_key));
    // md5Hash((char *) my_key, my_pad); // pad = md5Hash(key)
    MD5((const unsigned char*)my_key, strlen((const char*)my_key), my_pad);
    memcpy(key+16,(const unsigned char *) my_pad, sizeof(my_pad));

    // construct an object of PAnonymizer with the key
    my_anonymizer.reset( new PAnonymizer(my_key));
  }
  else
  {
    //FIXME: isnt this being checked via schema validation ???
    cerr << "flaim: Invalid parameter for Binary Prefix-Preserving." << endl;      
    exit(1);
  }

  lastID = 0;
}

//virtual destructor
BinaryPrefixPresv::~BinaryPrefixPresv()
{
  if (verbose) cerr << "flaim: Destructor of BinaryPrefixPresv. " << endl;  
}


//anonymization function 
int BinaryPrefixPresv::anonymizeValue(Value *v) 
{
  switch (v->Type())
  {
    case VAL_TYPE_UINT8:
    case VAL_TYPE_UINT16:
    case VAL_TYPE_UINT32:
    case VAL_TYPE_INT8:
	case VAL_TYPE_INT16:
    case VAL_TYPE_INT32:
    {
       uint32_t *ipAddr = Value::to_uint32(v);
       uint32_t ret;
       ret = my_anonymizer->anonymize((*ipAddr));
       (*ipAddr) = ret;
	   return 0;
	}
	case VAL_TYPE_CHAR_PTR:
    {
        unsigned char *ipAddr = (unsigned char *) (Value::to_char_ptr(v));
        unsigned int numBytes = v->Bytes();  
		
		if (numBytes>16){
		   cerr << "flaim BinaryPrefixPresv: data length exceeds the limit" <<endl;
           exit(1);
		}
        my_anonymizer->anonymize(ipAddr, numBytes);
		return 0;
    }
	default:
	{
	   cerr << "flaim BinaryPrefixPresv: Unsupported data type" <<endl;
        exit(1);
	}
  }

}
