/**********************************************************
 *
 * libpacct_parser.h
 * Header file for the libpacct parser
 *
 * originally drafted by Xiaolin Luo
 * last modified May 22 20:51:01 CDT 2007
 * 
 **********************************************************/

#ifndef _LIBPACCT_PARSER_H
#define _LIBPACCT_PARSER_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <fstream>

#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "libpacct_structs.h"
#include "flaim.h"
#include "value.h"
#include "record.h"
#include "map_record.h"

using namespace std;

//parser class
class LibpacctParser : public LogParser
{
    public:

	// constructor 
	LibpacctParser(string const & inputfile, string const & outputfile,
		       string const & errorfile);

        //destructor
	~LibpacctParser();

	//give the name of the module schema file to the caller
	int getModuleSchemaFile(string & schemaFile);

	//get one record from the input file
	Record * getRecord() throw (exception&, ifstream::failure&);

	//put one record to the output file
	int putRecord(Record * r);

	//test to see if the end of the log has been reached
	int isEndofLog();

	// print the number of the current record
	int getRecordSeq(int & seq);

	// reset the pointer to the start of the file
	int resetFilePointer();

    private:
	
	//get functions
	Record* getPacctV02Rcd(Record * r, acct_v02_t * pacct_rcd_v02);   //little endian
	Record* getPacctV02rRcd(Record * r, acct_v02_t * pacct_rcd_v02);  //big endian
	Record* getPacctV3Rcd(Record * r, acct_v3_t * pacct_rcd_v3);
	Record* getPacctV3rRcd(Record * r, acct_v3_t * pacct_rcd_v3);

	//put functions
	int putPacctV02Rcd(Record * r);   //little endian
	int putPacctV02rRcd(Record * r);  //big endian
	int putPacctV3Rcd(Record * r);
	int putPacctV3rRcd(Record * r);

	// member variables
	string infileName; // name of file to read from
	string outfileName; // name of file to write to
	string errfileName; // name of file to write errors to
	ifstream infile; // descriptor for file to read from
	ofstream outfile; // descriptor for file to write to
	ofstream errfile; // descriptor for file to write errors to

	int curSeq; // current record number
	char pacctVer; // constants to tell us the verison of pacct
        bool isErrors;         	   	
};

#endif // _LIBPACCT_PARSER_H
