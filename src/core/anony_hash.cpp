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
// Copyright Â© 2005-2006
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
//
// This file contains the defs for the hostname specific algos
// defined in anony_algos.hpp
//
//********************************************************************

#include "anony_algos.hpp"
#include "core_globals.h"
#include <openssl/sha.h>
#include <openssl/md2.h>
#include <openssl/ripemd.h>


void to_hexstr(unsigned char*, char*);
void get_int64(Value*, int64_t&);
void set_int64(Value*, int64_t&);
/*
//note: in and out are assumed non null
void to_hexstr( unsigned char*in, char *out ) {
    //if(verbose) cerr << "IN:"<<in<<endl;
    for ( int x = 0, y = 0; x < 16; x++ ) {
        uint32_t n = 0;
        //chew the lower nibble
        n = ( in[x] & 0xF0 ) >> 4;
        out[y++] = "0123456789abcdef"[n];
        if ( verbose ) fprintf( stderr, "%c", out[y-1] );
        //bite the upper nibble
        n = ( in[x] & 0x0F );
        out[y++] = "0123456789abcdef"[n];
        if ( verbose ) fprintf( stderr, "%c", out[y-1] );
        n = in[x];
        if ( verbose ) fprintf( stderr, "(%x) ", n );
    }
    if ( verbose ) cerr << endl;
}
*/

// EVP_md2(), EVP_md5(), EVP_sha(), EVP_sha1(), EVP_sha224(), EVP_sha256(),
// EVP_sha384(), EVP_sha512() and EVP_ripemd160() digest algorithms respectively.
// The associated signature algorithm is RSA in each case.
const char* const Hash::typeParam = "digestAlgorithm"; // options: MD2, MD5, SHA, SHA1, SHA224, SHA256, SHA384, SHA512, RIPEMD160
const char* const Hash::pwdParam = "passphrase";

Hash::Hash(vector<string>& param_names, vector<string>& param_values)
: AnonyAlg(param_names, param_values) {
    if (verbose) cerr << "flaim: Constructor of Hash." << endl;
    
    if ((param_names.size() > 2) || (param_names.size() != param_values.size())) {
        cerr << "flaim: Incorrect number of parameters for Hash" << endl;
        exit(1);
    }
    
    // set default value
    Hash::hashType = HASHTYPE_MD5;
    Hash::evpHashFunc = &EVP_md5;
    Hash::hashFunc = &MD5;
    Hash::pwd.clear();
    
    // @@@
    // cerr << "1111111111111111" << endl;
    
    for (int i = 0; i < param_names.size(); i++) {
        // EVP_md2(), EVP_md5(), EVP_sha(), EVP_sha1(), EVP_sha224(), EVP_sha256(),
        // EVP_sha384(), EVP_sha512() and EVP_ripemd160()
        if (param_names[i] == Hash::typeParam) {
            if (param_values[i] == "MD2") { // 128
                Hash::hashType = HASHTYPE_MD2;
                Hash::evpHashFunc = &EVP_md2;
                Hash::hashFunc = &MD2;
            }
            else if (param_values[i] == "MD5") { // 128
                Hash::hashType = HASHTYPE_MD5;
                Hash::evpHashFunc = &EVP_md5;
                Hash::hashFunc = &MD5;
            }
            else if (param_values[i] == "SHA") { // 160
                Hash::hashType = HASHTYPE_SHA;
                Hash::evpHashFunc = &EVP_sha;
                Hash::hashFunc = &SHA;
            }
            else if (param_values[i] == "SHA1") { // 160
                Hash::hashType = HASHTYPE_SHA1;
                Hash::evpHashFunc = &EVP_sha1;
                Hash::hashFunc = &SHA1;
            }
#if !defined(OPENSSL_NO_SHA256) && defined(SHA256_CBLOCK)
            else if (param_values[i] == "SHA224") {
                Hash::hashType = HASHTYPE_SHA224;
                Hash::evpHashFunc = &EVP_sha224;
                Hash::hashFunc = &SHA224;
            }
            else if (param_values[i] == "SHA256") {
                Hash::hashType = HASHTYPE_SHA256;
                Hash::evpHashFunc = &EVP_sha256;
                Hash::hashFunc = &SHA256;
            }
#endif
#if !defined(OPENSSL_NO_SHA512) && defined(SHA512_CBLOCK)
            else if (param_values[i] == "SHA384") {
                Hash::hashType = HASHTYPE_SHA384;
                Hash::evpHashFunc = &EVP_sha384;
                Hash::hashFunc = &SHA384;
            }
            else if (param_values[i] == "SHA512") {
                Hash::hashType = HASHTYPE_SHA512;
                Hash::evpHashFunc = &EVP_sha512;
                Hash::hashFunc = &SHA512;
            } 
#endif
            else if (param_values[i] == "RIPEMD160") {
                Hash::hashType = HASHTYPE_RIPEMD160;
                Hash::evpHashFunc = &EVP_ripemd160;
                Hash::hashFunc = &RIPEMD160;
            }
            else {
                cerr <<"flaim: Invalid param value for Hash" <<endl;
                exit(1);
            }
        }
        else if (param_names[i] == Hash::pwdParam) {
            Hash::pwd = param_values[i];
        }
        else {
            cerr <<"flaim: Invalid param name for Hash" <<endl;
            exit(1);
        }
    }
    
    // @@@
    // cerr << "22222" << endl;
}

int Hash::anonymizeValue(Value* v) {
    
    unsigned char* mac;
    unsigned int macLen;    
  
    // @@@
    // cerr << "3333333" << endl;
    switch (Hash::hashType) {
        case HASHTYPE_MD2: // 16
        case HASHTYPE_MD5:
            mac = new unsigned char[17];
            memset(mac, 0, 17);
            macLen = 16;            
            break;
        case HASHTYPE_SHA:
        case HASHTYPE_SHA1:
        case HASHTYPE_RIPEMD160:
            mac = new unsigned char[21];
            memset(mac, 0, 21);
            macLen = 20;
            break;
        case HASHTYPE_SHA224:
            mac = new unsigned char[29];
            memset(mac, 0, 29);
            macLen = 28;            
            break;
        case HASHTYPE_SHA256:
            mac = new unsigned char[33];
            memset(mac, 0, 33);
            macLen = 32;
            break;
        case HASHTYPE_SHA384:
            mac = new unsigned char[49];
            memset(mac, 0, 49);
            macLen = 48;
            break;
        case HASHTYPE_SHA512:
            mac = new unsigned char[65];
            memset(mac, 0, 65);
            macLen = 64;            
    }
    // @@@
    // cerr << "44444" << endl;
    switch (v->Type()) {
        case VAL_TYPE_UINT8:
        case VAL_TYPE_UINT16:
        case VAL_TYPE_UINT32:
        case VAL_TYPE_INT8:
        case VAL_TYPE_INT16:
        case VAL_TYPE_INT32: {
            int64_t num = 0;
            get_int64(v, num);
            
            if (pwd.empty()) {
                (*hashFunc)((const unsigned char*)&num, 8, mac);
            } else {
                HMAC((*evpHashFunc)(), Hash::pwd.c_str(), Hash::pwd.size(), 
                        (const unsigned char*)&num, 8, mac, &macLen);
            }

            set_int64(v, num);            
            break;
        }
        case VAL_TYPE_CHAR_PTR: {
            char* c = Value::to_char_ptr(v);

            if (pwd.empty()) {
                (*hashFunc)((const unsigned char*)c, v->Bytes(), mac);
            } else {
                HMAC((*evpHashFunc)(), Hash::pwd.c_str(), Hash::pwd.size(), 
                        (const unsigned char*)c, v->Bytes(), mac, &macLen);
            }

            memcpy(v->Ptr(), ((void*)(mac)), v->Bytes());
            break;
        }
        case VAL_TYPE_STRING: {
            string *s = Value::to_string(v);            

            if (pwd.empty()) {
                (*hashFunc)((const unsigned char*)s->c_str(), s->size(), mac);
            } else {
                HMAC((*evpHashFunc)(), Hash::pwd.c_str(), Hash::pwd.size(), 
                        (const unsigned char*)s->c_str(), s->size(), mac, &macLen);
            }
            
            int32_t tmpSize = 2 * macLen + 1;
            char* temp = new char[tmpSize];
            memset(temp, 0, tmpSize);
            to_hexstr(mac, temp);

            *s = string(temp);
            delete[] temp;
            break;
        }
        default:
            cerr << "flaim Hashmac: Unsupported data type" <<endl;
            delete[] mac;
            exit(1);
    }
    delete[] mac;
    // @@@
    // cerr << "555555" << endl;
    return 0;
}

Hash::~Hash() {
}
