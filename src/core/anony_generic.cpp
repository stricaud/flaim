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
// $Id: anony_generic.cpp 1045 2008-02-04 21:50:48Z byoo3 $
//
// This file contains the defs of the generic anony algos defined
// in anony_algos.hpp
// These algos have the property that they are applicable to multiple
// types of fields
//
//********************************************************************

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <list>
#include <stdint.h>
#include "core_globals.h"
#include "listed_record.h"
#include "anony_algos.hpp"

//#define DEBUG // comment this out to remove verbose debugging messages


///////////////////////////////////////
//Generic/Common Utility Functions Def
//There are decl in the anony_algos.hpp

void Tokenize(const string& str, vector<string>& tokens,
        const string& delimiters) {
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);
    
    while (string::npos != pos || string::npos != lastPos) {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}


void get_uint32(Value *v, uint32_t &i) {
    switch (v->Type()) {
        case VAL_TYPE_UINT8: {
            uint8_t c = *Value::to_uint8(v);
            i = (uint32_t)c;
            return;
        }
        case VAL_TYPE_UINT16: {
            uint16_t c = *Value::to_uint16(v);
            i = (uint32_t)c;
            return;
        }
        case VAL_TYPE_UINT32: {
            uint32_t c = *Value::to_uint32(v);
            i = (uint32_t)c;
            return;
        }
        
        
        //FIXME: Verify that the signed int cases are working correctly
        case VAL_TYPE_INT8: {
            int8_t c = *Value::to_int8(v);
            i = (uint32_t)c;
            return;
        }
        case VAL_TYPE_INT16: {
            int16_t c = *Value::to_int16(v);
            i = (uint32_t)c;
            return;
        }
        case VAL_TYPE_INT32: {
            int32_t c = *Value::to_int32(v);
            i = (uint32_t)c;
            return;
        }
        
        
        case VAL_TYPE_CHAR_PTR: {
            cerr << "flaim: char* Unsupported... ABORT !!!!!!!!!!!!" <<endl;
            exit(1);
        }
        
        case VAL_TYPE_STRING: {
            cerr << "flaim: string Unsupported... ABORT !!!!!!!!!!!!" <<endl;
            exit(1);
        }
        
        default: {
            cerr << "flaim: Unknown Unsupported... ABORT !!!!!!!!!!!!" <<endl;
            exit(1);
        }
    }
    
    cerr << "flaim: set/get_intxx Reached an invalid location... ABORT !!!!!!!!!!!!" <<endl;
    exit(1);
    return;
}


void set_uint32(Value *v, uint32_t &i) {
    switch (v->Type()) {
        case VAL_TYPE_UINT8: {
            uint8_t c = *Value::to_uint8(v);
            c = (uint8_t)i;
            memcpy(v->Ptr(), &c, v->Bytes());
            return;
        }
        case VAL_TYPE_UINT16: {
            uint16_t c = *Value::to_uint16(v);
            c = (uint16_t)i;
            memcpy(v->Ptr(), &c, v->Bytes());
            return;
        }
        case VAL_TYPE_UINT32: {
            uint32_t c = *Value::to_uint32(v);
            c = (uint32_t)i;
            memcpy(v->Ptr(), &c, v->Bytes());
            return;
        }
        
        
        //FIXME: Verify that the signed int cases are working correctly
        case VAL_TYPE_INT8: {
            int8_t c = *Value::to_int8(v);
            c = (int8_t)i;
            memcpy(v->Ptr(), &c, v->Bytes());
            return;
        }
        case VAL_TYPE_INT16: {
            int16_t c = *Value::to_int16(v);
            c = (int16_t)i;
            memcpy(v->Ptr(), &c, v->Bytes());
            return;
        }
        case VAL_TYPE_INT32: {
            int32_t c = *Value::to_int32(v);
            c = (int32_t)i;
            memcpy(v->Ptr(), &c, v->Bytes());
            return;
        }
        
        
        case VAL_TYPE_CHAR_PTR: {
            cerr << "flaim: char* Unsupported... ABORT !!!!!!!!!!!!" <<endl;
            exit(1);
        }
        
        case VAL_TYPE_STRING: {
            cerr << "flaim: string Unsupported... ABORT !!!!!!!!!!!!" <<endl;
            exit(1);
        }
        
        default: {
            cerr << "flaim: Unknown Unsupported... ABORT !!!!!!!!!!!!" <<endl;
            exit(1);
        }
    }
    
    cerr << "Reached an invalid location... ABORT !!!!!!!!!!!!" <<endl;
    exit(1);
    return;
}


void get_int64(Value *v, int64_t &i) {
    switch (v->Type()) {
        case VAL_TYPE_UINT8: {
            uint8_t c = *Value::to_uint8(v);
            i = (int64_t)c;
            return;
        }
        case VAL_TYPE_UINT16: {
            uint16_t c = *Value::to_uint16(v);
            i = (int64_t)c;
            return;
        }
        case VAL_TYPE_UINT32: {
            uint32_t c = *Value::to_uint32(v);
            i = (int64_t)c;
            return;
        }
        
        
        //FIXME: Verify that the signed int cases are working correctly
        case VAL_TYPE_INT8: {
            int8_t c = *Value::to_int8(v);
            i = (int64_t)c;
            return;
        }
        case VAL_TYPE_INT16: {
            int16_t c = *Value::to_int16(v);
            i = (int64_t)c;
            return;
        }
        case VAL_TYPE_INT32: {
            int32_t c = *Value::to_int32(v);
            i = (int64_t)c;
            return;
        }
        
        
        case VAL_TYPE_CHAR_PTR: {
            cerr << "flaim: char* Unsupported... ABORT !!!!!!!!!!!!" <<endl;
            exit(1);
        }
        
        case VAL_TYPE_STRING: {
            cerr << "flaim: string Unsupported... ABORT !!!!!!!!!!!!" <<endl;
            exit(1);
        }
        
        default: {
            cerr << "flaim: Unknown Unsupported... ABORT !!!!!!!!!!!!" <<endl;
            exit(1);
        }
    }
    
    cerr << "Reached an invalid location... ABORT !!!!!!!!!!!!" <<endl;
    exit(1);
    return;
}


void set_int64(Value *v, int64_t &i) {
    switch (v->Type()) {
        case VAL_TYPE_UINT8: {
            uint8_t c = *Value::to_uint8(v);
            c = (uint8_t)i;
            memcpy(v->Ptr(), &c, v->Bytes());
            return;
        }
        case VAL_TYPE_UINT16: {
            uint16_t c = *Value::to_uint16(v);
            c = (uint16_t)i;
            memcpy(v->Ptr(), &c, v->Bytes());
            return;
        }
        case VAL_TYPE_UINT32: {
            uint32_t c = *Value::to_uint32(v);
            c = (uint32_t)i;
            memcpy(v->Ptr(), &c, v->Bytes());
            return;
        }
        
        
        //FIXME: Verify that the signed int cases are working correctly
        case VAL_TYPE_INT8: {
            int8_t c = *Value::to_int8(v);
            c = (int8_t)i;
            memcpy(v->Ptr(), &c, v->Bytes());
            return;
        }
        case VAL_TYPE_INT16: {
            int16_t c = *Value::to_int16(v);
            c = (int16_t)i;
            memcpy(v->Ptr(), &c, v->Bytes());
            return;
        }
        case VAL_TYPE_INT32: {
            int32_t c = *Value::to_int32(v);
            c = (int32_t)i;
            memcpy(v->Ptr(), &c, v->Bytes());
            return;
        }
        
        
        case VAL_TYPE_CHAR_PTR: {
            cerr << "flaim: char* Unsupported... ABORT !!!!!!!!!!!!" <<endl;
            exit(1);
        }
        
        case VAL_TYPE_STRING: {
            cerr << "flaim: string Unsupported... ABORT !!!!!!!!!!!!" <<endl;
            exit(1);
        }
        
        default: {
            cerr << "flaim: Unknown Unsupported... ABORT !!!!!!!!!!!!" <<endl;
            exit(1);
        }
    }
    
    cerr << "flaim: set/get_intxx Reached an invalid location... ABORT !!!!!!!!!!!!" <<endl;
    exit(1);
    return;
}



/*
 * //Generalized random permutation
 *
 * RandPermute::RandPermute(vector<string> & param_names, vector<string> & param_values)
 * : AnonyAlg(param_names, param_values)
 * {
 * if (verbose) cerr << "flaim: Constructor of RandomPermute." << endl;
 *
 * mapTable.clear();
 * entryTable.clear();
 * mapCount = 0;
 *
 * lastID = 0;
 *
 * if ((fp = fopen("/dev/urandom","rb"))==NULL){
 * cerr << " Failed to open /dev/urandom." << endl;
 * exit(1);
 * }
 * }
 *
 * RandPermute::~RandPermute()
 * {
 * if (verbose) cerr << "flaim: Destructor of RandomPermute. " << endl;
 *
 * mapTable.clear();
 * entryTable.clear();
 * mapCount = 0;
 *
 * if (fp != NULL)
 * fclose(fp);
 * }
 *
 * int RandPermute::anonymizeValue(Value *v)
 * {
 * //---- algo ----//
 * //uint32_t *c = (uint32_t *)(v->Ptr());
 * uint32_t num;
 * get_uint32(v, num);
 * uint32_t *c = &num;
 *
 * permuteKey = *c;
 * //check if the num is in the hashtable
 * if (mapTable.find(permuteKey) != mapTable.end())
 * {
 * //use the existed mapping
 * (*c) = mapTable[permuteKey];
 * }
 * else //create a new mapping
 * {
 * do
 * {
 * // get the permuteEntry from /dev/urandom
 * if(fread(&permuteEntry,sizeof(unsigned int),1,fp)!=1)
 * {
 * cerr << " Failed to get the permuteEntry from /dev/urandom." << endl;
 * exit(1);
 * }
 *
 * }while (entryTable.find(permuteEntry) != entryTable.end()); //the generated num has been used
 *
 * if (verbose)
 * cerr << "permuteEntry = " << (unsigned int)permuteEntry << endl;
 *
 * //add the new pair to the mapping table and mark it in the entry table
 * mapTable.insert(intPair(permuteKey, permuteEntry));
 * entryTable.insert(intPair(permuteEntry, ++mapCount));
 * (*c) = permuteEntry;
 * }
 * set_uint32(v, num);
 *
 * return 0;
 * }
 */


//Binary random permutation

BinaryRandPermute::BinaryRandPermute(vector<string> & param_names, vector<string> & param_values)
: AnonyAlg(param_names, param_values) {
    if (verbose) cerr << "flaim: Constructor of BinaryRandomPermute." << endl;
    
    intMapTable.clear();
    intEntryTable.clear();
    strMapTable.clear();
    strEntryTable.clear();
    mapCount = 0;
    
    secondaryFieldName = "";
    
    if (param_names.size() != param_values.size()) {
        cerr << "flaim: mismatch in num of params for RandomPermutation" <<endl;
        exit(1);
    }
    
    for (int i = 0; i < param_values.size(); i++) {
        if (param_names[i] == TSAnonyAlg::secondParam) {
            secondaryFieldName = param_values[i];
        }
    }
    
    lastID = 0;
    
    if ((fp = fopen("/dev/urandom", "rb")) == NULL){
        cerr << " Failed to open /dev/urandom." << endl;
        exit(1);
    }
}


BinaryRandPermute::~BinaryRandPermute() {
    if (verbose) cerr << "flaim: Destructor of BinaryRandomPermute. " << endl;
    
    intMapTable.clear();
    intEntryTable.clear();
    strMapTable.clear();
    strEntryTable.clear();
    mapCount = 0;
    
    if (fp != NULL)
        fclose(fp);
}

int BinaryRandPermute::anonymizeValue(Value *v) {
    //---- algo ----//
    
    switch (v->Type()) {
        case VAL_TYPE_UINT8:
        case VAL_TYPE_UINT16:
        case VAL_TYPE_UINT32:
        case VAL_TYPE_INT8:
        case VAL_TYPE_INT16:
        case VAL_TYPE_INT32: {
            uint32_t num, num2; // initVal;
            get_uint32(v, num);
            uint32_t *c = &num;
            // initVal = *c;            
            intPermuteKey = *c;
            // check if the num is in the hashtable
            if (intMapTable.find(intPermuteKey) != intMapTable.end()) {
                // use the existed mapping
                (*c) = intMapTable[intPermuteKey];
            }
            else // create a new mapping
            {
                do {
                    /* get the permuteEntry from /dev/urandom */
                    if(fread(&intPermuteEntry, sizeof(unsigned int), 1, fp)!=1) {
                        cerr << " Failed to get the permuteEntry from /dev/urandom." << endl;
                        exit(1);
                    }
                } while (intEntryTable.find(intPermuteEntry) != intEntryTable.end()); //the generated num has been used
                
                if (verbose)
                    cerr << "intPermuteEntry = " << (unsigned int)intPermuteEntry << endl;
                
                // add the new pair to the mapping table and mark it in the entry table
                intMapTable.insert(intPair(intPermuteKey, intPermuteEntry));
                intEntryTable.insert(intPair(intPermuteEntry, ++mapCount));
                (*c) = intPermuteEntry;
            }
            set_uint32(v, num);
            
            // anonymize the secondary field
            if (secondaryFieldName != "") {
                Value val;
                Value* v2 = &val;
                if (verbose)
                    cerr << secondField << endl;
                
                // Ingore the secondField if not exist
                if (0 != coreList[lastID].theRecord->getField((char*)secondaryFieldName.c_str(), val)) {
                    --(coreList[lastID].refCount);
                    ++lastID;
                    return 0;
                }
                
                // if (verbose) val.dumpPrint("Before2");
                
                // FIXME: check return value here ?
                // anonymizeValue(v2);
                get_uint32(v2, num2);
                if (UINT32_MAX - num < num2 - intPermuteKey) {
                    num2 = UINT32_MAX;
                    set_uint32(v2, num2);
                } else {
                    num2 = num + (num2 - intPermuteKey);
                    set_uint32(v2, num2);
                }
                // if (verbose) val.dumpPrint("After2");
            }
            
            return 0;
        }
        case VAL_TYPE_CHAR_PTR: {
            unsigned char *c = (unsigned char *) (Value::to_char_ptr(v));
            // char *c = (char *) (Value::to_char_ptr(v));
            unsigned int numBytes = v->Bytes();
            strPermuteKey = string((char *)c, numBytes);
            
            char tempKey[100];
            
            //check if the num is in the hashtable
            if (strMapTable.find(strPermuteKey) != strMapTable.end()) {
                //use the existed mapping
                memcpy(c, strMapTable[strPermuteKey].c_str(), numBytes);
            }
            else //create a new mapping
            {
                if (verbose){
                    for (int i=0; i< numBytes; i++)  cerr << (unsigned short)c[i] <<".";
                    cerr << endl;
                }
                do {
                    // get the permuteEntry from /dev/urandom
                    if(fread(tempKey, numBytes, 1, fp)!=1) {
                        cerr << " Failed to get the permuteEntry from /dev/urandom." << endl;
                        exit(1);
                    }
                    else
                        strPermuteEntry = string(tempKey, numBytes);
                    
                }while (strEntryTable.find(strPermuteEntry) != strEntryTable.end()); //the generated num has been used
                
                
                //add the new pair to the mapping table and mark it in the entry table
                strMapTable.insert(strPair(strPermuteKey, strPermuteEntry));
                strEntryTable.insert(mixPair(strPermuteEntry, ++mapCount));
                memcpy(c, strPermuteEntry.c_str(), numBytes);
                
                if (verbose){
                    for (int i=0; i< numBytes; i++)  cerr << (unsigned short) c[i] <<".";
                    cerr << endl;
                }
            }
            
            return 0;
        }
        default: {
            cerr << "flaim BinaryRandPermute: Unsupported data type" <<endl;
            exit(1);
        }
    }//switch
}



const char* const Classify::cfgStrParam = "configString";

//Generalized classification
Classify::Classify(vector<string> & param_names, vector<string> & param_values)
: AnonyAlg(param_names, param_values), binSpec() {
    if (verbose) cerr << "flaim: Constructor of Classify. " << endl;
    
    if( (param_names.size() != 1) || (param_values.size() != 1) ||
            (param_names[0] != Classify::cfgStrParam)) {
        cerr << "flaim: Incorrect number of parameters for Classifiy" <<endl;
        exit(1);
    }
    if (verbose) cerr << "flaim: Classify config string = " << param_values[0] << endl;
    
    vector<string> binList, bin;
    int64_t a , b, last_a = 0, last_b = 0 ;
    Tokenize(param_values[0], binList, ",");
    
    for (int j=0; j < binList.size(); j++) {
        if (verbose) cerr << "flaim: binList["<<j<<"] = " << binList[j] << endl;
        bin.clear();
        Tokenize(binList[j], bin, ":");
        
        for (int x = 0 ; x < bin.size(); x ++)
            if (verbose) cerr << "flaim: bin = " << bin[x] << endl;
        
        if (bin.size()!=2 ) {
            cerr << "flaim: Invalid config string for classify" << endl;
            exit(1);
        }
        a = atol(bin[0].c_str());
        b = atol(bin[1].c_str());
        if ((j > 0) && ((a <= last_a ) || (b <= last_b ))) {
            cerr << "flaim: config string for classify has to be sorted" << endl;
            exit(1);
        }
        binSpec.push_back(pair<int64_t, int64_t > (a, b)  );
        last_a = a;
        last_b = b;
        
        pair<int64_t, int64_t > p;
        p = binSpec[j];
        if (verbose) cerr << "flaim: NOW CLASSIFY " << p.first <<"=" << p.second << endl;
    }
    
    lastID = 0;
    
    /*
  binSpec.push_back(pair<uint32_t, uint32_t > (50, 1)  );
  binSpec.push_back(pair<uint32_t, uint32_t > (2048, 2)  );
  binSpec.push_back(pair<uint32_t, uint32_t > (8192, 3)  );
  binSpec.push_back(pair<uint32_t, uint32_t > (16384, 4) );
   
  pair<uint32_t, uint32_t > p;
  p = binSpec[0];
  cerr << p.first <<"=" << p.second << endl;
     */
}

Classify::~Classify() {
    if (verbose) cerr << "flaim: Destructor of Classify. " << endl;
}

int Classify::anonymizeValue(Value *v) {
    //---- algo ----//
    int64_t n = 125, flg = 0;
    int less = 0, more = 0;
    get_int64(v, n);
    
    for (int i = 0;i < binSpec.size(); i++) {
        pair<int64_t, int64_t > p;
        p = binSpec[i];
        if (verbose) cerr << "flaim: data:"<< n <<" bin["<<i<<"]:"<< p.first <<endl;
        
        if (n > p.first) {
            more = 1;
            if (less == 1) {
                cerr << "flaim: BIGGER mortal before SMALLER ones... unsorted classify config string !!" <<endl;
                exit(1);
            }
        }
        else if (n < p.first) {
            less = 1;
            if (more == 1) {
                flg = 1;
                n = p.second;
                if (verbose) cerr << "flaim: GOTCHA after first !! binned to = " << n <<endl;
                break;
            } else {
                flg = 1;
                n = p.second;
                if (verbose) cerr << "flaim: GOTCHA on first !! binned to = " << n <<endl;
                break;
            }
        }
        else if (n == p.first) {
            less = 1;
            if (more == 1) {
                flg = 1;
                n = p.second;
                if (verbose) cerr << "flaim: MATCH !! changed to = " << n <<endl;
                break;
            } else {
                flg = 1;
                n = p.second;
                if (verbose) cerr << "flaim: MATCH on first !! changed to = " << n <<endl;
                break;
            }
        }
        
    }
    if (verbose) cerr << "flaim: Broke out, new value = " << n <<endl;
    set_int64(v, n);
    //return v;
    
    return 0;
}




const char* const BinaryBlackMarker::numMarksParam = "numMarks";
const char* const BinaryBlackMarker::replacementParam = "replacement";


//Generalized BinaryBlackmarker - Numeric and byte array only
BinaryBlackMarker::BinaryBlackMarker(vector<string> & param_names, vector<string> & param_values)
: AnonyAlg(param_names, param_values) {
    if (verbose) cerr << "flaim: Constructor of BinaryBlackMarker." << endl;
    
    
    if( (param_names.size() != 2) || (param_values.size() != 2) ) {
        cerr << "flaim: Incorrect number of parameters for BinaryBlackMarker" <<endl;
        exit(1);
    }
    
    numMarks = -1;
    replacement = 0;
    
    for (int i = 0; i < param_names.size(); i++) {
        if (param_names[i] == BinaryBlackMarker::numMarksParam) {
            numMarks = (uint32_t)atol(param_values[i].c_str());
        }
        else if (param_names[i] == BinaryBlackMarker::replacementParam) {
            replacement = (uint32_t)atoi(param_values[i].c_str());
        }
    }
    
    if (numMarks < 0) {
        cerr << "flaim: Invalid/Incomplete parameter specification for BinaryBlackMarker" <<endl;
        exit(1);
    }
    
    //FIXME: i use atol here instead of atoi and then down cast to uint32
    //replacement = (uint32_t)atol(param_values[0].c_str());
    //keep the string around just in case
    //replacementStr = param_values[0];
    if (verbose) cerr << "flaim: BinaryBlackMarker, replacement = " << replacement << endl;
    
    lastID = 0;
}


BinaryBlackMarker::~BinaryBlackMarker() {
    if (verbose) cerr << "flaim: Destructor of BinaryBlackMarker." << endl;
}


int BinaryBlackMarker::anonymizeValue(Value *v) {
    int curNumMarks;
    
    if (v->Type() != VAL_TYPE_STRING) {
        if (numMarks > (v->Bytes()*8))
            curNumMarks = v->Bytes()*8;
        else
            curNumMarks = numMarks;
        
    }else {
        fprintf(stderr, "flaim: invalid data type for BinaryBlackMarker\n");
        exit(1);
    }
    
    switch (v->Type()) {
        case VAL_TYPE_UINT8: {
            uint8_t *c = Value::to_uint8(v);
            uint8_t mask = 0;
            mask = (~0ULL)<<curNumMarks;
            if (replacement == 0) {
                *c = *c & mask;
            } else {
                mask = ~(mask);
                *c = *c | mask;
            }
            break;
        }
        
        case VAL_TYPE_UINT16: {
            uint16_t *c = Value::to_uint16(v);
            uint16_t mask = 0;
            mask = (~0ULL)<<curNumMarks;
            
            if (replacement == 0) {
                *c = *c & mask;
            } else {
                mask = ~(mask);
                *c = *c | mask;
            }
            break;
        }
        
        case VAL_TYPE_UINT32: {
            uint32_t *c = Value::to_uint32(v);
            uint32_t mask = 0;
            mask = (~0ULL)<<curNumMarks;
            
            if (replacement == 0) {
                *c = *c & mask;
            } else {
                mask = ~(mask);
                *c = *c | mask;
            }
            
            break;
        }
        
        
        //FIXME: Verify that the signed int cases are working correctly
        case VAL_TYPE_INT8: {
            int8_t *c = (int8_t*)Value::to_int8(v);
            uint8_t mask = 0;
            mask = (~0ULL)<<curNumMarks;
            
            if (replacement == 0) {
                *c = *c & mask;
            } else {
                mask = ~(mask);
                *c = *c | mask;
            }
            
            break;
        }
        case VAL_TYPE_INT16: {
            uint16_t *c = (uint16_t*)Value::to_int16(v);
            uint16_t mask = 0;
            mask = (~0ULL)<<curNumMarks;
            
            if (replacement == 0) {
                *c = *c & mask;
            } else {
                mask = ~(mask);
                *c = *c | mask;
            }
            
            break;
        }
        case VAL_TYPE_INT32: {
            uint32_t *c = (uint32_t*)Value::to_int32(v);
            uint32_t mask = 0;
            mask = (~0ULL)<<curNumMarks;
            
            if (replacement == 0) {
                *c = *c & mask;
            } else {
                mask = ~(mask);
                *c = *c | mask;
            }
            
            break;
        }
        
        
        case VAL_TYPE_CHAR_PTR: {
            char *c = Value::to_char_ptr(v);
            unsigned int cSize = v->Bytes();
            unsigned int numBytes = curNumMarks/8;
            unsigned int numBits = curNumMarks%8;
            
            for (int i = cSize-1; i> (int)(cSize-numBytes-1); i--) {
                if (replacement == 0) {
                    c[i] = (uint8_t)0x00;
                } else {
                    c[i] = (uint8_t)0xFF;
                }
            }
            
            if (numBits > 0) {
                uint8_t mask = 0;
                mask = (~0UL)<<numBits;
                if (replacement == 0) {
                    c[(int)(cSize-numBytes-1)] = c[(int)(cSize-numBytes-1)] & mask;
                } else {
                    mask = ~(mask);
                    c[(int)(cSize-numBytes-1)] = c[(int)(cSize-numBytes-1)] | mask;
                }
            }
            break;
        }
        
        default:
            break;
    }
    
    return 0;
}


const char* const StringBlackMarker::numMarksParam = "numMarks";
const char* const StringBlackMarker::replacementParam = "replacement";

//Generalized StringBlackmarker - String only
StringBlackMarker::StringBlackMarker(vector<string> & param_names, vector<string> & param_values)
: AnonyAlg(param_names, param_values) {
    if (verbose) cerr << "flaim: Constructor of StringBlackMarker." << endl;
    
    if( (param_names.size() != 2) || (param_values.size() != 2) ) {
        cerr << "flaim: Incorrect number of parameters for StringBlackMarker" <<endl;
        exit(1);
    }
    
    numMarks = -1;
    replacementStr = "";
    
    for (int i = 0; i < param_names.size(); i++) {
        if (param_names[i] == StringBlackMarker::numMarksParam) {
            if (atol(param_values[i].c_str()) < 0){
                cerr << "flaim: Invalid number of marked chars for StringBlackMarker" <<endl;
                exit(1);
            }
            numMarks = (uint32_t)atol(param_values[i].c_str());
        }
        else if (param_names[i] == StringBlackMarker::replacementParam) {
            replacementStr = param_values[i];
        }
    }
    
    
    //FIXME: i use atol here instead of atoi and then down cast to uint32
    //replacement = (uint32_t)atol(param_values[0].c_str());
    //keep the string around just in case
    //replacementStr = param_values[0];
    if (verbose) cerr << "flaim: StringBlackMarker, replacementStr = " << replacementStr << endl;
    
    lastID = 0;
}


StringBlackMarker::~StringBlackMarker() {
    if (verbose) cerr << "flaim: Destructor of StringBlackMarker." << endl;
}


int StringBlackMarker::anonymizeValue(Value *v) {
    
    if (v->Type() != VAL_TYPE_STRING) {
        fprintf(stderr, "flaim: invalid data type for StringBlackMarker\n");
        exit(1);
    }
    else {
        
        string *str = Value::to_string(v);
        string &c = (*str);
        unsigned int cSize = c.size();
        
        if (numMarks < str->size()) { //blackmark the part of string
            for (int i = cSize-1; i> (int) (cSize-numMarks-1); i--){
                c[i] = replacementStr[0];
            }
        }
        else{  //blackmark the whole string
            for (int i = cSize-1; i>=0; i--){
                c[i] = replacementStr[0];
            }
        }
    }
    return 0;
}



//Generalized Annihilation: remove the unanonymized data field
Annihilation::Annihilation(vector<string> & param_names, vector<string> & param_values)
: AnonyAlg(param_names, param_values) {
    if(verbose) cerr << "flaim: Constructor of Annihilation." << endl;
    lastID = 0;
    /*
  if( (param_names.size() != 1) || (param_values.size() != 1) ) {
    cerr << "flaim: incorrect number of params for Annihilation" <<endl;
    exit(1);
  }
     */
}

Annihilation::~Annihilation() {
    if (verbose) cerr << "flaim: Destructor of Annihilation." << endl;
}

int Annihilation::anonymizeValue(Value *v) {
    switch (v->Type()) {
        case VAL_TYPE_UINT8: {
            uint8_t *c = Value::to_uint8(v);
            *c = 0;
            break;
        }
        case VAL_TYPE_UINT16: {
            uint16_t *c = Value::to_uint16(v);
            *c = 0;
            break;
        }
        case VAL_TYPE_UINT32: {
            uint32_t *c = Value::to_uint32(v);
            *c = 0;
            break;
        }
        
        
        //FIXME: Verify that the signed int cases are working correctly
        case VAL_TYPE_INT8: {
            int8_t *c = Value::to_int8(v);
            *c = 0;
            break;
        }
        case VAL_TYPE_INT16: {
            int16_t *c = Value::to_int16(v);
            *c = 0;
            break;
        }
        case VAL_TYPE_INT32: {
            int32_t *c = Value::to_int32(v);
            *c = 0;
            break;
        }
        
        
        case VAL_TYPE_CHAR_PTR:  //put each byte to 0
        {
            char *c = Value::to_char_ptr(v);
            //should not change the size of the char array
            for (int i = 0 ; i < v->Bytes(); i++ ) {
                c[i] = 0;
            }
            break;
        }
        case VAL_TYPE_STRING:  //replace the string with null string
        {
            string *str = Value::to_string(v);
            string &c = (*str);
            for (int i = 0 ; i < str->size(); i++ ) {
                c[i] = '\0';
            }
            break;
        }
        
        default:
            break;
    }
    return 0;
}









const char* const Substitute::subsParam = "substitute";

//Generalized Annihilation: remove the unanonymized data field
Substitute::Substitute(vector<string> & param_names, vector<string> & param_values)
: AnonyAlg(param_names, param_values) {
    if(verbose) cerr << "flaim: Constructor of Substitute." << endl;
    lastID = 0;
    
    if( (param_names.size() != 1) || (param_values.size() != 1) ||
            (param_names[0] != Substitute::subsParam) ) {
        cerr << "flaim: incorrect number of params for Substitute" <<endl;
        exit(1);
    }
    subsStr = param_values[0];
    subsNum = atoi(param_values[0].c_str()); //note no error checking here
    
    if (verbose) {
        cerr << "flaim: str=" << subsStr << ", num=" << subsNum << endl;
    }
}

Substitute::~Substitute() {
    if (verbose) cerr << "flaim: Destructor of Substitute." << endl;
}

int Substitute::anonymizeValue(Value *v) {
    if (v->Type() == VAL_TYPE_CHAR_PTR) {
        cerr << "flaim: char* is an invalid field type for substitute" <<endl;
    }
    
    if (v->Type() == VAL_TYPE_STRING) {
        *(Value::to_string(v)) = subsStr;
    } else {
        
        int64_t n = 0;
        get_int64(v, n);
        n = subsNum;
        set_int64(v, n);
    }
    return 0;
}

const char* const NumericTruncate::numBitsParam = "numShifts";
const char* const NumericTruncate::radixParam = "radix";
const char* const NumericTruncate::directionParam = "direction"; //optional

//Generalized truncation
NumericTruncate::NumericTruncate(vector<string> & param_names, vector<string> & param_values)
: AnonyAlg(param_names, param_values) {
    if (verbose) cerr << "flaim: Constructor of NumericTruncate. " << endl;
    shiftNum = -1;
    radix = -1;
    lsbFirst = true;
    
    if( (param_names.size() < 2) || (param_names.size() > 3) ||
            (param_values.size() != param_names.size()) ) {
        cerr << "flaim: Incorrect number of parameters for NumericTruncate" <<endl;
        exit(1);
    }
    
    for (int i = 0; i < param_values.size(); i++) {
        if (param_names[i] == NumericTruncate::numBitsParam) {
            shiftNum = atoi(param_values[i].c_str());
        }
        else if (param_names[i] == NumericTruncate::radixParam) {
            radix = atoi(param_values[i].c_str());
        }
        else if (param_names[i] == NumericTruncate::directionParam) {
            if (param_values[i] == "left") {
                lsbFirst = false;
            }
            else if (param_values[i] == "right") {
                lsbFirst = true;
            }
        }
    }
    
    if((shiftNum < 0) ||(radix == -1) || !((radix == 2) || (radix == 10)) ) {
        cerr <<"flaim: NumericTruncate, incomplete/invalid params" <<endl;
        exit(1);
    }
    
    if (verbose) {
        cerr << "flaim: NumericTruncate - shiftNum=" << shiftNum
                << ", radix=" << radix << ", lsbFirst=" <<lsbFirst << endl;
    }
    lastID = 0;
}

NumericTruncate::~NumericTruncate() {
    if (verbose) cerr << "flaim: Destructor of NumericTruncate. " << endl;
}

int NumericTruncate::anonymizeValue(Value *v) {
    int curShiftNum = 0;
    
    if (radix == 2) {
        //---- algo ----//
        
        switch (v->Type()) {
            case VAL_TYPE_UINT8:
            case VAL_TYPE_UINT16:
            case VAL_TYPE_UINT32:
            case VAL_TYPE_INT8:
            case VAL_TYPE_INT16:
            case VAL_TYPE_INT32: {
                int64_t num = 0;
                get_int64(v, num);
                
                if (shiftNum > (v->Bytes()*8))
                    curShiftNum = v->Bytes()*8;
                else
                    curShiftNum = shiftNum;
                
                if (lsbFirst) //shift to right
                    (num)>>=curShiftNum;
                else // shift to left
                    (num)<<=curShiftNum;
                
                set_int64(v, num);
                break;
            }
            default:
                cerr << "flaim: Invalid field type specified for NumericTruncate" <<endl;
                exit(1);
        }
    }
    else if (radix == 10) {
        int64_t num;
        int64_t div = 1;
        
        for(int i = 0; i< shiftNum; i++)
            div *= 10;
        
        if (verbose) cerr << "flaim: NumericTruncate, radix10, div=" <<div <<endl;
        get_int64(v, num);
        if (lsbFirst)
            num = num / div;
        else
            num = num * div;
        if (verbose) cerr << "flaim: NumericTruncate, result=" <<num <<endl;
        set_int64(v, num);
    }
    
    return 0;
}

const char* const StringTruncate::numCharsParam = "numChars";
const char* const StringTruncate::directionParam = "direction"; //optional

//Generalized truncation
StringTruncate::StringTruncate(vector<string> & param_names, vector<string> & param_values)
: AnonyAlg(param_names, param_values) {
    if (verbose) cerr << "flaim: Constructor of StringTruncate. " << endl;
    shiftNum = -1;
    lsbFirst = true;
    
    if( (param_names.size() < 1) || (param_names.size() > 2) ||
            (param_values.size() != param_names.size()) ) {
        cerr << "flaim: Incorrect number of parameters for StringTruncate" <<endl;
        exit(1);
    }
    
    for (int i = 0; i < param_names.size(); i++) {
        if (param_names[i] == StringTruncate::numCharsParam) {
            shiftNum = atoi(param_values[i].c_str());
        }
        else if (param_names[i] == StringTruncate::directionParam) {
            if (param_values[i] == "left") {
                lsbFirst = false;
            }
            else if (param_values[i] == "right") {
                lsbFirst = true;
            }
        }
    }
    
    if(shiftNum < 0) {
        cerr <<"flaim: StringTruncate, invalid no. of chars to be truncated" <<endl;
        exit(1);
    }
    
    if (verbose) {
        cerr << "flaim: StringTruncate - shiftNum=" << shiftNum
                << ", lsbFirst=" <<lsbFirst << endl;
    }
    lastID = 0;
}

StringTruncate::~StringTruncate() {
    if (verbose) cerr << "flaim: Destructor of StringTruncate. " << endl;
}

int StringTruncate::anonymizeValue(Value *v) {
    //---- algo ----//
    if (v->Type() != VAL_TYPE_STRING) {
        cerr << "flaim: Invalid field type specified for StringTruncate" <<endl;
        exit(1);
    }
    string *str = Value::to_string(v);
    string &c = (*str);
    
    if (shiftNum <= str->size() ) {
        
        if (lsbFirst) { //direction== right
            c.erase(c.size()-shiftNum);
        } else { //direction == left
            c.erase(0, shiftNum);
        }
    }
    else{  //erase the whole string
        c.erase(0, str->size());
    }
    
    return 0;
}

const char* const BytesTruncate::numBitsParam = "numBits";
const char* const BytesTruncate::directionParam = "direction"; //optional

BytesTruncate::BytesTruncate(vector<string> & param_names, vector<string> & param_values)
: AnonyAlg(param_names, param_values) {
    if (verbose) cerr << "flaim: Constructor of BytesTruncate. " << endl;
    shiftNum = -1;
    lsbFirst = true;  //default is truncating from the right
    
    if( (param_names.size() < 1) || (param_names.size() > 2) ||
            (param_values.size() != param_names.size()) ) {
        cerr << "flaim: Incorrect number of parameters for BytesTruncate" <<endl;
        exit(1);
    }
    
    for (int i = 0; i < param_names.size(); i++) {
        if (param_names[i] == BytesTruncate::numBitsParam) {
            shiftNum = atoi(param_values[i].c_str());
        }
        else if (param_names[i] == BytesTruncate::directionParam) {
            if (param_values[i] == "left") {
                lsbFirst = false;
            }
            else if (param_values[i] == "right") {
                lsbFirst = true; //
            }
        }
    }
    
    if(shiftNum < 0) {
        cerr <<"flaim: BytesTruncate, incomplete/invalid params" <<endl;
        exit(1);
    }
    if (verbose) {
        cerr << "flaim: BytesTruncate - shiftNum=" << shiftNum
                << ", lsbFirst=" <<lsbFirst << endl;
    }
    lastID = 0;
}


BytesTruncate::~BytesTruncate() {
    if (verbose) cerr << "flaim: Destructor of BytesTruncate. " << endl;
}

//Doesnt really truncate... instead inserts zeros at the other end
//to make sure that the field size remains the same.. eg. for mac addresses
int BytesTruncate::anonymizeValue(Value *v) {
    int curShiftNum;
    
    //---- algo ----//
    if (v->Type() != VAL_TYPE_CHAR_PTR) {
        cerr << "flaim: Invalid field type specified for BytesTruncate" <<endl;
        exit(1);
    }
    char *c = Value::to_char_ptr(v);
    
    if (shiftNum > (8*v->Bytes()))
        curShiftNum = 8*v->Bytes();
    else
        curShiftNum =  shiftNum;
    
    int dataBits = (8*v->Bytes());
    int trailBits = curShiftNum % 8;
    int byteShifts = curShiftNum / 8;
    uint8_t lowerMask = (1<<trailBits) - 1;
    uint8_t upperMask = ~lowerMask;
    
    if (lsbFirst) {//right shift
        int i;
        
        for (i = v->Bytes()-1; (i >= (byteShifts)); i--)
            c[i] = c[i - byteShifts];
        
        for (int j=0;j<byteShifts;j++) {
            c[j] = 0;
            //NOTE aka FIXME: The above could be potentially nasty if the module
            //is using '\0' to detect end of a char*
            //It should instead use the size field
        }
        
        if (trailBits != 0) {
            for (i = v->Bytes()-1; (i >= (byteShifts)); i--) {
                c[i] = ((c[i] >> trailBits)&lowerMask) | ((c[i-1] & lowerMask) << (8 - trailBits));
            }
        }
        
    } else {//left shift
        int i;
        for (i = 0; (i < (v->Bytes() - byteShifts)); i++)
            c[i] = c[i + byteShifts];
        
        for (int j= v->Bytes()-byteShifts;j < v->Bytes();j++) {
            c[j] = 0;
            //NOTE aka FIXME: The above could be potentially nasty if the module
            //is using '\0' to detect end of a char*
            //It should instead use the size field
        }
        
        if (trailBits != 0) {
            for (i = 0; (i < (v->Bytes() - byteShifts)); i++) {
                c[i] = ((c[i] << trailBits) & upperMask) |
                        ((c[i+1] & upperMask) >> (8 - trailBits) );
            }
        }
        
    }
    return 0;
}
