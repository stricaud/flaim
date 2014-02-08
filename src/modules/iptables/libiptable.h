/*****************************************************************************
 *
 * libiptable.h
 *
 * Header file for the record/value version of the iptable/netfilter module
 *
 * Last modified by Greg Colombo
 * Tue Jun 20 09:41:20 CDT 2006
 *
 *****************************************************************************/

#ifndef _LIBIPTABLE_H
#define _LIBIPTABLE_H

#include <fstream>
#include <string>
#include <map>

#include "flaim.h"
#include "record.h"
#include "map_record.h"

using namespace std;


// other useful constants for this module
const int epochYear = 70; // mktime() uses seconds since 1/1/1900

class IptableParser : public LogParser
{
    public:

	IptableParser(string in, string out, string fields);
	~IptableParser();

	// get the location of the module schema file
	int getModuleSchemaFile(string & schemaFile);

	// get one record
	Record* getRecord();

	// put one record
	int putRecord(Record * r);

	// check to see if we are at the end of the log
	int isEndofLog();

	// get the current record number
	int getRecordSeq(int & rcdSeq);

	// reset the file pointer
	int resetFilePointer();

    private:

	void initMonthMap();
	void initHexMap();

	int putRecordStreaming(Record * r);
	
	int getRecordHeader(const char* s, Record * r);
	int getIPv4Header(const char* s, Record * r);
	int getTCPHeader(const char* s, Record * r);
	int getUDPHeader(const char* s, Record * r);
	int getICMPHeader(const char* s, Record * r);
	int getGenericData(const char* s, Record * r);

	int putTCPHeader(Record * r);
	int putUDPHeader(Record * r);
	int putICMPHeader(Record * r);
	int putGenericData(Record * r);

	int putTCPHeaderStr(Record * r);
	int putUDPHeaderStr(Record * r);
	int putICMPHeaderStr(Record * r);
	int putGenericDataStr(Record * r);
	
	int curSeq; // current record number
	bool streamingInput, streamingOutput;

	ifstream infile;
	ofstream outfile, errfile;

	map<char*, int, ltstr> utilMonthMap;
	map<char, int> utilHexMap;
};

#endif
