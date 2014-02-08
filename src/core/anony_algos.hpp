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
// $Id: anony_algos.hpp 1042 2008-01-30 21:46:17Z byoo3 $
//
// This file contains the AnonyAlg class from which all the other
// algo impls are derived. These derived algos are also defined in
// this file. To rephrase all the anonymization algorithms and the
// class they are derived from are in this file... whew.
//
//********************************************************************


#ifndef _ANONY_ALGOS_H
#define _ANONY_ALGOS_H

#include <stdexcept>
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <ctime>
#include <list>
#include <vector>
#include <typeinfo>
#include <stdlib.h>
#include <stdint.h>

#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/md5.h>

// #include "md5.h"
#include "rijndael.h"
#include "panonymizer.h"
#include "value.h"
#include "value_types.h"
#include "record.h"
#include "flaim_core.hpp"
#include "listed_record.h"
#include "core_globals.h"
#include "time_enum.h"

using namespace std;


//class for implementing anonymization algorithms
class AnonyAlg {
public:    
    AnonyAlg(vector<string> & param_names, vector<string> & param_values) {
        if (verbose)
            cerr << "flaim: someone called AnonyAlg constructor" <<endl;
        
        secondField = "";
        lastID = 0;
        
        if (param_names.size() != param_values.size()) {
            cerr << "flaim: mismatch in num of params for AnonyAlg" <<endl;
            exit(1);
        }
        
        for (int i = 0; i < param_values.size(); i++) {
            if (param_names[i] == "secondaryField") {
                secondField = param_values[i];
            }
        }
    }    
    
    virtual ~AnonyAlg() {
        if (verbose)
            cerr << "flaim: someone called the AnonyAlg destructor for "
                    << algoName << endl;
    }
    
    char algoName[100]; //FIXME:possible buffer overflow source !!!
    
    //FIXME: Insert proper doc here explaining how exactly this function should be implemented
    virtual int anonymize(string fieldName) {
        Value val;
        Value* v = &val;
        clist_index_t maxID = coreList.front().id + coreList.size() /*- 1*/;
        if (verbose)
            fprintf(stderr, "flaim: someone called the standard common anonymize function for %s\n",
                    algoName);
        
        while (lastID != maxID) {
            //Ingore the record if the designated field is not in it
            if (0 != coreList[lastID].theRecord->getField((char*)fieldName.c_str(), val)) {
                --(coreList[lastID].refCount);
                
                ++lastID;
                return 0;
            }
            
            if (verbose) cerr << "flaim: lastID="<<lastID << "---------------------" <<endl;
            //if (verbose) val.dumpPrint("Before");
            
            //FIXME: check return value here ?
            anonymizeValue(v);
            // if (verbose) val.dumpPrint("After");
            
            //anonymize the secondary field
            if (secondField != "") {
                if(verbose)
                    cerr << secondField << endl;
                
                //Ingore the secondField if not exist
                if (0 != coreList[lastID].theRecord->getField((char*)secondField.c_str(), val)) {
                    --(coreList[lastID].refCount);
                    ++lastID;
                    return 0;
                }
                
                // if (verbose) val.dumpPrint("Before2");
                
                //FIXME: check return value here ?
                anonymizeValue(v);
                
                //if (verbose) val.dumpPrint("After2");
            }
            
            --(coreList[lastID].refCount);
            ++lastID;
        }
        
        //set flag "postRecordsBool" to true, if the refCount decreases to 0
        if (coreList[coreList.front().id].refCount == 0){
            if (verbose)
                cerr << "set postRecordsBool to true" <<endl;
            
            pthread_mutex_lock(&postRecordsAvailMutex);
            postRecordsBool = true;
            pthread_mutex_unlock(&postRecordsAvailMutex);
        }
        
        
        return 0;
    }
    
    //Actual anonymization on the data field, Return 1 to indicate error 0 for success
    virtual int anonymizeValue(Value * v) {
        fprintf(stderr, "flaim: someone called the BLANK function anonymizeValue for %s!!\n", algoName);
        exit(1);
        return 0;
    }
    
protected:
    clist_index_t lastID; //the sequence ID to indicate the first record not anonymized
    string secondField; //secondary field name
    
};


////////////////////////////////////////////////////////////////
//TSAnonyAlg
//
//
////////////////////////////////////////////////////////////////
class TSAnonyAlg : public AnonyAlg {
public:
    
    TSAnonyAlg(vector<string> & param_names, vector<string> & param_values) :
        AnonyAlg(param_names, param_values) {
            if (verbose) cerr << "flaim: Constructor of TSAnonyAlg" << endl;
            secondField = "";
            
            if (param_names.size() != param_values.size()) {
                cerr << "flaim: mismatch in num of params for TSAnonyAlg" <<endl;
                exit(1);
            }
            
            for (int i = 0; i < param_values.size(); i++) {
                if (param_names[i] == TSAnonyAlg::secondParam) {
                    secondField = param_values[i];
                }
            }
            
            lastID = 0;
        }
        
        static const char * const secondParam;
        
        virtual int anonymize(string fieldName) {
            Value val;
            Value* v = &val;
            clist_index_t maxID = coreList.front().id + coreList.size() /*- 1*/;
            if (verbose)
                cerr << "flaim: anonymize() of TSAnonyAlg" << endl;
            
            while (lastID != maxID) {
                //anonymize the primary field
                if(0 != coreList[lastID].theRecord->getField((char*)fieldName.c_str(), val)) {
                    --(coreList[lastID].refCount);
                    ++lastID;
                    return 0;
                }
                
                if (verbose) cerr << "flaim: lastID="<<lastID << " ---------------------" <<endl;
                //  if (verbose) val.dumpPrint("Before");
                
                //FIXME: check return value here ?
                anonymizeValue(v);
                
                //  if (verbose) val.dumpPrint("After");
                
                
                //anonymize the secondary field
                if (secondField != "") {
                    if (verbose)
                        cerr << secondField << endl;
                    
                    //Ingore the secondField if not exist
                    if (0 != coreList[lastID].theRecord->getField((char*)secondField.c_str(), val)) {
                        --(coreList[lastID].refCount);
                        ++lastID;
                        return 0;
                    }
                    
                    // if (verbose) val.dumpPrint("Before2");
                    
                    //FIXME: check return value here ?
                    anonymizeValue(v);
                    
                    //if (verbose) val.dumpPrint("After2");
                }
                
                --(coreList[lastID].refCount);
                ++lastID;
            }
            
            //set flag "postRecordsBool" to true, if the refCount decreases to 0
            if (coreList[coreList.front().id].refCount == 0){
                if (verbose)
                    cerr << "set postRecordsBool to true" <<endl;
                
                pthread_mutex_lock(&postRecordsAvailMutex);
                postRecordsBool = true;
                pthread_mutex_unlock(&postRecordsAvailMutex);
            }
            
            return 0;
        }
        
protected:
    string secondField; //secondary field name
};


///////////////////////////////////////
//Generic/Common Utility Functions
//decl here.. defs in anony_generic.cpp
void Tokenize(const string& str,
        vector<string>& tokens, const string& delimiters = " ");

void get_uint32(Value *v, uint32_t &i);

void set_uint32(Value *v, uint32_t &i);

void get_int64(Value *v, int64_t &i);

void set_int64(Value *v, int64_t &i);


/*
 * //class for IP Addr Prefix-preserving anonymization
 * class IpPrefixPresv:public AnonyAlg
 * {
 * public:
 * IpPrefixPresv(vector<string> & param_names, vector<string> & param_values);
 * virtual  ~IpPrefixPresv();
 *
 * virtual int anonymizeValue(Value *v);
 *
 * protected:
 * unsigned char key[32];
 * unsigned char my_key[16];
 * unsigned char my_pad[16];
 * auto_ptr<PAnonymizer>  my_anonymizer;
 * };*/


//class for IP Addr Prefix-preserving anonymization
class BinaryPrefixPresv:public AnonyAlg {
public:
    BinaryPrefixPresv(vector<string> & param_names, vector<string> & param_values);
    virtual  ~BinaryPrefixPresv();
    
    virtual int anonymizeValue(Value *v);
    
protected:
    unsigned char key[32];
    unsigned char my_key[16];
    unsigned char my_pad[16];
    auto_ptr<PAnonymizer>  my_anonymizer;
};

//class to timestamp unit truncation
class TimeUnitAnnihilation:public TSAnonyAlg {
public:
    TimeUnitAnnihilation(vector<string> & param_names, vector<string> & param_values);
    virtual  ~TimeUnitAnnihilation();
    
    //virtual int anonymize(string fieldName);
    virtual int anonymizeValue(Value *v);
    
    //param name def
    static const char * const timeFieldParam;
    
protected:
    enum timeField {T_INVALID=-1, T_SECS=0, T_MINS, T_HRS, T_DAYS, T_MONTHS, T_YRS};
    timeField trunc[6]; //the time units to be zeroed out: year, month,day,hour,minute,second
};

//class for random shift of timestamp
class RandTimeShift:public TSAnonyAlg {
public:
    RandTimeShift(vector<string> & param_names, vector<string> & param_values);
    virtual  ~RandTimeShift();
    
    virtual int anonymizeValue(Value *v);
    
    //param name def
    static const char * const tShift_lowerParam;
    static const char * const tShift_upperParam;
    
protected:
    time_t lowTimeShiftLimit; //lower limit for random time shift (in seconds)
    time_t upTimeShiftLimit;  //upper limit for random time shift (in seconds)
    time_t timeShift;  //random time shift;
    FILE *fp;
};


//class timestamp enumeration
class TimeEnumerate:public TSAnonyAlg {
public:
    TimeEnumerate(vector<string> & param_names, vector<string> & param_values);
    virtual  ~TimeEnumerate();
    
    virtual int anonymize(string fieldName); //different from other anony algo
    
    static const char* const buffer_sizeParam;
    static const char* const interval_lengthParam;
    static const char* const baseTimeParam;
    static const uint32_t default_buffer_size;
    
protected:
    uint32_t bufferSize; //buffer size for enumeration
    uint32_t timeInterval; //in seconds - hard to be careful to avoid overflow
    uint32_t randEpoch; //the randomly generated start time
    uint32_t lastSize;
    clist_index_t lastBack;  // What is this for?
    BufferList* buff_list;
    
private:
    virtual int fillBuffer(BufferList*, clist_index_t*);
    int numTimesRanCount;
    clist_index_t lastReadID; // The id number of the record that should be read next.
    
};


//class for MAC Adrr random permutation
class MacPermute:public AnonyAlg {
public:
    MacPermute(vector<string> & param_names, vector<string> & param_values);
    virtual  ~MacPermute();
    
    virtual int anonymizeValue(Value *v);
    
protected:
    uint64_t macKey, macEntry, mapCount;
    typedef pair <uint64_t, uint64_t> intPair;
    map<uint64_t, uint64_t>  macMapTable;
    map<uint64_t, uint64_t>  macEntryTable;
    FILE *fp;
};


//Generalized Numeric truncation
class NumericTruncate:public AnonyAlg {
public:
    NumericTruncate(vector<string> & param_names, vector<string> & param_values);
    virtual  ~NumericTruncate();
    
    virtual int anonymizeValue(Value *v);
    
    static const char* const numBitsParam;
    static const char* const radixParam;
    static const char* const directionParam;
    
protected:
    int shiftNum;
    int radix;
    bool lsbFirst;
};


//Generalized String truncation
class StringTruncate:public AnonyAlg {
public:
    StringTruncate(vector<string> & param_names, vector<string> & param_values);
    virtual  ~StringTruncate();
    
    virtual int anonymizeValue(Value *v);
    
    static const char* const numCharsParam;
    static const char* const directionParam;
    
protected:
    int shiftNum;
    bool lsbFirst;
};


//Generalized Byte truncation
class BytesTruncate:public AnonyAlg {
public:
    BytesTruncate(vector<string> & param_names, vector<string> & param_values);
    virtual  ~BytesTruncate();
    
    virtual int anonymizeValue(Value *v);
    
    static const char* const numBitsParam;
    static const char* const directionParam;
    
protected:
    int shiftNum;
    bool lsbFirst;
};

/*
 * //Generalized random permutation
 * class RandPermute:public AnonyAlg
 * {
 * public:
 * RandPermute(vector<string> & param_names, vector<string> & param_values);
 * virtual  ~RandPermute();
 * virtual int anonymizeValue(Value *v);
 *
 * //param name def
 * static const char * const secFieldName;
 *
 * protected:
 * unsigned int permuteKey, permuteEntry, mapCount;
 * typedef pair <unsigned int, unsigned int> intPair;
 * map<unsigned int, unsigned int> mapTable;
 * map<unsigned int, unsigned int> entryTable;
 * FILE *fp;
 * };
 */

//Generalized random permutation
class BinaryRandPermute:public AnonyAlg {
public:
    BinaryRandPermute(vector<string> & param_names, vector<string> & param_values);
    virtual  ~BinaryRandPermute();
    virtual int anonymizeValue(Value *v);
    
protected:
    unsigned int intPermuteKey, intPermuteEntry, mapCount;
    string strPermuteKey, strPermuteEntry;
    typedef pair <unsigned int, unsigned int> intPair;
    typedef pair <string, string> strPair;
    typedef pair <string, unsigned int> mixPair;
    map<unsigned int, unsigned int> intMapTable;
    map<unsigned int, unsigned int> intEntryTable;
    map<string, string> strMapTable;
    map<string, unsigned int> strEntryTable;
    FILE *fp;

    //param name def
    string secondaryFieldName;
};


//Generalized classification
class Classify:public AnonyAlg {
public:
    Classify(vector<string> & param_names, vector<string> & param_values);
    virtual  ~Classify();
    
    virtual int anonymizeValue(Value *v);
    
    static const char* const cfgStrParam;
    
protected:
    vector < pair <int64_t, int64_t> > binSpec;
};

//Generalized BinaryBlackmarker
class BinaryBlackMarker:public AnonyAlg {
public:
    BinaryBlackMarker(vector<string> & param_names, vector<string> & param_values);
    virtual  ~BinaryBlackMarker();
    
    virtual int anonymizeValue(Value *v);
    
    static const char* const numMarksParam;
    static const char* const replacementParam;
    
protected:
    unsigned int numMarks;
    uint32_t replacement;
};

//Generalized StringBlackmarker
class StringBlackMarker:public AnonyAlg {
public:
    StringBlackMarker(vector<string> & param_names, vector<string> & param_values);
    virtual  ~StringBlackMarker();
    
    virtual int anonymizeValue(Value *v);
    
    static const char* const numMarksParam;
    static const char* const replacementParam;
    
protected:
    unsigned int numMarks;
    string replacementStr;
};

//Generalized Annihilation: remove the unanonymized data field
class Annihilation:public AnonyAlg {
public:
    Annihilation(vector<string> & param_names, vector<string> & param_values);
    virtual  ~Annihilation();
    
    virtual int anonymizeValue(Value *v);
    
private:
    //uint32_t replaceNum;
    //char replaceChar;
    
};

//Hostname specific BlackMarker
class HostBlackMarker:public AnonyAlg {
public:
    HostBlackMarker(vector<string> & param_names, vector<string> & param_values);
    virtual  ~HostBlackMarker();
    
    virtual int anonymizeValue(Value *v);
    
    static const char* const typeParam;
    static const char* const hostAltParam;
    static const char* const domainAltParam;
    
private:
    bool isFullBlackMarker;
    string hostAlt;
    string domainAlt;
};

//Full Hostname Hashing - this needs to be figured out
class HostHash:public AnonyAlg {
public:
    HostHash(vector<string> & param_names, vector<string> & param_values);
    virtual  ~HostHash();
    
    virtual int anonymizeValue(Value *v);
    
    static const char* const typeParam;
    
private:
    bool isMD5Digest;
};

class Hash:public AnonyAlg {
public:
    Hash(vector<string> & param_names, vector<string> & param_values);
    virtual  ~Hash();
    virtual int anonymizeValue(Value *v);
    static const char* const typeParam;
    static const char* const pwdParam;
private:
    int hashType;
    string pwd;
    // HMAC_CTX ctx;
    const EVP_MD* (*evpHashFunc)(void);
    unsigned char* (*hashFunc)(const unsigned char*, size_t, unsigned char*);
    
    static const int HASHTYPE_MD2 = 0;
    static const int HASHTYPE_MD5 = 1;
    static const int HASHTYPE_SHA = 2;
    static const int HASHTYPE_SHA1 = 3;
    static const int HASHTYPE_SHA224 = 4;
    static const int HASHTYPE_SHA256 = 5;
    static const int HASHTYPE_SHA384 = 6;    
    static const int HASHTYPE_SHA512 = 7;    
    static const int HASHTYPE_RIPEMD160 = 8;
};

//Generalized Annihilation: remove the unanonymized data field
class Substitute:public AnonyAlg {
public:
    Substitute(vector<string> & param_names, vector<string> & param_values);
    virtual  ~Substitute();
    
    virtual int anonymizeValue(Value *v);
    
    static const char* const subsParam;
    
private:
    uint32_t subsNum;
    string subsStr;
};

// proof-of-concept: to be used in this branch only!!
/*
 * class ProofOfConcept : public AnonyAlg
 * {
 * public:
 * ProofOfConcept(vector<string> & param_names, vector<string> &
 * param_values);
 * virtual ~ProofOfConcept();
 * virtual int anonymize(string fieldName);
 * private:
 * std::list<listedRecord>::iterator iter;
 * };
 */

#endif
