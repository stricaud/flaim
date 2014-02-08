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
// $Id: value.h 237 2006-07-27 10:31:56Z vdhar $
//
// Header file for a generic Value class, which encapsulates pieces 
// of data so that they can be sent to FLAIM
//
//********************************************************************


#ifndef _VALUE_H
#define _VALUE_H
#include <inttypes.h>
#include <iostream>
#include <string>
#include "value_types.h"

using namespace std;

class Value
{
    public:

	// no-argument ctor -- used to create a "null" value for comparisons
	Value();
	
	// 3-argument ctor -- initialize each field of the value
	Value(void *ptr_i, int type_i, int bytes_i);

	// the Big Three
        /*
	~Value();
	Value(Value const & origVal);
	Value const & operator=(Value const & origVal);
        */
	

	// get functions
	void* Ptr() const;
	int Type() const;
	int Bytes() const;

        void setPtr(void* p) {
          ptr = p;
        }
        void setType(int t) {
          type = t;
        }
        void setBytes(int b) {
          bytes = b;
        }
        
        //Type conversion utility functions
        static uint32_t* to_uint32(Value *v);
        static uint16_t* to_uint16(Value *v);
        static uint8_t*  to_uint8(Value *v);

        static int32_t* to_int32(Value *v);
        static int16_t* to_int16(Value *v);
        static int8_t*  to_int8(Value *v);
        
        static char*  to_char_ptr(Value *v);
        static string*  to_string(Value *v);

        void dumpPrint(char * msg) {
          if (ptr != NULL) {
            fprintf (stderr, "flaim: %s >> ", msg);
            //char *c = (char *)(ptr);
            const char *c;
            if (type != VAL_TYPE_STRING) {
              c = (char *)(ptr);
            } else {
              c = ((string*)ptr)->c_str();
              bytes = ((string*)ptr)->size();
            }
            uint32_t d = 0;
            for (int i=bytes-1; i >= 0; i--) {
              d = c[i] && 0xff;
              cerr << hex << ((uint32_t)c[i] & 0xff) << dec << "-";
              //fprintf (stderr, "%X:", d);
            }
            fprintf(stderr, "\n");
          }
        }
        
    private:

	void *ptr; // pointer to the encapsulated data
	int type; // integer identifier of that data's type, for casting
	int bytes; // size of the data in bytes

};

#endif
