/**********************************************************
 *
 * libpcap_parser.h
 * Header file for a generalized libpcap parser
 *
 * originally drafted by Greg Colombo
 * last modified Fri May 19 14:59:01 CDT 2006
 * 
 **********************************************************/

#ifndef _LIBPCAP_PARSER_H
#define _LIBPCAP_PARSER_H

#include <stdint.h>
#include <string>
#include <fstream>

#include <unistd.h>
#include <sys/time.h>

#include "libpcap_structs.h"
#include "flaim.h"
#include "value.h"
#include "record.h"

using namespace std;

//char* default_err_file = "libpcap_flaim.err.log";


class LibpcapParser : public LogParser
{
    public:

	// constructors
	// LibpcapParser(); 
	LibpcapParser(string const & inputfile, string const & outputfile,
		string const & errorfile);

	~LibpcapParser();

	// give the name of the module schema file to the caller
	int getModuleSchemaFile(string & schemaFile);

	// get one record from the input file
	Record * getRecord();

	// put one record to the output file
	int putRecord(Record * r);

	// test to see if the end of the log has been reached
	int isEndofLog();

	// print the number of the current record
	int getRecordSeq(int & seq);

	// reset the pointer to the start of the file
	int resetFilePointer();

    private:

	// functions
	void initializeGlobalHeader();
	void initializeFiles();
	int writeErrRecord(char* header, char* bufRecord);
	
	uint16_t invShort(uint16_t target);
	uint32_t invLong(uint32_t target);

	// general get functions
	int getEthernetHeader(Record *) throw (exception&, ifstream::failure&);
	int getIPv4Header(Record *) throw (exception&, ifstream::failure&);
	int getTCPHeader(Record *) throw (exception&, ifstream::failure&);
	int getUDPHeader(Record *) throw (exception&, ifstream::failure&);
	int getICMPHeader(Record *) throw (exception&, ifstream::failure&);

	// general put functions
	int putEthernetHeader(Record * r);
	int putIPv4Header(Record * r);
	int putTCPHeader(Record * r);
	int putUDPHeader(Record * r);
	int putICMPHeader(Record * r);

	// ICMP-specific functions
	int getICMPGeneric(Record *) throw (exception&, ifstream::failure&);
	int getICMPTimestamp(Record *) throw (exception&, ifstream::failure&);

	int putICMPGeneric(Record * r);
	int putICMPTimestamp(Record * r);
	
	// member variables
	pcap_hdr_t globalHeader;
	
	string infileName; // name of file to read from
	string outfileName; // name of file to write to
	string errfileName; // name of file to write errors to
	ifstream infile; // descriptor for file to read from
	ofstream outfile; // descriptor for file to write to
	ofstream errfile; // descriptor for file to write errors to

	int curSeq; // current record number

	// constants to tell us the network and transport layer types
	int netTypeGet;
	int transTypeGet;
	int netTypePut;
	int transTypePut;

	bool invHeaders; // if true, invert libpcap headers' byte ordering
       bool isErrors;
	   	
};

#endif // _LIBPCAP_PARSER_H
