/*****************************************************************************
 *
 * libiptable.cpp
 *
 * Implementation file for the record/value version of the iptable parser
 *
 * last modified by Greg Colombo
 * Mon Jun 26 14:50:10 GMT 2006
 *
 * $Id: libiptable.cpp 1049 2008-02-22 20:00:59Z byoo3 $
 *
 *****************************************************************************/

#include <iostream>
#include <map>
#include <sstream>
#include <stdint.h>
#include <string>
#include <cstring>
#include <ctime>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "libiptable.h"
#include "map_record.h"
#include "net_structs.h"
#include "record.h"
#include "value.h"
#include "value_types.h"

// change the line below to change the schema file name that is returned
// note that this feature may be deprecated in versions 0.3 and later
#define SCHEMAFILE "iptable_schema.xml"

// define the input line length
#define INLINE_LENGTH 512 

using namespace std; // sinful, but we'll do it anyway

static string errfn = "libiptable_flaim.errors";
static char tmpinstr[INLINE_LENGTH];

//
// Inline utility functions: declarations
//
inline uint8_t convertToUint8(const string & s);
inline uint32_t convertToUint32(const string & s);

//
// Inline utility functions: definitions
//
inline uint8_t convertToUint8(const string & s)
{
	std::istringstream i(s);
	uint32_t x;
	i >> x;
	return (x & 0xFF);
}
inline uint32_t convertToUint32(const string & s)
{
	std::istringstream i(s);
	uint32_t x;
	i >> x;
	return x;
}

// *********************************************
// PUBLIC IptableParser implementation functions
// *********************************************

// IptableParser::IptableParser(string, string, string)
// Constructor which opens the appropriate files for reading and writing
//
// Parameters:
// 	in -- the name of the input file to open
// 	out -- the name of the output file to open
// 	fields -- deprecated
// Returns: nothing
// Modifies: nothing
IptableParser::IptableParser(string in, string out, string err) :
		curSeq(0), streamingInput(false), streamingOutput(false)
{
	// attempt to open the input file for reading. if no file was specified
	// or opening fails for any reason, default to reading from standard
	// input and print an error message
	if ("" != in)
	{
		infile.open(in.c_str(), ifstream::in);
		if (!infile.is_open())
		{
			cerr << "*** libiptable: Error opening input file" << endl;
		}
		else
		{
			cerr << "*** libiptable: Using " << in << " as input file" << endl;
		}
	}
	else
	{
		cerr << "*** libiptable: reading from stdin" << endl;
		streamingInput = true;
	}

	// similarly, if the output file cannot be opened for writing
	// (probably because someone else has control over it at the moment),
	// default to writing to standard output
	if ("" != out)
	{
		outfile.open(out.c_str(), ofstream::out);
		if (!outfile.is_open())
		{
			cerr << "*** libiptable: Error opening output file" << endl;
		}
		else
		{
 			cerr << "*** libiptable: Using " << out << " as output file" << endl;
		}
	}
	else
	{
		cerr << "*** libiptable: writing to stdout" << endl;
		streamingOutput = true;
	}
/*
        char curCTime[20];
        sprintf(curCTime, ".%d", time (NULL));
        errfn.append(curCTime);
*/
	errfile.open(err.c_str(), ofstream::out);
	cerr << "*** libiptable: Erroneous packets will be written to "
		<< err << endl;

	// initialize the mapping used to map months to numbers
	// and hex digits to integers
	initMonthMap();
	initHexMap();
}

// ~IptableParser()
// Destructor
IptableParser::~IptableParser()
{
	infile.close();
	outfile.close();
	errfile.close();
}

// int getModuleSchemaFile(string & schemaFile)
// Return, by reference, the name of the module schema file
//
// Parameters: schemaFile, the name of the schema file
// Returns: 0 on success
// Modifies: the referent of the argument is changed
int IptableParser::getModuleSchemaFile(string & schemaFile)
{
	schemaFile = SCHEMAFILE;
	return 0;
}

// int isEndofLog();
// Indicates whether there are more records in the log
//
// Parameters: none
// Returns: 0 if there are more records to read, 1 otherwise
// Modifies: nothing
int IptableParser::isEndofLog()
{
	if (streamingInput)
	{
		cin.peek();
		return cin.eof();
	}

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

// int getRecordSeq(int & rcdSeq)
// Return, by reference, the current sequence number
//
// Parameters: rcdSeq, the reference variable to store the sequence number to
// Returns: 0 on success
// Modifies: the parameter's referent
int IptableParser::getRecordSeq(int & rcdSeq)
{
	rcdSeq = curSeq;
	return 0;
}

// int resetFilePointer()
// Begin reading from the start of the file
//
// Parameters: none
// Returns: 0 on success
// Modifies: the get pointer is moved to the start of the file
// and curSeq is reset to 0
int IptableParser::resetFilePointer()
{
	curSeq = 0;
	infile.seekg(0, ios::beg);
}

// Record * getRecord()
// Allocate a new record and return a pointer to it after filling it in
// with data
//
// Parameters: none
// Returns: a pointer to a filled-in record; if an error occurred, returns NULL
// Modifies: moves the position of the get pointer
Record * IptableParser::getRecord()
{
	char *instr = new char[INLINE_LENGTH];
	Record *r = new MapRecord();

	// read the next record
	if (streamingInput) 
	{
		cin.getline(instr, INLINE_LENGTH);
	}
	else
	{
		infile.getline(instr, INLINE_LENGTH, '\n');
	}
	
	strncpy(tmpinstr, instr, INLINE_LENGTH);

	// attempt to read the appropriate data fields
	// if a particular field operation fails or the transport layer
	// type is unrecognized, write this packet as an erroneous packet
	// and return NULL
	try
	{
		if (getRecordHeader(instr, r)) throw 0;
		if (getIPv4Header(instr, r)) throw 1;

		// determine the protocol layer type
		if (NULL != strstr(tmpinstr, "PROTO=ICMP"))
		{
			uint8_t *proto = new uint8_t(TRANS_ICMP);
			r->setField("SYS_TRANS_TYPE", Value(proto, VAL_TYPE_UINT8, 1));
			if (getICMPHeader(instr, r)) throw 5;
		}
		else if (NULL != strstr(tmpinstr, "PROTO=TCP"))
		{
			uint8_t *proto = new uint8_t(TRANS_TCP);
			r->setField("SYS_TRANS_TYPE", Value(proto, VAL_TYPE_UINT8, 1));
			if (getTCPHeader(instr, r)) throw 2;
		}
		else if (NULL != strstr(tmpinstr, "PROTO=UDP"))
		{
			uint8_t *proto = new uint8_t(TRANS_UDP);
			r->setField("SYS_TRANS_TYPE", Value(proto, VAL_TYPE_UINT8, 1));
			if (getUDPHeader(instr, r)) throw 4;
		}
		else
		{
			// need to do some fancy manipulations to get the protocol name
			char *protoptr = strstr(tmpinstr, "PROTO=");
			protoptr = strtok(protoptr, " ");
			string *pstr = new string(protoptr, 6, 128);
			r->setField("SYS_TRANS_TYPE_STR",
			            Value(pstr, VAL_TYPE_STRING, 0));
			getGenericData(instr, r);
			// throw 3;
		}
	}
	catch (int e)
	{
		cerr << "libiptable: Erroneous packet encountered in getRecord(error code: " 
			<< e <<  ", record number: " << curSeq << ")" << endl;
		errfile.write(tmpinstr, strlen(instr));
		errfile.write("\n", 1);
		delete[] instr;

		Value v;
		if (0 == r->getField("SYS_TRANS_TYPE", v))
		{
			delete (uint8_t*)(v.Ptr());
		}

		++curSeq;
                delete r;
		return NULL;
	}

	delete[] instr;
	++curSeq;
	return r;
}

// int putRecord(Record * r)
// Unpack and write a record to the output file
//
// Parameters: r, the record to unpack
// Returns: 0 on success
// Modifies: Data is written to the output file
// Notes: This function destroys *r
int IptableParser::putRecord(Record * r)
{
	Value v;

	if (streamingOutput) return putRecordStreaming(r);

	// get transport layer type information
	int transType = TRANS_UNKNOWN;
	if (0 == r->getField("SYS_TRANS_TYPE", v))
	{
		transType = (int)(*(uint8_t*)v.Ptr());
		delete (uint8_t*)v.Ptr();
	}

	// get the system time
	struct tm *timestamp;
	r->getField("PCKT_TS_SEC", v);
	timestamp = localtime((time_t*)((uint32_t*)v.Ptr()));
	delete (uint32_t*)(v.Ptr());

	// unpack the timestamp
	switch (timestamp->tm_mon)
	{
	case 0: outfile << "Jan "; break;
	case 1: outfile << "Feb "; break;
	case 2: outfile << "Mar "; break;
	case 3: outfile << "Apr "; break;
	case 4: outfile << "May "; break;
	case 5: outfile << "Jun "; break;
	case 6: outfile << "Jul "; break;
	case 7: outfile << "Aug "; break;
	case 8: outfile << "Sep "; break;
	case 9: outfile << "Oct "; break;
	case 10: outfile << "Nov "; break;
	case 11: outfile << "Dec "; break;
	default: outfile << "*** "; break;
	}
	
	if (timestamp->tm_mday < 10) { outfile << " "; }
	outfile << timestamp->tm_mday << " ";
	if (timestamp->tm_hour < 10) { outfile << "0"; }
	outfile << timestamp->tm_hour << ":";
	if (timestamp->tm_min < 10) { outfile << "0"; }
	outfile << timestamp->tm_min << ":";
	if (timestamp->tm_sec < 10) { outfile << "0"; }
	outfile << timestamp->tm_sec << " ";

	// machine name
	r->getField("PCKT_MACHINE_NAME", v);
	outfile << *(string*)(v.Ptr()) << " ";
	delete (string*)(v.Ptr());

	// log prefix
	r->getField("PCKT_LOG_PREFIX", v);
	outfile << *(string*)(v.Ptr());
	delete (string*)(v.Ptr());

	// see if there is an input interface value set
	outfile << "IN=";
	if (0 == r->getField("PCKT_IN_INTERFACE", v))
	{
		outfile << *(string*)(v.Ptr());
		delete (string*)(v.Ptr());
	}
	outfile << " ";

	// same thing for the output interface
	outfile << "OUT=";
	if (0 == r->getField("PCKT_OUT_INTERFACE", v))
	{
		outfile << *(string*)(v.Ptr());
		delete (string*)(v.Ptr());
	}
	outfile << " ";

	// print the MAC address if it exists
	if (0 == r->getField("ETHER_DST_MAC", v))
	{
		char *ptr = (char*)v.Ptr();

		// switch to hexadecimal printing and start printing the MAC address
		// destination part
		outfile << "MAC=";
		for (int i = 0; i < 6; i++)
		{
			switch ((uint8_t)(ptr[i]) >> 4)
			{
			case 0: outfile << "0"; break;
			case 1: outfile << "1"; break;
			case 2: outfile << "2"; break;
			case 3: outfile << "3"; break;
			case 4: outfile << "4"; break;
			case 5: outfile << "5"; break;
			case 6: outfile << "6"; break;
			case 7: outfile << "7"; break;
			case 8: outfile << "8"; break;
			case 9: outfile << "9"; break;
			case 10: outfile << "a"; break;
			case 11: outfile << "b"; break;
			case 12: outfile << "c"; break;
			case 13: outfile << "d"; break;
			case 14: outfile << "e"; break;
			case 15: outfile << "f"; break;
			default: outfile << "*"; break;
			}
			switch (ptr[i] & 0x0F)
			{
			case 0: outfile << "0"; break;
			case 1: outfile << "1"; break;
			case 2: outfile << "2"; break;
			case 3: outfile << "3"; break;
			case 4: outfile << "4"; break;
			case 5: outfile << "5"; break;
			case 6: outfile << "6"; break;
			case 7: outfile << "7"; break;
			case 8: outfile << "8"; break;
			case 9: outfile << "9"; break;
			case 10: outfile << "a"; break;
			case 11: outfile << "b"; break;
			case 12: outfile << "c"; break;
			case 13: outfile << "d"; break;
			case 14: outfile << "e"; break;
			case 15: outfile << "f"; break;
			default: outfile << "*"; break;
			}
			outfile << ":";
		}
		delete[] ptr;

		// source part
		r->getField("ETHER_SRC_MAC", v);
		ptr = (char*)v.Ptr();
		for (int i = 0; i < 6; i++)
		{
			switch ((uint8_t)(ptr[i]) >> 4)
			{
			case 0: outfile << "0"; break;
			case 1: outfile << "1"; break;
			case 2: outfile << "2"; break;
			case 3: outfile << "3"; break;
			case 4: outfile << "4"; break;
			case 5: outfile << "5"; break;
			case 6: outfile << "6"; break;
			case 7: outfile << "7"; break;
			case 8: outfile << "8"; break;
			case 9: outfile << "9"; break;
			case 10: outfile << "a"; break;
			case 11: outfile << "b"; break;
			case 12: outfile << "c"; break;
			case 13: outfile << "d"; break;
			case 14: outfile << "e"; break;
			case 15: outfile << "f"; break;
			default: outfile << "*"; break;
			}
			switch(ptr[i] & 0x0F)
			{
			case 0: outfile << "0"; break;
			case 1: outfile << "1"; break;
			case 2: outfile << "2"; break;
			case 3: outfile << "3"; break;
			case 4: outfile << "4"; break;
			case 5: outfile << "5"; break;
			case 6: outfile << "6"; break;
			case 7: outfile << "7"; break;
			case 8: outfile << "8"; break;
			case 9: outfile << "9"; break;
			case 10: outfile << "a"; break;
			case 11: outfile << "b"; break;
			case 12: outfile << "c"; break;
			case 13: outfile << "d"; break;
			case 14: outfile << "e"; break;
			case 15: outfile << "f"; break;
			default: outfile << "*"; break;
			}
			outfile << ":";
		}
		delete[] ptr;

		// network layer type
		r->getField("SYS_NET_TYPE", v);
		outfile << hex <<
		((*(uint16_t*)(v.Ptr()) & 0xF000) >> 12) <<
		((*(uint16_t*)(v.Ptr()) & 0x0F00) >> 8) << ":" <<
		((*(uint16_t*)(v.Ptr()) & 0x00F0) >> 4) <<
		(*(uint16_t*)(v.Ptr()) & 0x000F) << " " << dec;
		delete (uint16_t*)(v.Ptr());

	}

	// IP addresses
	r->getField("IPV4_SRC_IP", v);
	struct in_addr ipaddr;
	ipaddr.s_addr = htonl(*(uint32_t*)(v.Ptr()));
	outfile << "SRC=" << inet_ntoa(ipaddr) << " ";
	delete (uint32_t*)(v.Ptr());

	r->getField("IPV4_DST_IP", v);
	ipaddr.s_addr = htonl(*(uint32_t*)(v.Ptr()));
	outfile << "DST=" << inet_ntoa(ipaddr) << " ";
	delete (uint32_t*)(v.Ptr());

	// IP length
	r->getField("IPV4_LENGTH", v);
	outfile << "LEN=" << *(uint16_t*)(v.Ptr()) << " ";
	delete (uint16_t*)(v.Ptr());

	// IP TOS
	r->getField("IPV4_TOS", v);
	outfile << hex << "TOS=0x" << ((*(uint8_t*)(v.Ptr()) & 0xF0) >> 4)
	<< (*(uint8_t*)(v.Ptr()) & 0x0F) << " ";
	delete (uint8_t*)(v.Ptr());

	// IP precision
	r->getField("IPV4_PRECEDENCE", v);
	outfile << hex << "PREC=0x" << uppercase
	<< ((*(uint8_t*)(v.Ptr()) & 0xF0) >> 4)
	<< (*(uint8_t*)(v.Ptr()) & 0x0F) << nouppercase << " ";
	delete (uint8_t*)(v.Ptr());

	// IP TTL
	r->getField("IPV4_TTL", v);
	outfile << dec << "TTL=" << (uint32_t)*(uint8_t*)(v.Ptr()) << " ";
	delete (uint8_t*)(v.Ptr());

	// IP ID
	r->getField("IPV4_ID", v);
	outfile << "ID=" << *(uint16_t*)(v.Ptr()) << " ";
	delete (uint16_t*)(v.Ptr());

	// various flags, if they exist and were not anonymized
	if (0 == r->getField("IPV4_CE", v) && 1 == *(uint8_t*)(v.Ptr()))
	{
		outfile << "CE ";
		delete (uint8_t*)(v.Ptr());
	}
	if (0 == r->getField("IPV4_DF", v) && 1 == *(uint8_t*)(v.Ptr()))
	{
		outfile << "DF ";
		delete (uint8_t*)(v.Ptr());
	}
	if (0 == r->getField("IPV4_MF", v) && 1 == *(uint8_t*)(v.Ptr()))
	{
		outfile << "MF ";
		delete (uint8_t*)(v.Ptr());
	}
	if (0 == r->getField("IPV4_FRAG", v))
	{
		outfile << "FRAG=" << *(uint16_t*)(v.Ptr()) << " ";
		delete (uint16_t*)(v.Ptr());
	}
	if (0 == r->getField("IPV4_OPT", v))
	{
		outfile << "OPT (";
		for (int i = 0; i < v.Bytes(); i++)
		{
			char *ptr = (char*)(v.Ptr());
			outfile << hex << uppercase << ((*(uint8_t*)(ptr + i) & 0xF0) >> 4)
			<< (*(uint8_t*)(ptr + i) & 0x0F) << nouppercase << dec;
		}
		outfile << ") ";
		delete[] (char*)(v.Ptr());
	}

	// transport layer type
	switch (transType)
	{
	case TRANS_TCP:
		outfile << "PROTO=TCP ";
		putTCPHeader(r);
		break;
	case TRANS_UDP:
		outfile << "PROTO=UDP ";
		putUDPHeader(r);
		break;
	case TRANS_ICMP:
		outfile << "PROTO=ICMP ";
		putICMPHeader(r);
		break;
	case TRANS_UNKNOWN:
		putGenericData(r);
		break;
	default:
                delete r;
		return 1;
		break;
	}
        delete r;
	outfile << endl;
	return 0;
}

// **********************************************
// PRIVATE IptableParser implementation functions
// **********************************************

// void initMonthMap()
// Initialize the internal map used to map months of the
// year to their tm_struct values
//
// Parameters: none
// Returns: nothing
// Modifies: Initializes the utilMonthMap field of the class
void IptableParser::initMonthMap()
{
	utilMonthMap["Jan"] = 0;
	utilMonthMap["Feb"] = 1;
	utilMonthMap["Mar"] = 2;
	utilMonthMap["Apr"] = 3;
	utilMonthMap["May"] = 4;
	utilMonthMap["Jun"] = 5;
	utilMonthMap["Jul"] = 6;
	utilMonthMap["Aug"] = 7;
	utilMonthMap["Sep"] = 8;
	utilMonthMap["Oct"] = 9;
	utilMonthMap["Nov"] = 10;
	utilMonthMap["Dec"] = 11;
	return;
}

// void initHexMap()
// Initialize the internal map used to map hex digits to integers
//
// Parameters: none; returns nothing; initializes utilHexMap
void IptableParser::initHexMap()
{
	utilHexMap['0'] = 0;
	utilHexMap['1'] = 1;
	utilHexMap['2'] = 2;
	utilHexMap['3'] = 3;
	utilHexMap['4'] = 4;
	utilHexMap['5'] = 5;
	utilHexMap['6'] = 6;
	utilHexMap['7'] = 7;
	utilHexMap['8'] = 8;
	utilHexMap['9'] = 9;
	utilHexMap['a'] = 10;
	utilHexMap['b'] = 11;
	utilHexMap['c'] = 12;
	utilHexMap['d'] = 13;
	utilHexMap['e'] = 14;
	utilHexMap['f'] = 15;
	utilHexMap['A'] = 10;
	utilHexMap['B'] = 11;
	utilHexMap['C'] = 12;
	utilHexMap['D'] = 13;
	utilHexMap['E'] = 14;
	utilHexMap['F'] = 15;
	return;
}

// int getRecordHeader(char* const s, Record * r)
// Get the header information for a record
//
// Parameters:
// 	-- s, the text record being processed
// 	-- r, the Record object to store data to
// Returns: 0 on success, nonzero on any type of failure
// Modifies: The get pointer is moved and data is packed into
// 	the record
int IptableParser::getRecordHeader(const char* s, Record * r)
{
	char *buf; // read buffer

	// timestamp
	struct tm theTime;
	string timeString = "";

	// get the first field, which is the month of the year
	buf = strtok((char*)s, " ");
	timeString += (string(buf) + " ");

	// and the date
	buf = strtok(NULL, " ");
	timeString += (string(buf) + " ");

	// now the time of day
	buf = strtok(NULL, " ");
	timeString += (string(buf) + " ");

	// cerr << "timeString = " << timeString << endl;

	// fill in the remaining fields of the time structure
	theTime.tm_year = epochYear; // dummy out the year
	theTime.tm_isdst = -1; //no dst
	theTime.tm_wday = 0;
	theTime.tm_yday = 0;
	theTime.tm_gmtoff = 0; // timezone;
	theTime.tm_zone = 0;
	strptime(timeString.c_str(), "%b %d %T", &theTime);

	// finally, convert the data into a usable integer format
	// and pack it into the record (phew!)
	uint32_t *pckt_ts_sec = new uint32_t((uint32_t)mktime(&theTime));
	r->setField("PCKT_TS_SEC", Value(pckt_ts_sec, VAL_TYPE_UINT32, 4));

	// machine name
	buf = strtok(NULL, " ");
	string *pckt_mch_name = new string(buf);
	r->setField("PCKT_MACHINE_NAME", Value(pckt_mch_name, VAL_TYPE_STRING, 0));

	// quasi-FIXME: due to a bug in netfilter, prefixes specified with the
	// --log-prefix option do not necessarily have a space after them,
	// but this module assumes that they do
	//
	// or at least, this was true in 2001
	string *log_prefix = new string("");
	while (1)
	{
		buf = strtok(NULL, " ");
		if (NULL != strstr(buf, "IN=")) break;
		*log_prefix += (string(buf) + " ");
	}
	r->setField("PCKT_LOG_PREFIX", Value(log_prefix, VAL_TYPE_STRING, 0));

	// now buf contains IN= which is the input interface
	string tmpstr(buf);
	if (tmpstr.length() > 3) // incoming packet with interface data
	{
		string *interface_in = new string(tmpstr, 3, 64);
		r->setField("PCKT_IN_INTERFACE", Value(interface_in,
				VAL_TYPE_STRING, 0));
	}

	// output interface
	buf = strtok(NULL, " ");
	tmpstr = string(buf);
	if (tmpstr.length() > 4)
	{
		string *interface_out = new string(tmpstr, 4, 64);
		r->setField("PCKT_OUT_INTERFACE", Value(interface_out,
				VAL_TYPE_STRING, 0));
	}

	// and, finally, the MAC address, if it exists
	if (NULL != strstr(tmpinstr, "MAC="))
	{
		buf = strtok(NULL, " ");
		char* dstmac = new char[6];
		char* srcmac = new char[6];
		uint16_t* type = new uint16_t;

		// destination portion
		int i;
		uint8_t first, second;
		for (i = 0; i < 6; i++)
		{
			first = utilHexMap[buf[4 + (3 * i)]] << 4;
			second = utilHexMap[buf[5 + (3 * i)]];
			dstmac[i] = first | second;
		}

		// source portion
		for (i = 0; i < 6; i++)
		{
			first = utilHexMap[buf[22 + (3 * i)]] << 4;
			second = utilHexMap[buf[23 + (3 * i)]];
			srcmac[i] = first | second;
		}

		// type portion
		*type = (utilHexMap[buf[40]] << 12 |
		         utilHexMap[buf[41]] << 8 |
		         utilHexMap[buf[43]] << 4 |
		         utilHexMap[buf[44]]);

		r->setField("ETHER_DST_MAC", Value(dstmac, VAL_TYPE_CHAR_PTR, 6));
		r->setField("ETHER_SRC_MAC", Value(srcmac, VAL_TYPE_CHAR_PTR, 6));
		r->setField("SYS_NET_TYPE", Value(type, VAL_TYPE_UINT16, 2));
	}

	return 0;
}

// int getIPv4Header(const char* s, Record * r)
// Get an IPv4 header and pack it into a record
//
// Parameters:
// 	-- s, the log entry to parse
// 	-- r, the record to pack
// Returns: 0 on success, 1 on failure
// Modifies: Moves the get pointer and changes the values in r
int IptableParser::getIPv4Header(const char* s, Record * r)
{
	char *buf; // read buffer
	buf = strtok(NULL, " "); // put SRC= field into buf

	struct in_addr src_ip, dst_ip;
	char *ptr = &buf[4];
 	if (0 == inet_aton(ptr, &src_ip)) return 1;

	buf = strtok(NULL, " ");
	ptr = &buf[4];
	if (0 == inet_aton(ptr, &dst_ip)) return 1;

	uint32_t psrc_ip_static = ntohl(src_ip.s_addr);
	uint32_t pdst_ip_static = ntohl(dst_ip.s_addr);
	uint32_t *psrc_ip = new uint32_t(psrc_ip_static);
	uint32_t *pdst_ip = new uint32_t(pdst_ip_static);

	r->setField("IPV4_SRC_IP", Value(psrc_ip, VAL_TYPE_UINT32, 4));
	r->setField("IPV4_DST_IP", Value(pdst_ip, VAL_TYPE_UINT32, 4));

	// IP datagram length (LEN=)
	buf = strtok(NULL, " ");
	string tmpstr(buf, 4, 128);
	uint16_t *length = new uint16_t((uint16_t)convertToUint32(tmpstr));
	r->setField("IPV4_LENGTH", Value(length, VAL_TYPE_UINT16, 2));

	// IP TOS field (TOS=)
	buf = strtok(NULL, " ");
	uint8_t *tos = new uint8_t((utilHexMap[buf[6]] << 4) | utilHexMap[buf[7]]);
	r->setField("IPV4_TOS", Value(tos, VAL_TYPE_UINT8, 1));

	// IP precision field (PREC=)
	buf = strtok(NULL, " ");
	uint8_t *prc = new uint8_t((utilHexMap[buf[7]] << 4) | utilHexMap[buf[8]]);
	r->setField("IPV4_PRECEDENCE", Value(prc, VAL_TYPE_UINT8, 1));

	// IP TTL field (TTL=)
	buf = strtok(NULL, " ");
	tmpstr = string(buf, 4, 128);
	uint8_t *ttl = new uint8_t(convertToUint8(tmpstr));
	r->setField("IPV4_TTL", Value(ttl, VAL_TYPE_UINT8, 1));

	// IP ID field (ID=)
	buf = strtok(NULL, " ");
	tmpstr = string(buf, 3, 128);
	uint16_t *id = new uint16_t((uint16_t)convertToUint32(tmpstr));
	r->setField("IPV4_ID", Value(id, VAL_TYPE_UINT16, 2));

	// the next few fields are variable flags, so we use a while loop
	// to wait until we hit the PROTO= field
	buf = strtok(NULL, " ");
	while (NULL == strstr(buf, "PROTO="))
	{
		if (0 == strcmp(buf, "CE"))
		{
			uint8_t *ce = new uint8_t(1);
			r->setField("IPV4_CE", Value(ce, VAL_TYPE_UINT8, 1));
		}
		else if (0 == strcmp(buf, "DF"))
		{
			uint8_t *df = new uint8_t(1);
			r->setField("IPV4_DF", Value(df, VAL_TYPE_UINT8, 1));
		}
		else if (0 == strcmp(buf, "MF"))
		{
			uint8_t *mf = new uint8_t(1);
			r->setField("IPV4_MF", Value(mf, VAL_TYPE_UINT8, 1));
		}
		else if (NULL != strstr(buf, "FRAG="))
		{
			tmpstr = string(buf, 5, 128);
			uint16_t *frag = new uint16_t((uint16_t)convertToUint32(tmpstr));
			r->setField("IPV4_FRAG", Value(frag, VAL_TYPE_UINT16, 2));
		}
		else if (0 == strcmp(buf, "OPT"))
		{
			buf = strtok(NULL, " ");
			char *opt = new char[(strlen(buf) / 2) - 1];
			for(int i = 0; buf[2 * i + 1] != ')'; i++)
			{
				opt[i] =
				        ((buf[2 * i + 1] - '0') << 4) | (buf[2 * i + 2] - '0');
			}
			r->setField("IPV4_OPT", Value(opt, VAL_TYPE_CHAR_PTR,
			                              (strlen(buf) / 2) - 1));
		}

		buf = strtok(NULL, " ");
	}

	// once we hit the PROTO= field we're done as this field is analyzed by
	// the caller
	return 0;
}

// int getTCPHeader(const char* s, record* r)
// Get the TCP-related fields from a record and pack them
//
// Parameters: s, the input string; r, the record to pack
// Returns: 0 on success, 1 on failure
// Modifies: Moves the get pointer and modifies the record
int IptableParser::getTCPHeader(const char* s, Record *r)
{
	char *buf;
	string tmpstr;

	// the last thing we read was the PROTO= field, so...
	// source port
	buf = strtok(NULL, " ");

	if (strcmp(buf, "INCOMPLETE") == 0)
	{
		buf = strtok(NULL, " ");
		tmpstr = string(buf, 1, 128);
		uint32_t *incomplete = new uint32_t(convertToUint32(tmpstr));
		r->setField("TCP_INCOMPLETE", Value(incomplete, VAL_TYPE_UINT32, 4));
		return 0;
	}

	if (NULL != strstr(buf, "SPT="))
	{
		tmpstr = string(buf, 4, 128);
		uint16_t *srcprt = new uint16_t((uint16_t)convertToUint32(tmpstr));
		r->setField("TCP_SRC_PORT", Value(srcprt, VAL_TYPE_UINT16, 2));

		// destination port
		buf = strtok(NULL, " ");
	}

	if (NULL != strstr(buf, "DPT="))
	{
		tmpstr = string(buf, 4, 128);
		uint16_t *dstprt = new uint16_t((uint16_t)convertToUint32(tmpstr));
		r->setField("TCP_DST_PORT", Value(dstprt, VAL_TYPE_UINT16, 2));

		// sequence number
		buf = strtok(NULL, " ");
	}

	if (NULL != strstr(buf, "SEQ="))
	{
		tmpstr = string(buf, 4, 128);
		uint32_t *seq = new uint32_t(convertToUint32(tmpstr));
		r->setField("TCP_SEQUENCE", Value(seq, VAL_TYPE_UINT32, 4));

		// ack number
		buf = strtok(NULL, " ");
	}

	if (NULL != strstr(buf, "ACK="))
	{
		tmpstr = string(buf, 4, 128);
		uint32_t *ack_no = new uint32_t(convertToUint32(tmpstr));
		r->setField("TCP_ACK_NO", Value(ack_no, VAL_TYPE_UINT32, 4));

		// window number
		buf = strtok(NULL, " ");
	}

	if (NULL != strstr(buf, "WINDOW="))
	{
		tmpstr = string(buf, 7, 128);
		uint16_t *win = new uint16_t((uint16_t)convertToUint32(tmpstr));
		r->setField("TCP_WINDOW", Value(win, VAL_TYPE_UINT16, 2));

		// reserved bits
		buf = strtok(NULL, " ");
	}

	if (NULL != strstr(buf, "RES="))
	{
		uint8_t *rbits = new uint8_t(((buf[6] - '0') << 4) | (buf[7] - '0'));
		r->setField("TCP_RESERVED", Value(rbits, VAL_TYPE_UINT8, 1));

		// the next few fields are optional flags
		buf = strtok(NULL, " ");
	}

	while (NULL != buf && NULL == strstr(buf, "URGP=")
	                && NULL == strstr(buf, "OPT"))
	{
		if (0 == strcmp(buf, "URG"))
		{
			uint8_t *urgent = new uint8_t(1);
			r->setField("TCP_FLAG_URGENT", Value(urgent, VAL_TYPE_UINT8, 1));
		}
		else if (0 == strcmp(buf, "ACK"))
		{
			uint8_t *ack = new uint8_t(1);
			r->setField("TCP_FLAG_ACK", Value(ack, VAL_TYPE_UINT8, 1));
		}
		else if (0 == strcmp(buf, "PSH"))
		{
			uint8_t *psh = new uint8_t(1);
			r->setField("TCP_FLAG_PSH", Value(psh, VAL_TYPE_UINT8, 1));
		}
		else if (0 == strcmp(buf, "RST"))
		{
			uint8_t *rst = new uint8_t(1);
			r->setField("TCP_FLAG_RST", Value(rst, VAL_TYPE_UINT8, 1));
		}
		else if (0 == strcmp(buf, "SYN"))
		{
			uint8_t *syn = new uint8_t(1);
			r->setField("TCP_FLAG_SYN", Value(syn, VAL_TYPE_UINT8, 1));
		}
		else if (0 == strcmp(buf, "FIN"))
		{
			uint8_t *fin = new uint8_t(1);
			r->setField("TCP_FLAG_FIN", Value(fin, VAL_TYPE_UINT8, 1));
		}

		buf = strtok(NULL, " ");
	} // while(NULL == strstr(buf, "URGP="))

	if (NULL == buf) return 0; // ran out of information on the line

	if (NULL != strstr(buf, "URGP="))
	{
		tmpstr = string(buf, 5, 128);
		uint32_t *urgp = new uint32_t(convertToUint32(tmpstr));
		r->setField("TCP_URGENT_PTR", Value(urgp, VAL_TYPE_UINT32, 4));
	}

	// see if options are present
	if (0 == strcmp(buf, "OPT") ||
	                (NULL != (buf = strtok(NULL, " ")) && 0 == strcmp(buf, "OPT")))
	{
		buf = strtok(NULL, " ");
		char *tcpopt = new char[(strlen(buf) / 2) - 1];
		for (int i = 0; buf[2 * i + 1] != ')'; i++)
		{
			tcpopt[i] =
			        (utilHexMap[buf[2 * i + 1]] << 4) |
			        (utilHexMap[buf[2 * i + 2]]);
		}
		r->setField("TCP_OPTIONS", Value(tcpopt, VAL_TYPE_CHAR_PTR,
		                                 (strlen(buf) / 2) - 1));
	}

	return 0;
}

// int getUDPHeader(const char* s, Record * r)
// Pack the UDP portion of a record
//
// Parameters: r, the record to pack; s, the string form of the record
// Returns: 0 on success, 1 on failure
// Modifies: Moves the get pointer and adds data to the record
int IptableParser::getUDPHeader(const char* s, Record * r)
{
	char *buf;
	string tmpstr;

	buf = strtok(NULL, " ");
	if (NULL != strstr(buf, "SPT="))
	{
		tmpstr = string(buf, 4, 128);
		uint16_t *srcprt = new uint16_t((uint16_t)convertToUint32(tmpstr));
		r->setField("UDP_SRC_PORT", Value(srcprt, VAL_TYPE_UINT16, 2));

		buf = strtok(NULL, " ");
	}

	if (NULL != strstr(buf, "DPT="))
	{
		tmpstr = string(buf, 4, 128);
		uint16_t *dstprt = new uint16_t((uint16_t)convertToUint32(tmpstr));
		r->setField("UDP_DST_PORT", Value(dstprt, VAL_TYPE_UINT16, 2));

		buf = strtok(NULL, " ");
	}

	if (NULL != strstr(buf, "LEN="))
	{
		tmpstr = string(buf, 4, 128);
		uint16_t *udplen = new uint16_t((uint16_t)convertToUint32(tmpstr));
		r->setField("UDP_LENGTH", Value(udplen, VAL_TYPE_UINT16, 2));
	}

	return 0;
}

// int getICMPHeader(const char* s, Record * r)
// Get the ICMP-related fields of a record
//
// Parameters: s, the record being read from; r, the Record to pack
// Returns: 0 on success, 1 on failure
// Modifies: moves the get pointer and adds data to r
int IptableParser::getICMPHeader(const char* s, Record * r)
{
	char *buf;
	string tmpstr;

	buf = strtok(NULL, " ");
	// check to make sure the rest of the packet is intact
	if (0 == strcmp(buf, "INCOMPLETE"))
	{
		buf = strtok(NULL, " ");
		tmpstr = string(buf, 1, 128);
		uint32_t *inc = new uint32_t(convertToUint32(tmpstr));
		r->setField("ICMP_TOTAL_INCOMPLETE", Value(inc, VAL_TYPE_UINT32, 4));
		return 0;
	}

	// first item after PROTO=ICMP is type
	if (0 == strstr(buf, "TYPE="))
	{
		tmpstr = string(buf, 5, 128);
		uint8_t *type = new uint8_t(convertToUint8(tmpstr));
		r->setField("ICMP_TYPE", Value(type, VAL_TYPE_UINT8, 1));

		// code
		buf = strtok(NULL, " ");
	}

	if (0 == strstr(buf, "CODE="))
	{
		tmpstr = string(buf, 5, 128);
		uint8_t *code = new uint8_t(convertToUint8(tmpstr));
		r->setField("ICMP_CODE", Value(code, VAL_TYPE_UINT8, 1));

		// at this point there are three possibilities for what the next
		// call to strtok will yield:
		// -- INCOMPLETE [## bytes]
		// -- ID=##### SEQ=#####
		// -- [SRC=###.###.###.### ...
		// the only way to find out is to do the strtok and ask questions
		buf = strtok(NULL, " ");
	}

	if (buf[0] == 'I') // one of the first two
	{
		if (buf[1] == 'N') // INCOMPLETE
		{
			buf = strtok(NULL, " ");
			tmpstr = string(buf, 1, 128);
			uint32_t *inc = new uint32_t(convertToUint32(tmpstr));
			r->setField("ICMP_INCOMPLETE", Value(inc, VAL_TYPE_UINT32, 4));
			return 0;
		}
		else if (buf[1] == 'D') // ID
		{
			tmpstr = string(buf, 3, 128);
			uint32_t *id = new uint32_t(convertToUint32(tmpstr));
			r->setField("ICMP_EXT_ID", Value(id, VAL_TYPE_UINT32, 4));

			buf = strtok(NULL, " ");
			tmpstr = string(buf, 4, 128);
			uint32_t *seq = new uint32_t(convertToUint32(tmpstr));
			r->setField("ICMP_EXT_SEQ", Value(seq, VAL_TYPE_UINT32, 4));

			return 0;
		}
		else // didn't recognize, something bad happened
		{
			return 1;
		}
	}
	else if (buf[0] == '[') // encapsulated data
	{
		struct in_addr src_ip, dst_ip;
		char *ptr = &buf[5];
		if (0 == inet_aton(ptr, &src_ip)) return 1;

		buf = strtok(NULL, " ");
		ptr = &buf[4];
 		if (0 == inet_aton(ptr, &dst_ip)) return 1;

		uint32_t psrc_ip_s = ntohl(src_ip.s_addr);
		uint32_t pdst_ip_s = ntohl(dst_ip.s_addr);
		uint32_t *psrc_ip = new uint32_t(psrc_ip_s);
		uint32_t *pdst_ip = new uint32_t(pdst_ip_s);

		r->setField("ICMP_SRC_IP", Value(psrc_ip, VAL_TYPE_UINT32, 4));
		r->setField("ICMP_DST_IP", Value(pdst_ip, VAL_TYPE_UINT32, 4));

		// IP datagram length (LEN=)
		buf = strtok(NULL, " ");
		if (NULL != strstr(buf, "LEN="))
		{
			string tmpstr(buf, 4, 128);
			uint16_t *length = new uint16_t((uint16_t)convertToUint32(tmpstr));
			r->setField("ICMP_LENGTH", Value(length, VAL_TYPE_UINT16, 2));

			// IP TOS field (TOS=)
			buf = strtok(NULL, " ");
		}

		if (NULL != strstr(buf, "TOS="))
		{
			uint8_t *tos = new uint8_t((utilHexMap[buf[6]] << 4)
			                           | utilHexMap[buf[7]]);
			r->setField("ICMP_TOS", Value(tos, VAL_TYPE_UINT8, 1));

			// IP precision field (PREC=)
			buf = strtok(NULL, " ");
		}

		if (NULL != strstr(buf, "PREC="))
		{
			uint8_t *prc = new uint8_t((utilHexMap[buf[7]] << 4)
			                           | utilHexMap[buf[8]]);
			r->setField("ICMP_PRECEDENCE", Value(prc, VAL_TYPE_UINT8, 1));

			// IP TTL field (TTL=)
			buf = strtok(NULL, " ");
		}

		if (NULL != strstr(buf, "TTL="))
		{
			tmpstr = string(buf, 4, 128);
			uint8_t *ttl = new uint8_t(convertToUint8(tmpstr));
			r->setField("ICMP_TTL", Value(ttl, VAL_TYPE_UINT8, 1));

			// IP ID field (ID=)
			buf = strtok(NULL, " ");
		}

		if (NULL != strstr(buf, "ID="))
		{
			tmpstr = string(buf, 3, 128);
			uint16_t *id = new uint16_t((uint16_t)convertToUint32(tmpstr));
			r->setField("ICMP_ID", Value(id, VAL_TYPE_UINT16, 2));

			// the next few fields are variable flags, so we use a while loop gmtime_r((time_t*)((uint32_t*)(v.Ptr())), &timestamp);
			// to wait until we hit the PROTO= field
			buf = strtok(NULL, " ");
		}

		while (NULL == strstr(buf, "PROTO="))
		{
			if (0 == strcmp(buf, "CE"))
			{
				uint8_t *ce = new uint8_t(1);
				r->setField("ICMP_CE", Value(ce, VAL_TYPE_UINT8, 1));
			}
			else if (0 == strcmp(buf, "DF"))
			{
				uint8_t *df = new uint8_t(1);
				r->setField("ICMP_DF", Value(df, VAL_TYPE_UINT8, 1));
			}
			else if (0 == strcmp(buf, "MF"))
			{
				uint8_t *mf = new uint8_t(1);
				r->setField("ICMP_MF", Value(mf, VAL_TYPE_UINT8, 1));
			}
			else if (NULL != strstr(buf, "FRAG="))
			{
				tmpstr = string(buf, 5, 128);
				uint16_t *frag = new uint16_t(
				                         (uint16_t)convertToUint32(tmpstr));
				r->setField("ICMP_FRAG", Value(frag, VAL_TYPE_UINT16, 2));
			}
			else if (0 == strcmp(buf, "OPT"))
			{
				buf = strtok(NULL, " ");
				char *opt = new char[(strlen(buf) / 2) - 1];
				for(int i = 0; buf[2 * i + 1] != ')'; i++)
				{
					opt[i] =
					        ((buf[2 * i + 1] - '0') << 4) | (buf[2 * i + 2] - '0');
				}
				r->setField("ICMP_OPT", Value(opt, VAL_TYPE_CHAR_PTR,
				                              (strlen(buf) / 2) - 1));
			}

			buf = strtok(NULL, " ");
		} // while ... PROTO=

		// now at PROTO= field
		tmpstr = string(buf, 6, 128);
		if (tmpstr == "TCP")
		{
			uint8_t *icmpt = new uint8_t(TRANS_TCP);
			r->setField("ICMP_TRANS_TYPE", Value(icmpt, VAL_TYPE_UINT8, 1));

			buf = strtok(NULL, " ");

			if (strcmp(buf, "INCOMPLETE") == 0)
			{
				buf = strtok(NULL, " ");
				tmpstr = string(buf, 1, 128);
				uint32_t *incomplete = new uint32_t(convertToUint32(tmpstr));
				r->setField("ICMP_TCP_INCOMPLETE",
				            Value(incomplete, VAL_TYPE_UINT32, 4));
				return 0;
			}

			tmpstr = string(buf, 4, 128);
			uint16_t *srcprt = new uint16_t((uint16_t)convertToUint32(tmpstr));
			r->setField("ICMP_TCP_SRC_PORT",
			            Value(srcprt, VAL_TYPE_UINT16, 2));

			// destination port
			buf = strtok(NULL, " ");
			tmpstr = string(buf, 4, 128);
			uint16_t *dstprt = new uint16_t((uint16_t)convertToUint32(tmpstr));
			r->setField("ICMP_TCP_DST_PORT",
			            Value(dstprt, VAL_TYPE_UINT16, 2));

			// sequence number
			buf = strtok(NULL, " ");
			tmpstr = string(buf, 4, 128);
			uint32_t *seq = new uint32_t(convertToUint32(tmpstr));
			r->setField("ICMP_TCP_SEQUENCE",
			            Value(seq, VAL_TYPE_UINT32, 4));

			// ack number
			buf = strtok(NULL, " ");
			tmpstr = string(buf, 4, 128);
			uint32_t *ack_no = new uint32_t(convertToUint32(tmpstr));
			r->setField("ICMP_TCP_ACK_NO",
			            Value(ack_no, VAL_TYPE_UINT32, 4));

			// window number
			buf = strtok(NULL, " ");
			tmpstr = string(buf, 7, 128);
			uint16_t *win = new uint16_t((uint16_t)convertToUint32(tmpstr));
			r->setField("ICMP_TCP_WINDOW", Value(win, VAL_TYPE_UINT16, 2));

			// reserved bits
			buf = strtok(NULL, " ");
			uint8_t *rbits = new uint8_t((
			                                     (buf[6] - '0') << 4) | (buf[7] - '0'));
			r->setField("ICMP_TCP_RESERVED", Value(rbits, VAL_TYPE_UINT8, 1));

			// the next few fields are optional flags
			buf = strtok(NULL, " ");
			while (NULL == strstr(buf, "URGP="))
			{
				if (0 == strcmp(buf, "URG"))
				{
					uint8_t *urgent = new uint8_t(1);
					r->setField("ICMP_TCP_FLAG_URGENT",
					            Value(urgent, VAL_TYPE_UINT8, 1));
				}
				else if (0 == strcmp(buf, "ACK"))
				{
					uint8_t *ack = new uint8_t(1);
					r->setField("ICMP_TCP_FLAG_ACK",
					            Value(ack, VAL_TYPE_UINT8, 1));
				}
				else if (0 == strcmp(buf, "PSH"))
				{
					uint8_t *psh = new uint8_t(1);
					r->setField("ICMP_TCP_FLAG_PSH",
					            Value(psh, VAL_TYPE_UINT8, 1));
				}
				else if (0 == strcmp(buf, "RST"))
				{
					uint8_t *rst = new uint8_t(1);
					r->setField("ICMP_TCP_FLAG_RST",
					            Value(rst, VAL_TYPE_UINT8, 1));
				}
				else if (0 == strcmp(buf, "SYN"))
				{
					uint8_t *syn = new uint8_t(1);
					r->setField("ICMP_TCP_FLAG_SYN",
					            Value(syn, VAL_TYPE_UINT8, 1));
				}
				else if (0 == strcmp(buf, "FIN"))
				{
					uint8_t *fin = new uint8_t(1);
					r->setField("ICMP_TCP_FLAG_FIN",
					            Value(fin, VAL_TYPE_UINT8, 1));
				}

				buf = strtok(NULL, " ");
			} // while(NULL == strstr(buf, "URGP="))

			tmpstr = string(buf, 5, 128);
			uint32_t *urgp = new uint32_t(convertToUint32(tmpstr));
			r->setField("ICMP_TCP_URGENT_PTR",
			            Value(urgp, VAL_TYPE_UINT32, 4));

			// see if options are present
			if (NULL != (buf = strtok(NULL, " ")) && 0 == strcmp(buf, "OPT"))
			{
				buf = strtok(NULL, " ");
				char *tcpopt = new char[(strlen(buf) / 2) - 1];
				for (int i = 0; buf[2 * i + 1] != ')'; i++)
				{
					tcpopt[i] =
					        (utilHexMap[buf[2 * i + 1]] << 4) |
					        (utilHexMap[buf[2 * i + 2]]);
				}
				r->setField("ICMP_TCP_OPTIONS",
				            Value(tcpopt, VAL_TYPE_CHAR_PTR,
				                  (strlen(buf) / 2) - 1));
			}

			return 0;
		} // if (tmpstr == TCP)
		else if (tmpstr == "UDP")
		{
			uint8_t *t = new uint8_t(TRANS_UDP);
			r->setField("ICMP_TRANS_TYPE", Value(t, VAL_TYPE_UINT8, 1));

			buf = strtok(NULL, " ");
			tmpstr = string(buf, 4, 128);
			uint16_t *srcprt = new uint16_t((uint16_t)convertToUint32(tmpstr));
			r->setField("ICMP_UDP_SRC_PORT", Value(srcprt, VAL_TYPE_UINT16, 2));

			buf = strtok(NULL, " ");
			tmpstr = string(buf, 4, 128);
			uint16_t *dstprt = new uint16_t((uint16_t)convertToUint32(tmpstr));
			r->setField("ICMP_UDP_DST_PORT", Value(dstprt, VAL_TYPE_UINT16, 2));

			buf = strtok(NULL, " ");
			tmpstr = string(buf, 4, 128);
			uint16_t *udplen = new uint16_t((uint16_t)convertToUint32(tmpstr));
			r->setField("ICMP_UDP_LENGTH", Value(udplen, VAL_TYPE_UINT16, 2));

			return 0;
		} // if (tmpstr == UDP)
		else if (tmpstr == "ICMP")
		{
			uint8_t *t = new uint8_t(TRANS_ICMP);
			r->setField("ICMP_TRANS_TYPE", Value(t, VAL_TYPE_UINT8, 1));

			buf = strtok(NULL, " ");
			tmpstr = string(buf, 5, 128);
			uint8_t *typei = new uint8_t(convertToUint8(tmpstr));
			r->setField("ICMP_TYPE_INTERNAL", Value(typei,
			                                        VAL_TYPE_UINT8, 1));

			buf = strtok(NULL, " ");
			tmpstr = string(buf, 5, 128);
			uint8_t *codei = new uint8_t(convertToUint8(tmpstr));
			r->setField("ICMP_CODE_INTERNAL", Value(codei,
			                                        VAL_TYPE_UINT8, 1));

			buf = strtok(NULL, " ");
			tmpstr = string(buf, 3, 128);
			uint32_t *idi = new uint32_t(convertToUint32(tmpstr));
			r->setField("ICMP_ID_INTERNAL", Value(idi,
			                                      VAL_TYPE_UINT32, 4));

			buf = strtok(NULL, " ");
			tmpstr = string(buf, 4, 128);
			uint32_t *seqi = new uint32_t(convertToUint32(tmpstr));
			r->setField("ICMP_SEQ_INTERNAL", Value(seqi,
			                                       VAL_TYPE_UINT32, 4));
		}
		else // not TCP or UDP
		{
			return 1;
		}
	} // else if (buf[0] == '[')
	else
	{
		return 1;
	}
	
	return 0;
}

// int getGenericData(const char * s, Record * r)
// Get data belonging to a log type not otherwise recognized
//
// Parameters: s, the record being read; r, the Record to pack
// Returns: 0 on success, 1 on failure
// Modifies: Data is added to r and the get pointer is moved
int IptableParser::getGenericData(const char * s, Record * r)
{
	// read the rest of the data available
	string *tmpstr = new string("");
	char *buf;

	while (NULL != (buf = strtok(NULL, " ")))
	{
		*tmpstr += (string(buf) + " ");
	}

	r->setField("SYS_GENERIC_DATA", Value(tmpstr, VAL_TYPE_STRING, 0));

	return 0;
}

// int putTCPHeader(Record * r)
// Unpack the TCP portion of a record and print it
//
// Parameters: r, the record to unpack
// Returns: 0 on success, 1 on failure
// Modifies: Data is removed from r and written to the output file
int IptableParser::putTCPHeader(Record * r)
{
	Value v;

	if (r->getField("TCP_INCOMPLETE", v) == 0)
	{
		outfile << "INCOMPLETE [" << *(uint32_t*)(v.Ptr()) << " bytes] ";
		delete (uint32_t*)(v.Ptr());
		return 0;
	}

	if (0 == r->getField("TCP_SRC_PORT", v))
	{
		outfile << "SPT=" << *(uint16_t*)(v.Ptr()) << " ";
		delete (uint16_t*)(v.Ptr());
	}

	if (0 == r->getField("TCP_DST_PORT", v))
	{
		outfile << "DPT=" << *(uint16_t*)(v.Ptr()) << " ";
		delete (uint16_t*)(v.Ptr());
	}

	if (0 == r->getField("TCP_SEQUENCE", v))
	{
		outfile << "SEQ=" << *(uint32_t*)(v.Ptr()) << " ";
		delete (uint32_t*)(v.Ptr());
	}

	if (0 == r->getField("TCP_ACK_NO", v))
	{
		outfile << "ACK=" << *(uint32_t*)(v.Ptr()) << " ";
		delete (uint32_t*)(v.Ptr());
	}

	if (0 == r->getField("TCP_WINDOW", v))
	{
		outfile << "WINDOW=" << *(uint16_t*)(v.Ptr()) << " ";
		delete (uint16_t*)(v.Ptr());
	}

	if (0 == r->getField("TCP_RESERVED", v))
	{
		outfile << "RES=0x" << hex << (uint32_t)(*(uint8_t*)(v.Ptr()) >> 4)
		<< (uint32_t)(*(uint8_t*)(v.Ptr()) & 0x0F) << " " << dec;
		delete (uint8_t*)(v.Ptr());
	}

	// optional flags
	if (0 == r->getField("TCP_FLAG_URGENT", v) && 1 == *(uint8_t*)(v.Ptr()))
	{
		outfile << "URG ";
		delete (uint8_t*)(v.Ptr());
	}
	if (0 == r->getField("TCP_FLAG_ACK", v) && 1 == *(uint8_t*)(v.Ptr()))
	{
		outfile << "ACK ";
		delete (uint8_t*)(v.Ptr());
	}
	if (0 == r->getField("TCP_FLAG_PSH", v) && 1 == *(uint8_t*)(v.Ptr()))
	{
		outfile << "PSH ";
		delete (uint8_t*)(v.Ptr());
	}
	if (0 == r->getField("TCP_FLAG_RST", v) && 1 == *(uint8_t*)(v.Ptr()))
	{
		outfile << "RST ";
		delete (uint8_t*)(v.Ptr());
	}
	if (0 == r->getField("TCP_FLAG_SYN", v) && 1 == *(uint8_t*)(v.Ptr()))
	{
		outfile << "SYN ";
		delete (uint8_t*)(v.Ptr());
	}
	if (0 == r->getField("TCP_FLAG_FIN", v) && 1 == *(uint8_t*)(v.Ptr()))
	{
		outfile << "FIN ";
		delete (uint8_t*)(v.Ptr());
	}

	// back to our normal programming
	if (0 == r->getField("TCP_URGENT_PTR", v))
	{
		outfile << "URGP=" << *(uint32_t*)(v.Ptr()) << " ";
		delete (uint32_t*)(v.Ptr());
	}

	if (0 == r->getField("TCP_OPTIONS", v))
	{
		outfile << "OPT (";
		for(int i = 0; i < v.Bytes(); i++)
		{
			char *ptr = (char*)(v.Ptr());
			outfile << hex << uppercase << ((*(uint8_t*)(ptr + i) & 0xF0) >> 4)
			<< (*(uint8_t*)(ptr + i) & 0x0F) << nouppercase << dec;
		}
		outfile << ") ";
		delete[] (char*)(v.Ptr());
	}

	return 0;
}

// int putUDPHeader(Record * r)
// Puts a UDP header back to the output file
//
// Parameters: r, the record to unpack and dump
// Returns: 0 on success, 1 on failure
// Modifies: Puts data to the file and deletes data from r
int IptableParser::putUDPHeader(Record * r)
{
	Value v;

	if (0 == r->getField("UDP_SRC_PORT", v))
	{
		outfile << dec << "SPT=" << *(uint16_t*)(v.Ptr()) << " ";
		delete (uint16_t*)(v.Ptr());
	}

	if (0 == r->getField("UDP_DST_PORT", v))
	{
		outfile << "DPT=" << *(uint16_t*)(v.Ptr()) << " ";
		delete (uint16_t*)(v.Ptr());
	}

	if (0 == r->getField("UDP_LENGTH", v))
	{
		outfile << "LEN=" << *(uint16_t*)(v.Ptr()) << " ";
		delete (uint16_t*)(v.Ptr());
	}

	return 0;
}

// int putICMPHeader(Record * r)
// Unpack and output an ICMP header
//
// Parameters: r, the record to unpack
// Returns: 0 on success, 1 on failure
// Modifies: data is written to the output file and r has data destroyed
int IptableParser::putICMPHeader(Record * r)
{
	Value v;

	if (0 == r->getField("ICMP_TOTAL_INCOMPLETE", v))
	{
		outfile << "INCOMPLETE [" << *(uint32_t*)(v.Ptr())
		<< " bytes] ";
		delete (uint32_t*)(v.Ptr());
		return 0;
	}

	if (0 == r->getField("ICMP_TYPE", v))
	{
		outfile << "TYPE=" << (uint32_t)(*(uint8_t*)(v.Ptr())) << " ";
		delete (uint8_t*)(v.Ptr());
	}

	if (0 == r->getField("ICMP_CODE", v))
	{
		outfile << "CODE=" << (uint32_t)(*(uint8_t*)(v.Ptr())) << " ";
		delete (uint8_t*)(v.Ptr());
	}

	// incomplete
	if (0 == r->getField("ICMP_INCOMPLETE", v))
	{
		outfile << "INCOMPLETE [" << *(uint32_t*)(v.Ptr()) << " bytes] ";
		delete (uint32_t*)(v.Ptr());
		return 0;
	}

	// external ID and SEQ fields but no additional data
	else if (0 == r->getField("ICMP_EXT_ID", v))
	{
		outfile << "ID=" << *(uint32_t*)(v.Ptr()) << " ";
		delete (uint32_t*)(v.Ptr());

		r->getField("ICMP_EXT_SEQ", v);
		outfile << "SEQ=" << *(uint32_t*)(v.Ptr()) << " ";
		delete (uint32_t*)(v.Ptr());
	}
	else if (0 == r->getField("ICMP_SRC_IP", v))
	{
		if (0 == r->getField("ICMP_SRC_IP", v))
		{
			struct in_addr ipaddr;
			ipaddr.s_addr = htonl(*(uint32_t*)(v.Ptr()));
			outfile << "[SRC=" << inet_ntoa(ipaddr) << " ";
			delete (uint32_t*)(v.Ptr());
		}

		if (0 == r->getField("ICMP_DST_IP", v))
		{
			struct in_addr ipaddr;
			ipaddr.s_addr = htonl(*(uint32_t*)(v.Ptr()));
			outfile << "DST=" << inet_ntoa(ipaddr) << " ";
			delete (uint32_t*)(v.Ptr());
		}

		// IP length
		if (0 == r->getField("ICMP_LENGTH", v))
		{
			outfile << "LEN=" << *(uint16_t*)(v.Ptr()) << " ";
			delete (uint16_t*)(v.Ptr());
		}

		// IP TOS
		if (0 == r->getField("ICMP_TOS", v))
		{
			outfile << hex << "TOS=0x" << ((*(uint8_t*)(v.Ptr()) & 0xF0) >> 4)
			<< (*(uint8_t*)(v.Ptr()) & 0x0F) << " ";
			delete (uint8_t*)(v.Ptr());
		}

		// IP precision
		if (0 == r->getField("ICMP_PRECEDENCE", v))
		{
			outfile << hex <<
			uppercase <<
			"PREC=0x" << ((*(uint8_t*)(v.Ptr()) & 0xF0) >> 4)
			<< (*(uint8_t*)(v.Ptr()) & 0x0F) << nouppercase << " ";
			delete (uint8_t*)(v.Ptr());
		}

		// IP TTL
		if (0 == r->getField("ICMP_TTL", v))
		{
			outfile << dec << "TTL=" << (uint32_t)*(uint8_t*)(v.Ptr()) << " ";
			delete (uint8_t*)(v.Ptr());
		}

		// IP ID
		if (0 == r->getField("ICMP_ID", v))
		{
			outfile << "ID=" << *(uint16_t*)(v.Ptr()) << " ";
			delete (uint16_t*)(v.Ptr());
		}

		// various flags, if they exist and were not anonymized
		if (0 == r->getField("ICMP_CE", v) && 1 == *(uint8_t*)(v.Ptr()))
		{
			outfile << "CE ";
			delete (uint8_t*)(v.Ptr());
		}
		if (0 == r->getField("ICMP_DF", v) && 1 == *(uint8_t*)(v.Ptr()))
		{
			outfile << "DF ";
			delete (uint8_t*)(v.Ptr());
		}
		if (0 == r->getField("ICMP_MF", v) && 1 == *(uint8_t*)(v.Ptr()))
		{
			outfile << "MF ";
			delete (uint8_t*)(v.Ptr());
		}
		if (0 == r->getField("ICMP_FRAG", v))
		{
			outfile << "FRAG=" << *(uint16_t*)(v.Ptr()) << " ";
			delete (uint16_t*)(v.Ptr());
		}
		if (0 == r->getField("ICMP_OPT", v))
		{
			outfile << "OPT (";
			for(int i = 0; i < v.Bytes(); i++)
			{
				char *ptr = (char*)(v.Ptr());
				outfile << hex << uppercase <<
				((*(uint8_t*)(ptr + i) & 0xF0) >> 4)
				<< (*(uint8_t*)(ptr + i) & 0x0F) << nouppercase << dec;
			}
			outfile << ") ";
			delete[] (char*)(v.Ptr());
		}

		// determine the protocol type
		r->getField("ICMP_TRANS_TYPE", v);
		uint8_t ttype = *(uint8_t*)(v.Ptr());
		delete (uint8_t*)(v.Ptr());

		if (ttype == TRANS_TCP)
		{
			outfile << "PROTO=TCP ";

			if (r->getField("ICMP_TCP_INCOMPLETE", v) == 0)
			{
				outfile << "INCOMPLETE [" <<
				*(uint32_t*)(v.Ptr()) << " bytes] ";
				delete (uint32_t*)(v.Ptr());
			}
			else
			{

				if (0 == r->getField("ICMP_TCP_SRC_PORT", v))
				{
					outfile << "SPT=" << *(uint16_t*)(v.Ptr()) << " ";
					delete (uint16_t*)(v.Ptr());
				}

				if (0 == r->getField("ICMP_TCP_DST_PORT", v))
				{
					outfile << "DPT=" << *(uint16_t*)(v.Ptr()) << " ";
					delete (uint16_t*)(v.Ptr());
				}

				if (0 == r->getField("ICMP_TCP_SEQUENCE", v))
				{
					outfile << "SEQ=" << *(uint32_t*)(v.Ptr()) << " ";
					delete (uint32_t*)(v.Ptr());
				}

				if (0 == r->getField("ICMP_TCP_ACK_NO", v))
				{
					outfile << "ACK=" << *(uint32_t*)(v.Ptr()) << " ";
					delete (uint32_t*)(v.Ptr());
				}

				if (0 == r->getField("ICMP_TCP_WINDOW", v))
				{
					outfile << "WINDOW=" << *(uint16_t*)(v.Ptr()) << " ";
					delete (uint16_t*)(v.Ptr());
				}

				if (0 == r->getField("ICMP_TCP_RESERVED", v))
				{
					outfile << "RES=0x" << hex
					<< (uint32_t)(*(uint8_t*)(v.Ptr()) >> 4)
					<< (uint32_t)(*(uint8_t*)(v.Ptr()) & 0x0F) << " " << dec;
					delete (uint8_t*)(v.Ptr());
				}

				// optional flags
				if (0 == r->getField("ICMP_TCP_FLAG_URGENT", v) && 1 ==
				                *(uint8_t*)(v.Ptr()))
				{
					outfile << "URG ";
					delete (uint8_t*)(v.Ptr());
				}
				if (0 == r->getField("ICMP_TCP_FLAG_ACK", v) && 1 ==
				                *(uint8_t*)(v.Ptr()))
				{
					outfile << "ACK ";
					delete (uint8_t*)(v.Ptr());
				}
				if (0 == r->getField("ICMP_TCP_FLAG_PSH", v) && 1 ==
				                *(uint8_t*)(v.Ptr()))
				{
					outfile << "PSH ";
					delete (uint8_t*)(v.Ptr());
				}
				if (0 == r->getField("ICMP_TCP_FLAG_RST", v) && 1 ==
				                *(uint8_t*)(v.Ptr()))
				{
					outfile << "RST ";
					delete (uint8_t*)(v.Ptr());
				}
				if (0 == r->getField("ICMP_TCP_FLAG_SYN", v) && 1 ==
				                *(uint8_t*)(v.Ptr()))
				{
					outfile << "SYN ";
					delete (uint8_t*)(v.Ptr());
				}
				if (0 == r->getField("ICMP_TCP_FLAG_FIN", v) && 1 ==
				                *(uint8_t*)(v.Ptr()))
				{
					outfile << "FIN ";
					delete (uint8_t*)(v.Ptr());
				}

				// back to our normal programming
				if (0 == r->getField("ICMP_TCP_URGENT_PTR", v))
				{
					outfile << "URGP=" << *(uint32_t*)(v.Ptr()) << " ";
					delete (uint32_t*)(v.Ptr());
				}

				if (0 == r->getField("ICMP_TCP_OPTIONS", v))
				{
					outfile << "OPT (";
					for(int i = 0; i < v.Bytes(); i++)
					{
						char *ptr = (char*)(v.Ptr());
						outfile << hex << uppercase <<
						((*(uint8_t*)(ptr + i) & 0xF0) >> 4)
						<< (*(uint8_t*)(ptr + i) & 0x0F) << nouppercase << dec;
					}
					outfile << ") ";
					delete[] (char*)(v.Ptr());
				}
			}
		} // if (internal protocol is TCP)
		else if (ttype == TRANS_UDP)
		{
			outfile << "PROTO=UDP ";

			if (0 == r->getField("ICMP_UDP_SRC_PORT", v))
			{
				outfile << dec << "SPT=" << *(uint16_t*)(v.Ptr()) << " ";
				delete (uint16_t*)(v.Ptr());
			}

			if (0 == r->getField("ICMP_UDP_DST_PORT", v))
			{
				outfile << "DPT=" << *(uint16_t*)(v.Ptr()) << " ";
				delete (uint16_t*)(v.Ptr());
			}

			if (0 == r->getField("ICMP_UDP_LENGTH", v))
			{
				outfile << "LEN=" << *(uint16_t*)(v.Ptr()) << " ";
				delete (uint16_t*)(v.Ptr());
			}
		}
		else if (ttype == TRANS_ICMP)
		{
			outfile << "PROTO=ICMP ";

			if (0 == r->getField("ICMP_TYPE_INTERNAL", v))
			{
				outfile << "TYPE=" << (uint32_t)(*(uint8_t*)(v.Ptr())) << " ";
				delete (uint8_t*)(v.Ptr());
			}

			if (0 == r->getField("ICMP_CODE_INTERNAL", v))
			{
				outfile << "CODE=" << (uint32_t)(*(uint8_t*)(v.Ptr())) << " ";
				delete (uint8_t*)(v.Ptr());
			}

			if (0 == r->getField("ICMP_ID_INTERNAL", v))
			{
				outfile << "ID=" << *(uint32_t*)(v.Ptr()) << " ";
				delete (uint32_t*)(v.Ptr());
			}

			if (0 == r->getField("ICMP_SEQ_INTERNAL", v))
			{
				outfile << "SEQ=" << *(uint32_t*)(v.Ptr()) << " ";
				delete (uint32_t*)(v.Ptr());
			}
		}
		else
			return 1;
		outfile << "] ";
		return 0;
	}
	else
		return 1;

	return 0;

}

// int putGenericData(Record * r)
// Put the rest of the data for a packet whose protocol type we didn't
// know how to handle
//
// Parameters: r, the record to unpack
// Returns: 0 on success
// Modifies: puts data to the file
int IptableParser::putGenericData(Record * r)
{
	Value v;

	r->getField("SYS_TRANS_TYPE_STR", v);
	outfile << "PROTO=" << *(string*)(v.Ptr()) << " ";
	delete (string*)(v.Ptr());

	r->getField("SYS_GENERIC_DATA", v);
	outfile << *(string*)(v.Ptr());
	delete (string*)(v.Ptr());

	return 0;
}

//Dynamic loading related function
extern "C" LogParser * create(string srcLog, string destLog, string errOutLog, string anonyField)
{
	if (errOutLog.compare("") == 0)
	{
		return new IptableParser(srcLog, destLog, errfn);
	}
	else 
	{
		return new IptableParser(srcLog, destLog, errOutLog);		
	}
}

//destroy an object for iptableLogParser
extern "C" void destroy(LogParser *p)
{
	delete p;
}
