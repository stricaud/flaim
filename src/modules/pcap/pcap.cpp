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
// $Id: pcap.cpp 928 2007-08-16 21:25:48Z byoo3 $
//
// Module which provides faster I/O functionality for pcap-format logs
//
//********************************************************************

#include <istream>
#include <fstream>
#include <iostream>
#include <inttypes.h>
#include <netinet/in.h>
#include <sstream>
#include <string>

#include "libpcap_structs.h"
#include "libpcap_parser.h"
#include "map_record.h"
#include "value.h"
#include "value_types.h"

using namespace std;

static const string xmlFileName = "pcap_schema.xml";
static const string errfn = "libpcap_flaim.errors";

//#define DEBUG // comment this out to remove debugging messages

// LibpcapParser::LibpcapParser(string const & infile, string const & outfile)
// Constructor
//
// Parameters: infile, the file to read from; outfile, the file to write to
// Returns: Constructs a new object
// Modifies: Nothing
// Notes: The new parser will attempt to open input/output streams
LibpcapParser::LibpcapParser(string const & inputfile,
                             string const & outputfile, string const & errorfile) :
		infileName(inputfile), outfileName(outputfile), errfileName(errorfile),
		curSeq(0)
{
/*
	char curCTime[20];
	sprintf(curCTime, ".%d", time (NULL));
	errfileName.append(curCTime);
*/
	cerr << "*** pcap module: input from "
		<< "[" << infileName << "]" << endl
		<< "*** pcap module: output to " << "[" << outfileName << "]" << endl
		<< "*** pcap module: errors to " << "[" << errfileName << "]"
		<< endl;

	if ("" == infileName)
	{
		cerr << "*** pcap module: This module does not support "
			<< "streaming input." << endl
			<< "*** pcap module: Please specify an input file." << endl;
		exit(2);
	}

	if ("" == outfileName)
	{
		cerr << "*** pcap module: This module does not support "
			<< "streaming output." << endl
			<< "*** pcap module: Please specify an output file." << endl;
	}

	// open the data files!
	infile.open(infileName.c_str(), ifstream::binary | ifstream::in);
	outfile.open(outfileName.c_str(), ofstream::binary | ofstream::out);
	errfile.open(errfileName.c_str(), ofstream::binary | ofstream::out);

	// make sure nothing failed
	if (infile.fail())
	{
		cerr << "*** pcap module: Error opening input file." << endl;
		exit(2);
	}
	if (outfile.fail())
	{
		cerr << "*** pcap module: Error opening output file." << endl;
		exit(2);
	}
	if (errfile.fail())
	{
		cerr << "*** pcap module: Error opening error file." << endl;
		exit(2);
	}

	infile.exceptions(ifstream::failbit |ifstream::badbit);
	
	// all the checks passed, prepare to grab data
	initializeGlobalHeader();
	initializeFiles();
}

// destructor
// I hate I/O :(
LibpcapParser::~LibpcapParser()
{
	outfile.flush();
	errfile.flush();
}

// int writeErrRecord(int offsetCurRec, int lenErrRecord)
// Write error record to .errors file
//
// Parameters: offsetErrRec, lenErrRecord
// Returns: 0 for success, nonzero for failure
// int LibpcapParser::writeErrRecord(char* header, char* bufRecord)
// {
// 	outfile.write(header, sizeof(pcap_pckt_hdr_t));
	// delete (pcap_pckt_hdr_t*)(header);
//	errfile.write(bufRecord, lenErrRecord);
//	delete[] errRecord;

//	return 0;
// }

// int getModuleSchemaFile(string & schemaFile)
// Return, by alias, the name of the schema file for this module
//
// Parameters: schemaFile, the target to store into
// Returns: 0 for success, nonzero for failure
// Modifies: Nothing except the parameter's referent
int LibpcapParser::getModuleSchemaFile(string & schemaFile)
{
	schemaFile = xmlFileName;
	return 0;
}

// Record * getRecord()
// Pack the next available record from the log into a Record object
//
// Parameters: none
// Returns: a pointer to a new record on success, NULL on failure
// Modifies: The record passed in as a parameter is filled in with data
Record * LibpcapParser::getRecord()
{
	int bcHeaderSize = 0; // count of bytes read as header data
	int bcRead = 0; // count of total number of bytes read
	int bcRval = 0; // holder for return values from getHeader fns
	int offsetCurRec =  infile.tellg(); // offset of current record
	bool isSafe = true;
	bool isException = false;
	
	// initialize a new record
	Record * r = new MapRecord();

	// Read a packet header
	pcap_pckt_hdr_t* phdr = new pcap_pckt_hdr_t;
	try
	{
		infile.read((char*)phdr, sizeof(pcap_pckt_hdr_t));
	}
	catch (ifstream::failure& ex)
	{
		cerr << "*** pcap module:" << curSeq <<":I/O error in reading header: " 
			<< ex.what() << endl;
		return NULL;
	}
	catch (exception& ex)
	{
		cerr << "*** pcap module:" << curSeq <<":Fatal error in reading header: " 
			<< ex.what() << endl;
		return NULL;
	}

	bcRead += sizeof(pcap_pckt_hdr_t);

	// If necessary, invert the headers for endianness reasons
	if (invHeaders)
	{
		phdr->ts_sec = invLong(phdr->ts_sec);
		phdr->ts_usec = invLong(phdr->ts_usec);
		phdr->incl_len = invLong(phdr->incl_len);
		phdr->orig_len = invLong(phdr->orig_len);
	}

	// Record Length
	int lenRec = sizeof(pcap_pckt_hdr_t) + phdr->incl_len;

	// Insert packet header fields into the record
	r->setField("TS_SEC", Value(&phdr->ts_sec, VAL_TYPE_UINT32, 4));
	r->setField("TS_USEC", Value(&phdr->ts_usec, VAL_TYPE_UINT32, 4));
	r->setField("INCL_LEN", Value(&phdr->incl_len, VAL_TYPE_UINT32, 4));
	r->setField("ORIG_LEN", Value(&phdr->orig_len, VAL_TYPE_UINT32, 4));
	r->setField("SYS_PCKT_HDR_LOC", Value(phdr, VAL_TYPE_CHAR_PTR,
			sizeof(pcap_pckt_hdr_t)));

	// Buffer for record data. to reduce IO, complexity
	char* bufRec = new char[phdr->incl_len];
	int offsetTemp = infile.tellg();

	try
	{
		infile.read(bufRec, phdr->incl_len);
		infile.seekg (offsetTemp, ios::beg);
		
		// Get the packet's link layer
		switch (globalHeader.network)
		{
		case ENCAP_ETHERNET: 
			bcRval = getEthernetHeader(r); break;
		default:
			cerr << "*** pcap module: Invalid link layer type in "
				<< "getRecord (error code: " << globalHeader.network 
				<<  ", record number: " <<	curSeq << ", offset: " 
				<< offsetCurRec << ", length: "<< lenRec << ")" << endl;

			errfile.write((char*)phdr, sizeof(pcap_pckt_hdr_t));
			errfile.write(bufRec, phdr->incl_len);
		
			bcRval = 0;
			isSafe = false;
			// return NULL; // whole data of pcap file cannot be handled.
		}
		bcHeaderSize += bcRval;
		bcRead += bcRval;
	 	// cerr << "*** pcap module:" << curSeq <<":read link layer: " << bcRval << endl;

		// Get the packet's network layer
		if (isSafe)
		{
			switch(netTypeGet) // filled in by link layer get call
			{
			case NET_IPV4:
				bcRval = getIPv4Header(r); break;
			default:
				cerr << "*** pcap module: Invalid network layer type in "
					<< "getRecord (error code: " << netTypeGet <<  ", record number: " 
					<< curSeq << ", offset: " << offsetCurRec <<  ", length: "
					<< lenRec << ")" << endl;
				errfile.write((char*)phdr, sizeof(pcap_pckt_hdr_t));
				errfile.write(bufRec, phdr->incl_len);
				isSafe = false;
				bcRval = 0;
			}
		}
		bcHeaderSize += bcRval;
		bcRead += bcRval;
	 	// cerr << "*** pcap module:" << curSeq <<":read network layer: " << bcRval << endl;

		// Get the packet's transport layer
		if (isSafe) 
		{
			switch(transTypeGet)
			{
			case TRANS_ICMP:
				if (phdr->incl_len <= bcRead) 
				{
					bcRval = 0;
					break;
				}
				bcRval = getICMPHeader(r); break;
			case TRANS_TCP:
				bcRval = getTCPHeader(r); break;
			case TRANS_UDP:
				bcRval = getUDPHeader(r); break;
			default:
				cerr << "*** pcap module: Warning: Invalid transport layer type in"
					<< " getRecord (error code: " << transTypeGet <<  ", record number: " 
					<< curSeq << ", offset: " << offsetCurRec <<  ", length: "
					<< lenRec << ")" << endl;
				errfile.write((char*)phdr, sizeof(pcap_pckt_hdr_t));
				errfile.write(bufRec, phdr->incl_len);
				bcRval = 0;
				isSafe = false;
				break;
			}
		}
		bcHeaderSize += bcRval;
		bcRead += bcRval;
	 	// cerr << "*** pcap module:" << curSeq <<":read transport layer: " << bcRval << endl;	
	 }
	catch (ifstream::failure& ex)
	{
		cerr << "*** pcap module:" << curSeq <<":I/O error in reading record: " 
			<< ex.what() << endl;
		isException = true;
	}
	catch (exception& ex)
	{
		cerr << "*** pcap module:" << curSeq <<":Fatal error in reading record: " 
			<< ex.what() << endl;
		isException = true;
	}

	// Do application layer handling
	int move = phdr->incl_len - bcHeaderSize; // amount of data left
	
	// err flag setting
	if (isException) 
	{
		errfile.write((char*)phdr, sizeof(pcap_pckt_hdr_t));
		errfile.write(bufRec, phdr->incl_len);
		delete[] bufRec;
		return NULL;
	}
	else if (!isSafe) {
		returnCode = 1;
		uint8_t* isSafe = new uint8_t(1);
		r->setField("IS_SAFE", Value(isSafe, VAL_TYPE_INT8, 1));
	
		char* errbuf = new char[phdr->incl_len];
		memcpy(errbuf, bufRec, phdr->incl_len);
		r->setField("SYS_APP_DATA", Value(errbuf, VAL_TYPE_CHAR_PTR, phdr->incl_len));
		if (move > 0) infile.seekg((int)infile.tellg() + move, ios::beg);
	}
	else if (move > 0)
	{
		char* buf = new char[move];
		infile.read(buf, move);
		bcRead += move;
		r->setField("SYS_APP_DATA", Value(buf, VAL_TYPE_CHAR_PTR, move));
	} 
	else if (move < 0)
	{
		cerr << "*** pcap module: Warning: record parsing becomes inconsistent (" 
			<< curSeq << ", offset: " << offsetCurRec <<  ", length: "
			<< phdr->incl_len << ")" << endl;
	}
	
	delete[] bufRec;

	// Finally, increment the sequence number
	++curSeq;

	return r;
}

// int putRecord(Record * r)
// Write a record to the output file
//
// Parameters: r, the record to write
// Returns: 0 on success, 1 on failure
// Modifies: Data is written to the output file
int LibpcapParser::putRecord(Record * r)
{
	Value v;

	// get the packet header & write it
	r->getField("SYS_PCKT_HDR_LOC", v);
	outfile.write((char*)(v.Ptr()), sizeof(pcap_pckt_hdr_t));
	delete (pcap_pckt_hdr_t*)(v.Ptr());

	// error handling
	if (0 == r->getField("IS_SAFE", v))
	{
		delete (uint8_t*)v.Ptr();
		if (0 == r->getField("SYS_APP_DATA", v))
		{
			outfile.write((char*)(v.Ptr()), v.Bytes());
			delete[] (char*)(v.Ptr());
		}
		return 0;
	}

	// get the link layer and write it
	switch(globalHeader.network)
	{
	case ENCAP_ETHERNET:
		putEthernetHeader(r); break;
	default:
		cerr << "*** pcap module: invalid link layer type in putRecord "
		<< "(" << globalHeader.network << ")" << endl;
		return 1;
	}

	// get the network layer and write it
	switch(netTypePut) // filled in by the previous layer
	{
	case NET_IPV4:
		putIPv4Header(r);
		break;
	default:
		cerr << "*** pcap module: invalid network layer type in "
		<< "putRecord (" << netTypePut << ")" << endl;
		return 1;
	}

	// get the transport layer and write it
	switch(transTypePut) // filled in by network layer
	{
	case TRANS_ICMP:
		putICMPHeader(r); break;
	case TRANS_TCP:
		putTCPHeader(r); break;
	case TRANS_UDP:
		putUDPHeader(r); break;
	default:
		cerr << "*** pcap module: Warning: invalid transport layer type in"
		<< " putRecord (" << transTypePut << ")" << endl;
		break;
		// return 1;
	}

	if (0 == r->getField("SYS_APP_DATA", v))
	{
		outfile.write((char*)(v.Ptr()), v.Bytes());
		delete[] (char*)(v.Ptr());
	}

	delete r;

	return 0;
}

// int isEndofLog()
// Test to see if the end of the current log has been reached
//
// Parameters: None
// Returns: 1 if there are no more records in the log, 0 otherwise
// Modifies: Nothing
int LibpcapParser::isEndofLog()
{
	bool eof = true;
	
	try 
	{
		infile.peek();
		eof = infile.eof();
		if (!eof) return 0;
	}
	catch (ifstream::failure& ex) {}

	return 1;
}

// int getRecordSeq(int & seq)
// Return the number of the current record
//
// Parameters: seq, the variable to store the record number into
// Returns: 0 on success
// Modifies: The parameter
int LibpcapParser::getRecordSeq(int & seq)
{
	seq = curSeq;
	return 0;
}

// int resetFilePointer()
// Move back to the first record in the file
//
// Parameters: None
// Returns: 0 on success
// Modifies: The current sequence number and location in the file are changed
int LibpcapParser::resetFilePointer()
{
	infile.seekg(0, ios::beg);
	initializeGlobalHeader();
	curSeq = 0;
	return 0;
}

// void initializeGlobalHeader()
// Initialize the global header field
//
// Parameters: None
// Returns: Nothing
// Modifies: The file's global header data is filled in
// Notes: This function assumes that the stream's get pointer currently
// points to the global header; the get pointer position is caller saved
//
// This function must be called before beginning to read records or things
// will get ugly in a hurry
void LibpcapParser::initializeGlobalHeader()
{
	infile.read((char*)(&globalHeader), 24);

	invHeaders = false;
	// if the magic number is backwards, we need to flip the
	// byte ordering in those fields
	if (0xD4C3B2A1 == globalHeader.magic_number)
	{
		invHeaders = true;
	}
	else if (0xA1B2C3D4 != globalHeader.magic_number)
	{
		cerr << "*** pcap module: bad magic number error! check input data" << endl;
		exit(2);
	}

	if (invHeaders)
	{
		globalHeader.magic_number = invLong(globalHeader.magic_number);
		globalHeader.version_major = invShort(globalHeader.version_major);
		globalHeader.version_minor = invShort(globalHeader.version_minor);
		globalHeader.thiszone =
			(int32_t)invLong((uint32_t)globalHeader.thiszone);
		globalHeader.sigfigs = invLong(globalHeader.sigfigs);
		globalHeader.snaplen = invLong(globalHeader.snaplen);
		globalHeader.network = invLong(globalHeader.network);
	}

}

// void initializeFiles()
// Initialize the output file by writing the global header to it
//
// Parameters: None
// Returns: Nothing
// Modifies: The output file has global header data written to it
// Notes: This function writes this->globalHeader to disk and thus assumes
// that it is already initialized
void LibpcapParser::initializeFiles()
{
	outfile.write((char*)(&globalHeader), 24);
	errfile.write((char*)(&globalHeader), 24);
}

/**********************************************************
 * Utility and file-scope functions
 **********************************************************/

// uint16_t invShort(uint16_t target)
// Invert the byte ordering of a 16-bit value
//
// Parameters: target, the value to invert
// Returns: the inverted value
// Modifies: Nothing
uint16_t LibpcapParser::invShort(uint16_t target)
{
	uint16_t rval = 0;

	rval |= (target & 0x00FF) << 8;
	rval |= (target & 0xFF00) >> 8;

	return rval;
}

// uint32_t invLong(uint32_t target)
// Invert the byte ordering of a 32-bit value
//
// Parameters: target, the value to invert
// Returns: the inverted value
// Modifies: Nothing
uint32_t LibpcapParser::invLong(uint32_t target)
{
	uint32_t rval = 0;

	rval |= (target & 0x000000FF) << 24;
	rval |= (target & 0x0000FF00) << 8;
	rval |= (target & 0x00FF0000) >> 8;
	rval |= (target & 0xFF000000) >> 24;

	return rval;
}

/*****************************
 * Dynamic loading functions *
 *****************************/

extern "C" LogParser * create(string srcLog, string destLog, string errOutLog, string anonyField)
{
	if (errOutLog.compare("") == 0)
	{
		return new LibpcapParser(srcLog, destLog, errfn);
	}
	else
	{
		return new LibpcapParser(srcLog, destLog, errOutLog);
	}
}

extern "C" void destroy(LogParser *p)
{
	delete p;
}
