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
// $Id: panonymizer.h 950 2007-09-07 01:31:13Z xluo1 $
//
// Package: NetflowAnonymizer
// File: panonymizer.cpp
// Last Update: 2004-11-8
// Authors: Jinliang Fan and Adam Slagell
//
//********************************************************************


#ifndef _PANONYMIZER_H_
#define _PANONYMIZER_H_

#include "rijndael.h"
#include "flaim_core.hpp"

class PAnonymizer { //Prefix-preserving anonymizer
  public:
    // Contructor need a 256-bit key
    // The first 128 bits of the key are used as the secret key for rijndael cipher
    // The second 128 bits of the key are used as the secret pad for padding
    PAnonymizer(const UINT8 * key);
    ~PAnonymizer();
  protected:
    UINT8 m_key[16]; //128 bit secret key
    UINT8 m_pad[16]; //128 bit secret pad
    Rijndael m_rin;  //Rijndael cipher as pseudorandom function

  public:
    UINT32 anonymize( const UINT32 orig_addr);   
    int anonymize(unsigned char* orig_addr, unsigned int numBytes);   
};

#endif //_PANONYMIZER_H_ 
