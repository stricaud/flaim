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
// Copyright (c) 2005-2006
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
// $Id: nfdump.cpp 273 2006-08-04 21:35:42Z gcolombo $
//
// Module which provides I/O functionality for nfdump-format logs
//
//********************************************************************

#include "record.h"
#include "map_record.h"
#include "value.h"
#include "value_types.h"
#include "nfdump.h"
#include "nfd_structs.h"
#include <stdio.h>

#include <iostream>
#include <fstream>
#include <string>
#include <deque>
#include <math.h>
#include <time.h>

////////////////////////////
// tmp for test. byoo3

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef WORDS_BIGENDIAN
#	define ntohll(n)	(n)
#	define htonll(n)	(n)
#else
#	define ntohll(n)	(((uint64_t)ntohl(n)) << 32) + ntohl((n) >> 32)
#	define htonll(n)	(((uint64_t)htonl(n)) << 32) + htonl((n) >> 32)
#endif
////////////////////////////

#define BUFFSIZE 1048576
#define MAX_BUFFER_SIZE 104857600

using namespace std;

static string errfn = "libnfdump_flaim.errors";
static const string schemafile = "nfdump_schema.xml";

static uint32_t invLong(uint32_t x);
static uint16_t invShort(uint16_t x);
static void print_record(void *record, char *s);

// ********************************************
// PUBLIC NfdumpParser implementation functions
// ********************************************

// NfdumpParser(string, string, string)
// Constructor
//
// Parameters: in, the name of the file to open for reading; out, the name of
// the file to open for writing; dummy, unused for now but may be used later
// to pass module configuration
// Returns: Nothing
// Modifies: Nothing
// Notes: Opens input/output streams
NfdumpParser::NfdumpParser(string in, string out, string err) : curSeq(0),
        getRcdsToGo(0), putRcdsToGo(0), getBlockRcdsToGo(0), putBlockRcdsToGo(0),
        getRcdVersion(0), putRcdVersion(0), invert(false) {
        
    if ("" == in) {
        cerr << "*** nfdump module: This module does not support "
                << "streaming input" << endl;
        exit(2);
    }
    if ("" == out) {
        cerr << "*** nfdump module: This module does not support "
                << "streaming output" << endl;
        exit(2);
    }
    /*
        char curCTime[20];
        sprintf(curCTime, ".%d", time (NULL));
        errfn.append(curCTime);
     */
    infile.open(in.c_str(), ifstream::binary | ifstream::in);
    outfile.open(out.c_str(), ofstream::binary | ofstream::out);
    errfile.open(err.c_str(), ofstream::binary | ofstream::out);
    
    bool screwedUp = false;
    if (infile.fail()) {
        cerr << "*** nfdump module: failed to open input file" << endl;
        screwedUp = true;
    }
    if (outfile.fail()) {
        cerr << "*** nfdump module: failed to open output file" << endl;
        screwedUp = true;
    }
    if (errfile.fail()) {
        cerr << "*** nfdump module: failed to open error file" << endl;
        screwedUp = true;
    }
    
    if (screwedUp) exit(2);
    
    infile.exceptions(ifstream::failbit |ifstream::badbit);
    // outfile.exceptions(ifstream::failbit |ifstream::badbit);
    
    cerr << "nfdump module: input from [" << in << "], "
            << "output to [" << out << "] " << endl
            << "erroneous data will be written to " << err << endl;
}

// ~NfdumpParser()
// Destructor
//
// WHAT dynamic memory?
NfdumpParser::~NfdumpParser() {
    outfile.flush();
}

// int getModuleSchemaFile(string)
// return, by reference, the module schema's filename
int NfdumpParser::getModuleSchemaFile(string & schemaFile) {
    schemaFile = schemafile; // schemafile was defined as a const string above
    return 0;
}

// int getRecordSeq
// return, by reference, the number of this record
int NfdumpParser::getRecordSeq(int & rcdSeq) {
    rcdSeq = curSeq;
    return 0;
}

// int resetFilePointer
// move back to the start of the file
int NfdumpParser::resetFilePointer() {
    curSeq = 0;
    infile.seekg(0, ios::beg);
    // getNextHeader();
    return 0;
}

// int isEndofLog()
// Test to see if the end of the current log has been reached
//
// Parameters: None
// Returns: 1 if there are no more records in the log, 0 otherwise
// Modifies: Nothing
int NfdumpParser::isEndofLog() {
    bool eof = true;
    if (getBlockRcdsToGo > 0) return 0;
    
    try {
        infile.peek();
        eof = infile.eof();
        infile.clear();
        if (!eof) return 0;
    }
    catch (exception& ex) {}
        
    // catch (ifstream::failure& ex) {}
#ifdef DEBUG
cerr << "nfdump module: v5 queue size is "
        << v5HeaderQueue.size() << endl;
cerr << "nfdump module: v7 queue size is "
        << v7HeaderQueue.size() << endl;
cerr << "nfdump module: stream position is " << tmp << endl;
#endif

    return 1;
}

// Record * getRecord()
// Create a new record and fill it in
Record* NfdumpParser::getRecord() {
    Record* r = new MapRecord();
    
    try {
        if (0 == getRcdsToGo && !isEndofLog() && 0 == getBlockRcdsToGo)  //get the record header
        {
            // cerr << "@@@1" << endl;
            if (0 != getNextHeader(r)) {
                cerr << "*** nfdump module: Error getting next flow header "
                        << "in getRecord(record number: " << curSeq << ")" << endl;
                return NULL;
            }
        }
        else if (9 == getRcdVersion && 0 == getBlockRcdsToGo && !isEndofLog()) {
            // cerr << "@@@2" << endl;
            getNextBlockHeader(r);
        }
        else  //get the record body
        {
            // cerr << "@@@3" << endl;
            switch (getRcdVersion) {
                case 5:
                    getVersion5(r);
                    break;
                case 7:
                    getVersion7(r);
                    break;
                case 9:
                    getVersion9(r);
                    break;
                default:
                    cerr << "*** nfdump module: fatal error: record version type "
                            << getRcdVersion << " is not supported" << endl;
                    return NULL;
            }
        }
    }
    catch (ifstream::failure& ex) {
        cerr << "*** nfdump module:" << curSeq <<":I/O error in reading header: "
                << ex.what() << endl;
        return NULL;
    }
    catch (exception& ex) {
        cerr << "*** nfdump module:" << curSeq <<":Fatal error in reading header: "
                << ex.what() << endl;
        return NULL;
    }
    
    ++curSeq;
    
    return r;
}

// int putRecord(Record *)
// unpack and delete the contents of a record, then free the record
int NfdumpParser::putRecord(Record * r) {
    Value v;
    
    // if (0 == putRcdsToGo && 0 == putBlockRcdsToGo)  //put the record header
    if (0 == r->getField("SYS_HEAD_TYPE", v)) {
        /*
         * if (0 != r->getField("SYS_HEAD_TYPE", v))
         * {
         * // r->printFieldName();
         * cerr << "*** nfdump module: Fatal error: Did not find head type field " << endl;
         * exit(1);
         * }
         */
        putHeader(r);
    }
    else if (0 == r->getField("BLOCK_HEAD_HOLDER", v)) {
        putBlockHeader(r);
    }
    else  //put the record body
    {
        if (0 != r->getField("SYS_RCD_TYPE", v)) {
            r->printFieldName();
            cerr << "*** nfdump module: Fatal error: Did not find record type field " << endl;
            exit(1);
        }
        
        if (*(uint8_t*)(v.Ptr()) != putRcdVersion) {
            cerr << "*** nfdump module: Fatal error: Record type does not match "
                    << "current put type" << endl;
            exit(1);
        }
        int verNum = *(uint8_t*)(v.Ptr());
        delete (uint8_t*)(v.Ptr());
        
        // switch off of the record type
        switch(verNum) {
            case 5:                
                nfv5_rcd_t* tmp5;
                r->getField("SYS_RCD_HOLDER", v);
                tmp5 = (nfv5_rcd_t*)(v.Ptr());
                if (invert) invertV5Record(tmp5);
                
                outfile.write((char*)(tmp5), sizeof(nfv5_rcd_t));
                delete tmp5;
                
                //decrement the record counter
                --putRcdsToGo;
                
                break;
                
            case 7:                
                nfv7_rcd_t* tmp7;
                r->getField("SYS_RCD_HOLDER", v);
                tmp7 = (nfv7_rcd_t*)(v.Ptr());
                if (invert) invertV7Record((nfv7_rcd_t*)(v.Ptr()));
                
                outfile.write((char*)(tmp7), sizeof(nfv7_rcd_t));
                delete tmp7;
                
                //decrement the record counter
                --putRcdsToGo;
                
                break;
                
            case 9:                
                master_record_t* tmp9;
                
                r->getField("SYS_RCD_HOLDER", v);
                tmp9 = (master_record_t*)(v.Ptr());
                
                WriteRecordOutput(tmp9);
                // if (invert) invertV9Record((master_record_t*)(v.Ptr()));
                
                // @@@
                // cerr << "put record address: " << tmp9 << endl;
                // cerr << "put sizeof(master_record_t)" << sizeof(master_record_t) << endl;
                // outfile.write((char*)(tmp9), sizeof(master_record_t));
                // outfile.write((char*)(tmp9), sizeof(master_record_t));
                
                delete tmp9;
                //decrement the record counter
                // --putBlockRcdsToGo;                                
                
                if (--putBlockRcdsToGo == 0 && putRcdsToGo == 0) {                                              
                    infile.seekg(0, ios::end);                        
                    int32_t diff = infile.tellg() - outfile.tellp();
                    if (diff <= 0) break;

                    char* tmpBuf = new char[diff];
                    int32_t iPos = (int32_t)infile.tellg() - diff;
                    infile.seekg(iPos, ios::beg);
                    infile.read(tmpBuf, diff);                        

                    // cerr << "@@@4 gcount: " << infile.gcount();                        
                    outfile.write(tmpBuf, diff);                        
                    delete[] tmpBuf;                    
                }
                
                break;
                
            default:                
                cerr << "*** nfdump module: Fatal error: Record type invalid"<< endl;
                exit(1);
        }
    }
    
    delete r;
    return 0;
}

// *********************************************
// PRIVATE NfdumpParser implementation functions
// *********************************************

// int getVersion5(Record * r);
// read a netflow v5 record and pack it
int NfdumpParser::getVersion5(Record * r) throw (exception&, ifstream::failure&) {
    nfv5_rcd_t* x = new nfv5_rcd_t;
    
    infile.read((char*)x, sizeof(nfv5_rcd_t));
    
    if (invert) invertV5Record(x);
    
    r->setField("SRC_IP", Value(&x->srcaddr, VAL_TYPE_UINT32, 4));
    r->setField("DST_IP", Value(&x->dstaddr, VAL_TYPE_UINT32, 4));
    r->setField("NEXT_HOP", Value(&x->nexthop, VAL_TYPE_UINT32, 4));
    r->setField("INPUT", Value(&x->input, VAL_TYPE_UINT16, 2));
    r->setField("OUTPUT", Value(&x->output, VAL_TYPE_UINT16, 2));
    r->setField("PACKETS", Value(&x->dpkts, VAL_TYPE_UINT32, 4));
    r->setField("OCTETS", Value(&x->doctets, VAL_TYPE_UINT32, 4));
    r->setField("TS_SEC_FIRST", Value(&x->first, VAL_TYPE_UINT32, 4));
    r->setField("TS_SEC_LAST", Value(&x->last, VAL_TYPE_UINT32, 4));
    r->setField("SRC_PORT", Value(&x->srcport, VAL_TYPE_UINT16, 2));
    r->setField("DST_PORT", Value(&x->dstport, VAL_TYPE_UINT16, 2));
    r->setField("TCP_FLAGS", Value(&x->tcp_flags, VAL_TYPE_UINT8, 1));
    r->setField("PROTOCOL", Value(&x->prot, VAL_TYPE_UINT8, 1));
    r->setField("TOS", Value(&x->tos, VAL_TYPE_UINT8, 1));
    r->setField("SRC_AS", Value(&x->src_as, VAL_TYPE_UINT16, 2));
    r->setField("DST_AS", Value(&x->dst_as, VAL_TYPE_UINT16, 2));
    r->setField("SRC_MASK", Value(&x->src_mask, VAL_TYPE_UINT8, 1));
    r->setField("DST_MASK", Value(&x->dst_mask, VAL_TYPE_UINT8, 1));
    
    uint8_t *type = new uint8_t(5); //version 5
    r->setField("SYS_RCD_TYPE", Value(type, VAL_TYPE_UINT8, 1));
    
    r->setField("SYS_RCD_HOLDER", Value(x, VAL_TYPE_CHAR_PTR,
            sizeof(nfv5_rcd_t)));
    
    //decrement the record counter
    --getRcdsToGo;
    
    return 0;
}

// int getVersion7(Record * r);
// read a netflow v7 record and pack it
int NfdumpParser::getVersion7(Record * r) throw (exception&, ifstream::failure&) {
    nfv7_rcd_t* x = new nfv7_rcd_t;
    
    infile.read((char*)x, sizeof(nfv7_rcd_t));
    
    if (invert) invertV7Record(x);
    
    r->setField("SRC_IP", Value(&x->srcaddr, VAL_TYPE_UINT32, 4));
    r->setField("DST_IP", Value(&x->dstaddr, VAL_TYPE_UINT32, 4));
    r->setField("NEXT_HOP", Value(&x->nexthop, VAL_TYPE_UINT32, 4));
    r->setField("INPUT", Value(&x->input, VAL_TYPE_UINT16, 2));
    r->setField("OUTPUT", Value(&x->output, VAL_TYPE_UINT16, 2));
    r->setField("PACKETS", Value(&x->dpkts, VAL_TYPE_UINT32, 4));
    r->setField("OCTETS", Value(&x->doctets, VAL_TYPE_UINT32, 4));
    r->setField("TS_SEC_FIRST", Value(&x->first, VAL_TYPE_UINT32, 4));
    r->setField("TS_SEC_LAST", Value(&x->last, VAL_TYPE_UINT32, 4));
    r->setField("SRC_PORT", Value(&x->srcport, VAL_TYPE_UINT16, 2));
    r->setField("DST_PORT", Value(&x->dstport, VAL_TYPE_UINT16, 2));
    r->setField("FLAGS1", Value(&x->flags1, VAL_TYPE_UINT8, 1));
    r->setField("TCP_FLAGS", Value(&x->tcp_flags, VAL_TYPE_UINT8, 1));
    r->setField("PROTOCOL", Value(&x->prot, VAL_TYPE_UINT8, 1));
    r->setField("TOS", Value(&x->tos, VAL_TYPE_UINT8, 1));
    r->setField("SRC_AS", Value(&x->src_as, VAL_TYPE_UINT16, 2));
    r->setField("DST_AS", Value(&x->dst_as, VAL_TYPE_UINT16, 2));
    r->setField("SRC_MASK", Value(&x->src_mask, VAL_TYPE_UINT8, 1));
    r->setField("DST_MASK", Value(&x->dst_mask, VAL_TYPE_UINT8, 1));
    r->setField("FLAGS2", Value(&x->flags2, VAL_TYPE_UINT16, 2));
    r->setField("ROUTER", Value(&x->router_sc, VAL_TYPE_UINT32, 4));
    
    uint8_t *type = new uint8_t(7); //version 7
    r->setField("SYS_RCD_TYPE", Value(type, VAL_TYPE_UINT8, 1));
    
    //store the record pointer
    r->setField("SYS_RCD_HOLDER", Value(x, VAL_TYPE_CHAR_PTR, sizeof(nfv7_rcd_t)));
    
    //decrement the record counter
    --getRcdsToGo;
    
    return 0;
}

// int getVersion9(Record * r);
// read a netflow v9 record and pack it
int NfdumpParser::getVersion9(Record * r) throw (exception&, ifstream::failure&) {
    master_record_t* master_record = new master_record_t;
    ExpandRecord(flow_record, master_record);
    
    char string[1024];
    print_record(master_record, string);
    // @@@
    // printf("%s\n", string);
    // cerr << "get record address: " << master_record << endl;
    // sleep(1);
    // cerr << "6" << endl;
    // cerr << "get record address: " << master_record << endl;
    // cerr << "get sizeof(master_record_t)" << sizeof(master_record_t) << endl;
    
    r->setField("FLAGS", Value(&master_record->flags, VAL_TYPE_INT32, 4));
    r->setField("SIZE", Value(&master_record->size, VAL_TYPE_INT16, 2));
    r->setField("MARK", Value(&master_record->mark, VAL_TYPE_INT16, 2));
    r->setField("MSEC_FIRST", Value(&master_record->msec_first, VAL_TYPE_UINT16, 2));
    r->setField("MSEC_LAST", Value(&master_record->msec_last, VAL_TYPE_UINT16, 2));
    r->setField("TS_SEC_FIRST", Value(&master_record->first, VAL_TYPE_UINT32, 4));
    r->setField("TS_SEC_LAST", Value(&master_record->last, VAL_TYPE_UINT32, 4));
    r->setField("DIR", Value(&master_record->dir, VAL_TYPE_INT8, 1));
    r->setField("TCP_FLAGS", Value(&master_record->tcp_flags, VAL_TYPE_INT8, 1));
    r->setField("PROT", Value(&master_record->prot, VAL_TYPE_INT8, 1));
    r->setField("TOS", Value(&master_record->tos, VAL_TYPE_INT8, 1));
    r->setField("INPUT", Value(&master_record->input, VAL_TYPE_INT16, 2));
    r->setField("OUTPUT", Value(&master_record->output, VAL_TYPE_INT16, 2));
    r->setField("SRC_PORT", Value(&master_record->srcport, VAL_TYPE_INT16, 2));
    r->setField("DST_PORT", Value(&master_record->dstport, VAL_TYPE_INT16, 2));
    r->setField("SRC_AS", Value(&master_record->srcas, VAL_TYPE_INT16, 2));
    r->setField("DST_AS", Value(&master_record->dstas, VAL_TYPE_INT16, 2));
    r->setField("FILL", Value(&master_record->fill, VAL_TYPE_INT32, 4));
    
    if ((master_record->flags & FLAG_IPV6_ADDR) != 0) {
        r->setField("SRC_IP", Value(&master_record->ip_union._v6.srcaddr, VAL_TYPE_CHAR_PTR, 16));
        r->setField("DST_IP", Value(&master_record->ip_union._v6.dstaddr, VAL_TYPE_CHAR_PTR, 16));
    }
    else {
        r->setField("SRC_IP", Value(&master_record->ip_union._v4.srcaddr, VAL_TYPE_UINT32, 4));
        r->setField("DST_IP", Value(&master_record->ip_union._v4.dstaddr, VAL_TYPE_UINT32, 4));
    }
    
    if ((master_record->flags & FLAG_PKG_64 ) != 0) {
        r->setField("OCTETS", Value(&master_record->dOctets, VAL_TYPE_CHAR_PTR, 8));
        r->setField("PACKETS", Value(&master_record->dPkts, VAL_TYPE_CHAR_PTR, 8));
    }
    else {
        r->setField("OCTETS", Value(&master_record->dOctets, VAL_TYPE_UINT32, 4));
        r->setField("PACKETS", Value(&master_record->dPkts, VAL_TYPE_UINT32, 4));
    }
    
    uint8_t *type = new uint8_t(9); //version 9
    r->setField("SYS_RCD_TYPE", Value(type, VAL_TYPE_UINT8, 1));
    
    //store the record pointer
    r->setField("SYS_RCD_HOLDER", Value(master_record, VAL_TYPE_CHAR_PTR, sizeof(master_record_t)));
    
    // Advance pointer by number of bytes for netflow record
    flow_record = (common_record_t *)((pointer_addr_t)flow_record + flow_record->size);
    
    if (--getBlockRcdsToGo == 0) free((void *)in_buff);
    
    return 0;
}

/*
 * Expand file record into master record for further processing
 * LP64 CPUs need special 32bit operations as it is not guarateed that 64bit
 * values are aligned
 */
inline void ExpandRecord(common_record_t* input_record, master_record_t* output_record ) {
    uint32_t *u;
    size_t size;
    void* p = (void*)input_record;
    
    // Copy common data block
    size = sizeof(common_record_t) - sizeof(uint8_t[4]);
    memcpy((void *)output_record, p, size);
    p = (void *)input_record->data;
    output_record->fill = 0;
    
    if ((input_record->flags & FLAG_IPV6_ADDR) != 0) {
        // IPv6
        memcpy((void *)output_record->v6.srcaddr, p, sizeof(ipv6_block_t));
        p = (void *)((pointer_addr_t)p + sizeof(ipv6_block_t));
    }
    else {
        // IPv4
        u = (uint32_t *)p;
        output_record->v6.srcaddr[0] = 0;
        output_record->v6.srcaddr[1] = 0;
        output_record->v4.srcaddr = u[0];
        output_record->v6.dstaddr[0] = 0;
        output_record->v6.dstaddr[1] = 0;
        output_record->v4.dstaddr = u[1];
        p = (void*)((pointer_addr_t)p + 2 * sizeof(uint32_t));
    }
// cerr << "2" << endl;
    // packet counter
    if ((input_record->flags & FLAG_PKG_64 ) != 0) {
        // 64bit packet counter
        value64_t l, *v = (value64_t *)p;
        l.val.val32[0] = v->val.val32[0];
        l.val.val32[1] = v->val.val32[1];
        output_record->dPkts = l.val.val64;
        p = (void *)((pointer_addr_t)p + sizeof(uint64_t));
    }
    else {
        // 32bit packet counter
        output_record->dPkts = *((uint32_t *)p);
        p = (void *)((pointer_addr_t)p + sizeof(uint32_t));
    }
// cerr << "3" << endl;
    // byte counter
    if ((input_record->flags & FLAG_BYTES_64) != 0 ) {
        // 64bit byte counter
        value64_t l, *v = (value64_t *)p;
        l.val.val32[0] = v->val.val32[0];
        l.val.val32[1] = v->val.val32[1];
        output_record->dOctets = l.val.val64;
        p = (void *)((pointer_addr_t)p + sizeof(uint64_t));
    }
    else {
        // 32bit bytes counter
        output_record->dOctets = *((uint32_t *)p);
        p = (void *)((pointer_addr_t)p + sizeof(uint32_t));
    }
}

int NfdumpParser::getNextBlockHeader(Record *r) throw (exception&, ifstream::failure&) {
    data_block_header_t* in_flow_header = new data_block_header_t;
    uint32_t buffer_size;
    int i, ret, gcount;
    
    // allocate buffer suitable for netflow version
    buffer_size = BUFFSIZE;
    in_buff = (common_record_t *)malloc(buffer_size);
    
    if (!in_buff) {
        cerr << "*** nfdump module: fatal error: Memory allocation error" << endl;
        return 1;
    }
    
    infile.read((char*)in_flow_header, sizeof(data_block_header_t));
    
    if (in_flow_header->id != DATA_BLOCK_TYPE_1) {
        cerr << "*** nfdump module: can't process block type " << in_flow_header->id << endl;
        return 1;
    }
    
    getBlockRcdsToGo = in_flow_header->NumBlocks;
    
    // @@@
    // cerr << "Number of read records: " << getBlockRcdsToGo << endl;
    
    if (in_flow_header->size > buffer_size) {
        void *tmp;
        // Actually, this should never happen, but catch it anyway
        if (in_flow_header->size > MAX_BUFFER_SIZE) {
            // this is most likely corrupt
            cerr << "*** nfdump module: Corrupt data file: Requested buffer size "
                    << in_flow_header->size << " exceeds max. buffer size." << endl;
            return 1;
        }
        
        // make it at least the requested size
        buffer_size = in_flow_header->size;
        tmp = realloc((void *)in_buff, buffer_size);
        if (!tmp) {
            cerr << "*** nfdump module: Can't reallocate buffer to " << buffer_size << " bytes " << endl;
            return 1;
        }
        in_buff = (common_record_t *)tmp;
    }
    
    try {
        infile.read((char *)in_buff, in_flow_header->size);
        gcount = infile.gcount();
        infile.peek();
        if (infile.eof()) {
            infile.clear();
        }
    }
    catch (exception& ex) {
        cerr << "*** nfdump module: Error reading data" << endl;
        return 1;
    }
    
    // @@@
    // cerr << "in_flow_header->size: " << in_flow_header->size << endl;
    // cerr << "infile.gcount(): " << gcount << endl;
    // cerr << "get BlockHeader: " << in_flow_header << endl;
    
    if (gcount == 0) {
        // break;
        return 0;
    }
    
    
    if (in_flow_header->size != gcount) {
        // this is a short read - most likely reading from the stdin pipe
        // loop until we have requested size
        size_t  request_size, total_size;
        void *read_ptr;
        
        total_size = gcount;
        request_size = in_flow_header->size - total_size;
        read_ptr = (void *)((pointer_addr_t)in_buff + total_size);
        do {
            try {
                infile.read((char*)read_ptr, request_size);
                gcount = infile.gcount();
                infile.peek();
                if (infile.eof()) {
                    infile.clear();
                }
            }
            catch (exception& ex) {
                cerr << "*** nfdump module: Error reading data" << endl;
                return 1;
            }
            
            if (gcount == 0) {
                break;
            }
            
            total_size += gcount;
            if (total_size < in_flow_header->size) {
                request_size = in_flow_header->size - gcount;
                read_ptr = (void *)((pointer_addr_t)in_buff + total_size);
                request_size = in_flow_header->size - total_size;
            }
        } while (gcount > 0 && ( total_size < in_flow_header->size));
        
        if (total_size != in_flow_header->size) {
            // still unsuccessful
            cerr << "*** nfdump module: Error reading data; Short read for netflow records: Expected "
                    << in_flow_header->size << ", got " <<  (unsigned long)total_size << endl;
            return 1;
        }
        else {
            // continue
            ret = in_flow_header->size;
        }
    }
    
    flow_record = in_buff;
    
    //store the block header pointer
    r->setField("BLOCK_HEAD_HOLDER", Value(in_flow_header, VAL_TYPE_CHAR_PTR, sizeof(data_block_header_t)));
    
    //decrement the record counter
    --getRcdsToGo;
    
}

// int getNextHeader(Record *r)
// Get the next header available in the file
//
// parameters: none
// returns: 0 on success, 1 on failure
// modifies: reads data from the input file and adds to the header queue
int NfdumpParser::getNextHeader(Record *r) throw (exception&, ifstream::failure&) {
    uint16_t version;
    infile.read((char*)(&version), sizeof(uint16_t));
    
    // if the endianness is backwards, invert everything
    switch(version) {
        case 5:
        case 7:
            getRcdVersion = version;
            break;
        case 0xA50C: // version 9: nfdump 1.5
            getRcdVersion = 9;
            break;
        case 0x0500:
            invert = true;
            getRcdVersion = 5; //set the record version for getting record body
            break;
        case 0x0700:
            invert = true;
            getRcdVersion = 7; //set the record version for getting record body
            break;
        case 0x0CA5:
            invert = true;
            getRcdVersion = 9;
            break;
        default:
            cerr << "*** nfdump module: fatal error: record version type "
                    << version << " is not supported" << endl;
            return 1;
    }
    
    // cerr << "Version number: "<< version << " , Record number: " << curSeq << " RcdVersion: " << getRcdVersion << endl;
    
    if (5 == getRcdVersion) {
        nfv5_hdr_t* hdr = new nfv5_hdr_t;
        infile.read(2 + (char*)(hdr), sizeof(nfv5_hdr_t) - 2); //skip the version field
        hdr->version = version;
        
        if (invert) invertV5Header(hdr);
        
        getRcdsToGo = hdr->count;
        
        //fill in the record fields
        r->setField("NFDUMP_VERSION", Value(&hdr->version, VAL_TYPE_UINT16, 2));
        r->setField("FLOW_COUNT", Value(&hdr->count, VAL_TYPE_UINT16, 2));
        r->setField("HEAD_SYS_UPTIME", Value(&hdr->uptime, VAL_TYPE_UINT32, 4));
        r->setField("HEAD_UNIX_SECS", Value(&hdr->unix_secs, VAL_TYPE_UINT32, 4));
        r->setField("HEAD_UNIX_NSECS", Value(&hdr->unix_nsecs, VAL_TYPE_UINT32, 4));
        r->setField("FLOW_SEQ", Value(&hdr->seq, VAL_TYPE_UINT32, 4));
        r->setField("ENGINE_TYPE", Value(&hdr->engine_type, VAL_TYPE_UINT8, 1));
        r->setField("ENGINE_ID", Value(&hdr->engine_id, VAL_TYPE_UINT8, 1));
        r->setField("RESERVE", Value(&hdr->reserved, VAL_TYPE_UINT16, 2));
        
        uint8_t *type = new uint8_t(5); //version 5
        r->setField("SYS_HEAD_TYPE", Value(type, VAL_TYPE_UINT8, 1));
        
        //store the header pointer
        r->setField("SYS_HEAD_HOLDER", Value(hdr, VAL_TYPE_CHAR_PTR, sizeof(nfv5_hdr_t)));
        
        return 0;
    }
    else if (7 == getRcdVersion) {
        nfv7_hdr_t* hdr = new nfv7_hdr_t;
        infile.read(2 + (char*)(hdr), sizeof(nfv7_hdr_t) - 2);
        hdr->version = version;
        
        if (invert) invertV7Header(hdr);
        
        getRcdsToGo = hdr->count;
        
        //fill in the record fields
        r->setField("NFDUMP_VERSION", Value(&hdr->version, VAL_TYPE_UINT16, 2));
        r->setField("FLOW_COUNT", Value(&hdr->count, VAL_TYPE_UINT16, 2));
        r->setField("HEAD_SYS_UPTIME", Value(&hdr->uptime, VAL_TYPE_UINT32, 4));
        r->setField("HEAD_UNIX_SECS", Value(&hdr->unix_secs, VAL_TYPE_UINT32, 4));
        r->setField("HEAD_UNIX_NSECS", Value(&hdr->unix_nsecs, VAL_TYPE_UINT32, 4));
        r->setField("FLOW_SEQ", Value(&hdr->seq, VAL_TYPE_UINT32, 4));
        r->setField("RESERVE", Value(&hdr->reserved, VAL_TYPE_UINT32, 4));
        
        uint8_t *type = new uint8_t(7); //version 7
        r->setField("SYS_HEAD_TYPE", Value(type, VAL_TYPE_UINT8, 1));
        
        //store the header pointer
        r->setField("SYS_HEAD_HOLDER", Value(hdr, VAL_TYPE_CHAR_PTR, sizeof(nfv7_hdr_t)));
        
        return 0;
    }
    else if (9 == getRcdVersion) {
        file_header_t* fileHeader = new file_header_t;
        stat_record_t* netflowStat = new stat_record_t;
        
        // if (invert) invertV9Header(hdr);
        
        infile.seekg(0, ios::beg);
        // ret = read(fd, (void *)&FileHeader, sizeof(FileHeader));
        infile.read((char*)fileHeader, sizeof(file_header_t));
        // read(fd, (void *)&NetflowStat, sizeof(NetflowStat));
        infile.read((char*)netflowStat, sizeof(stat_record_t));
        
        getRcdsToGo = fileHeader->NumBlocks;
        
        //fill in the record fields
        // r->setField("MAGIC", Value(&FileHeader->magic, VAL_TYPE_UINT16, 2));
        // r->setField("VERSION", Value(&FileHeader->version, VAL_TYPE_UINT16, 2));
        r->setField("IDENTITY", Value(&fileHeader->ident, VAL_TYPE_CHAR_PTR, 2));
        r->setField("FLAGS", Value(&fileHeader->flags, VAL_TYPE_UINT32, 4));
        r->setField("BLOCK_COUNT", Value(&fileHeader->NumBlocks, VAL_TYPE_UINT32, 4));
        
        uint8_t *type = new uint8_t(9); //version 9
        r->setField("SYS_HEAD_TYPE", Value(type, VAL_TYPE_UINT8, 1));
        
        //store the header pointer
        r->setField("SYS_HEAD_HOLDER", Value(fileHeader, VAL_TYPE_CHAR_PTR, sizeof(file_header_t)));
        r->setField("SYS_NETFLOWSTAT_HOLDER", Value(netflowStat, VAL_TYPE_CHAR_PTR, sizeof(stat_record_t)));
        
        // @@@
        // cerr << "get fileHeader: " << fileHeader << endl;
        // cerr << "get netflowHeader: " << netflowStat << endl;
        
        return 0;
    }
    else {
        return 1;
    }
}

int NfdumpParser::putBlockHeader(Record *r) {
    Value v;
    data_block_header_t* tmp9;
    
    r->getField("BLOCK_HEAD_HOLDER", v);
    tmp9 = (data_block_header_t*)(v.Ptr());
    putBlockRcdsToGo = tmp9->NumBlocks;
    
    // if (invert) invertV9BlockRecord((master_record_t*)(v.Ptr()));
    
    // @@@
    // cerr << "put BlockHeader: " << tmp9 << endl;
    // cerr << "put sizeof(data_block_header_t): " << sizeof(data_block_header_t) << endl;
    
    outfile.write((char*)(tmp9), sizeof(data_block_header_t));
    delete tmp9;
    
    //decrement the record counter
    --putRcdsToGo;
}

// int putHeader(int version)
// Put a header to the output file
//
// parameters: version, the version number to write
// returns: 0 on success, 1 on failure
// modifies: removes data from the header queues
int NfdumpParser::putHeader(Record *r) {
    Value v;
    int version;
    
    //get the header version
    r->getField("SYS_HEAD_TYPE", v);
    version = *(uint8_t*)v.Ptr();
    delete (uint8_t*)v.Ptr();
    
    if (5 == version) {
        nfv5_hdr_t* x;
        r->getField("SYS_HEAD_HOLDER", v);
        x = (nfv5_hdr_t*)(v.Ptr());
        
        putRcdsToGo = x->count;
        putRcdVersion = x->version;
        if (invert) invertV5Header(x);
        
        outfile.write((char*)x, sizeof(nfv5_hdr_t));
        
        delete x;
        
        return 0;
    }
    else if (7 == version) {
        nfv7_hdr_t* x;
        r->getField("SYS_HEAD_HOLDER", v);
        x = (nfv7_hdr_t*)(v.Ptr());
        
        putRcdsToGo = x->count;
        putRcdVersion = x->version;
        if (invert) invertV7Header(x);
        
        outfile.write((char*)x, sizeof(nfv7_hdr_t));
        
        delete x;
        
        return 0;
    }
    else if (9 == version) {
        file_header_t* fileHeader;
        stat_record_t* netflowStat;
        
        r->getField("SYS_HEAD_HOLDER", v);
        fileHeader = (file_header_t*)(v.Ptr());
        
        putRcdsToGo = fileHeader->NumBlocks;
        putRcdVersion = 9;
        
        // if (invert) invertV9Header(x);
        
        /* @@@
         * cerr << "start write file_header_t" << endl;
         * cerr << "file_header_t size: " << sizeof(file_header_t) << endl;
         * cerr << "outfile is open? " << outfile.is_open() << endl;
         * cerr << "putRcdsToGo: " << putRcdsToGo << endl;
         * if ((outfile.rdstate() & ofstream::failbit ) != 0)
         * cerr << "Error opening outfile\n";
         * cerr << "put sizeof(file_header_t): " << sizeof(file_header_t) << endl;
         * cerr << "put sizeof(stat_record_t): " << sizeof(stat_record_t) << endl;
         */
        outfile.write((char*)fileHeader, sizeof(file_header_t));
        // @@@
        // cerr << "put fileHeader: " << fileHeader << endl;
        
        delete fileHeader;
        
        r->getField("SYS_NETFLOWSTAT_HOLDER", v);
        netflowStat = (stat_record_t*)(v.Ptr());
        // @@@
        // cerr << "put NetflowHeader: " << netflowStat << endl;
        
        outfile.write((char*)netflowStat, sizeof(stat_record_t));
        
        delete netflowStat;
        
        return 0;
    }
    else {
        return 1;
    }
}

static void print_record(void *record, char *s ) {
    char as[40], ds[40], datestr1[64], datestr2[64], flags_str[16];
    time_t when;
    struct tm     *ts;
    master_record_t *r = (master_record_t *)record;
    
    if ( (r->flags & FLAG_IPV6_ADDR ) != 0 ) { // IPv6
        r->v6.srcaddr[0] = htonll(r->v6.srcaddr[0]);
        r->v6.srcaddr[1] = htonll(r->v6.srcaddr[1]);
        r->v6.dstaddr[0] = htonll(r->v6.dstaddr[0]);
        r->v6.dstaddr[1] = htonll(r->v6.dstaddr[1]);
        inet_ntop(AF_INET6, r->v6.srcaddr, as, sizeof(as));
        inet_ntop(AF_INET6, r->v6.dstaddr, ds, sizeof(ds));
    } else {        // IPv4
        r->v4.srcaddr = htonl(r->v4.srcaddr);
        r->v4.dstaddr = htonl(r->v4.dstaddr);
        inet_ntop(AF_INET, &r->v4.srcaddr, as, sizeof(as));
        inet_ntop(AF_INET, &r->v4.dstaddr, ds, sizeof(ds));
    }
    as[40-1] = 0;
    ds[40-1] = 0;
    
    when = r->first;
    ts = localtime(&when);
    strftime(datestr1, 63, "%Y-%m-%d %H:%M:%S", ts);
    
    when = r->last;
    ts = localtime(&when);
    strftime(datestr2, 63, "%Y-%m-%d %H:%M:%S", ts);
    
    snprintf(flags_str, 7, "  0x%2x\n", r->tcp_flags );
    snprintf(s, 1024-1, "\n"
            "Flow Record: \n"
            "  Flags       =       0x%.8x\n"
            "  size        =            %5u\n"
            "  mark        =            %5u\n"
            "  srcaddr     = %16s\n"
            "  dstaddr     = %16s\n"
            "  first       =       %10u [%s]\n"
            "  last        =       %10u [%s]\n"
            "  msec_first  =            %5u\n"
            "  msec_last   =            %5u\n"
            "  dir         =              %3u\n"
            "  tcp_flags   =             0x%2x %s\n"
            "  prot        =              %3u\n"
            "  tos         =              %3u\n"
            "  input       =            %5u\n"
            "  output      =            %5u\n"
            "  srcas       =            %5u\n"
            "  dstas       =            %5u\n"
            "  srcport     =            %5u\n"
            "  dstport     =            %5u\n"
            "  dPkts       =       %10llu\n"
            "  dOctets     =       %10llu\n"
            ,
            r->flags, r->size, r->mark, as, ds, r->first, datestr1, r->last,
            datestr2,
            r->msec_first, r->msec_last, r->dir, r->tcp_flags, flags_str, r->prot, r->tos,
            r->input, r->output, r->srcas, r->dstas, r->srcport, r->dstport,
            (unsigned long long)r->dPkts, (unsigned long long)r->dOctets);
    
    s[1024-1] = 0;
}

void NfdumpParser::WriteRecordOutput(master_record_t* input_record) {
    // uint32_t *u;
    size_t size;
    // void* p = (void*)input_record;
    
    // restore byte order
    if ((input_record->flags & FLAG_IPV6_ADDR ) != 0) { // IPv6
        input_record->v6.srcaddr[0] = ntohll(input_record->v6.srcaddr[0]);
        input_record->v6.srcaddr[1] = ntohll(input_record->v6.srcaddr[1]);
        input_record->v6.dstaddr[0] = ntohll(input_record->v6.dstaddr[0]);
        input_record->v6.dstaddr[1] = ntohll(input_record->v6.dstaddr[1]);
    } else {        // IPv4
        input_record->v4.srcaddr = ntohl(input_record->v4.srcaddr);
        input_record->v4.dstaddr = ntohl(input_record->v4.dstaddr);
    }
    
    // Copy common data block
    size = sizeof(common_record_t) - sizeof(uint8_t[4]);
    // memcpy((void *)commonOut, p, size);
    
    outfile.write((char*)(input_record), size);
    
    // p = (void *)((pointer_addr_t)input_record + size);
    
    if ((input_record->flags & FLAG_IPV6_ADDR) != 0) {
        // IPv6
        outfile.write((char*)input_record->v6.srcaddr, sizeof(ipv6_block_t));
    }
    else {
        // IPv4
        outfile.write((char*)&input_record->v4.srcaddr, sizeof(uint32_t));
        outfile.write((char*)&input_record->v4.dstaddr, sizeof(uint32_t));
    }
// cerr << "2" << endl;
    // packet counter
    if ((input_record->flags & FLAG_PKG_64 ) != 0) {
        // 64bit packet counter
        value64_t l;
        l.val.val64 = input_record->dPkts;
        outfile.write((char*)&l.val.val32[0], sizeof(uint32_t));
        outfile.write((char*)&l.val.val32[1], sizeof(uint32_t));
    }
    else {
        // 32bit packet counter
        outfile.write((char*)&input_record->dPkts, sizeof(uint32_t));
        
        // output_record->dPkts = *((uint32_t *)p);
        // p = (void *)((pointer_addr_t)p + sizeof(uint32_t));
    }
// cerr << "3" << endl;
    // byte counter
    if ((input_record->flags & FLAG_BYTES_64) != 0 ) {
        // 64bit byte counter
        value64_t l;
        l.val.val64 = input_record->dOctets;
        outfile.write((char*)&l.val.val32[0], sizeof(uint32_t));
        outfile.write((char*)&l.val.val32[1], sizeof(uint32_t));
    }
    else {
        // 32bit bytes counter
        outfile.write((char*)&input_record->dOctets, sizeof(uint32_t));
    }
}

void NfdumpParser::invertV5Header(nfv5_hdr_t* hdr) {
    hdr->version = invShort(hdr->version);
    hdr->count = invShort(hdr->count);
    hdr->uptime = invLong(hdr->uptime);
    hdr->unix_secs = invLong(hdr->unix_secs);
    hdr->unix_nsecs = invLong(hdr->unix_nsecs);
    hdr->seq = invLong(hdr->seq);
}

void NfdumpParser::invertV7Header(nfv7_hdr_t* hdr) {
    hdr->version = invShort(hdr->version);
    hdr->count = invShort(hdr->count);
    hdr->uptime = invLong(hdr->uptime);
    hdr->unix_secs = invLong(hdr->unix_secs);
    hdr->unix_nsecs = invLong(hdr->unix_nsecs);
    hdr->seq = invLong(hdr->seq);
}

void NfdumpParser::invertV5Record(nfv5_rcd_t* rcd) {
    rcd->srcaddr = invLong(rcd->srcaddr);
    rcd->dstaddr = invLong(rcd->dstaddr);
    rcd->nexthop = invLong(rcd->nexthop);
    rcd->input = invShort(rcd->input);
    rcd->output = invShort(rcd->output);
    rcd->dpkts = invLong(rcd->dpkts);
    rcd->doctets = invLong(rcd->doctets);
    rcd->first = invLong(rcd->first);
    rcd->last = invLong(rcd->last);
    rcd->srcport = invShort(rcd->srcport);
    rcd->dstport = invShort(rcd->dstport);
    rcd->src_as = invShort(rcd->src_as);
    rcd->dst_as = invShort(rcd->dst_as);
}

void NfdumpParser::invertV7Record(nfv7_rcd_t* rcd) {
    rcd->srcaddr = invLong(rcd->srcaddr);
    rcd->dstaddr = invLong(rcd->dstaddr);
    rcd->nexthop = invLong(rcd->nexthop);
    rcd->input = invShort(rcd->input);
    rcd->output = invShort(rcd->output);
    rcd->dpkts = invLong(rcd->dpkts);
    rcd->doctets = invLong(rcd->doctets);
    rcd->first = invLong(rcd->first);
    rcd->last = invLong(rcd->last);
    rcd->srcport = invShort(rcd->srcport);
    rcd->dstport = invShort(rcd->dstport);
    rcd->src_as = invShort(rcd->src_as);
    rcd->dst_as = invShort(rcd->dst_as);
    rcd->flags2 = invShort(rcd->flags2);
    rcd->router_sc = invLong(rcd->router_sc);
}

// ***********************
// Static helper functions
// ***********************
static uint32_t invLong(uint32_t x) {
    return ((0xFF000000 & x) >> 24) |
            ((0x00FF0000 & x) >> 8) |
            ((0x0000FF00 & x) << 8) |
            ((0x000000FF & x) << 24);
}

static uint16_t invShort(uint16_t x) {
    return ((0xFF00 & x) >> 8) | ((0x00FF & x) << 8);
}

// *************************
// Dynamic loading functions
// *************************

//Dynamic loading related function
extern "C" LogParser * create(string srcLog, string destLog, string errOutLog, string anonyField) {
    if (errOutLog.compare("") == 0) {
        return new NfdumpParser(srcLog, destLog, errfn);
    }
    else {
        return new NfdumpParser(srcLog, destLog, errOutLog);
    }
}

//destroy an object for iptableLogParser
extern "C" void destroy(LogParser *p) {
    delete p;
}
