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
// $Id: nfdump.h 269 2006-08-04 16:32:13Z gcolombo $
//
// Header file that defines the interface to an nfcap/nfdump format
// log parser
// 
//********************************************************************

#ifndef _NFDUMP_H
#define _NFDUMP_H

#include <string>
#include <fstream>
#include <deque>

#include "record.h"
#include "flaim.h"
#include "nfd_structs.h"
#include "nffile.h"

using namespace std;

class NfdumpParser : public LogParser
{
    public:
	NfdumpParser(string in, string out, string dummy);

	~NfdumpParser();

	// module API calls below
	int getModuleSchemaFile(string & schemaFile);
	int isEndofLog();
	int getRecordSeq(int & rcdSeq);
	int resetFilePointer();

	Record * getRecord();
	int putRecord(Record * r);

    private:
	int getNextHeader(Record*) throw (exception&, ifstream::failure&);
	int getNextBlockHeader(Record*) throw (exception&, ifstream::failure&);
	int putHeader(Record *r);
	int putBlockHeader(Record *r);
	
	int getVersion5(Record*) throw (exception&, ifstream::failure&);
	int getVersion7(Record*) throw (exception&, ifstream::failure&);
	int getVersion9(Record*) throw (exception&, ifstream::failure&);

	void invertV5Header(nfv5_hdr_t* hdr);
	void invertV7Header(nfv7_hdr_t* hdr);
	void invertV5Record(nfv5_rcd_t* rcd);
	void invertV7Record(nfv7_rcd_t* rcd);
	void WriteRecordOutput(master_record_t *input_record);
	
	int curSeq; // current sequence number

	int getRcdVersion; // current record type we are reading
	int putRcdVersion;
	int getRcdsToGo; // number of records left to read before a header
	int putRcdsToGo;
	int getBlockRcdsToGo; // number of records in block left to read before a header
	int putBlockRcdsToGo;

	common_record_t* flow_record;
	common_record_t* in_buff;
	
	// file descriptors for I/O
	ifstream infile;
	ofstream outfile;
	ofstream errfile;

	bool invert;
};

#endif // _NFDUMP_H
