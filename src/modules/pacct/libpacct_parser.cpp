/**********************************************************
 *
 * libpacct_parser.cpp
 * Header file for a generalized libpacct parser
 *
 * originally drafted by Xiaolin Luo
 * last modified Jun 28 22:49:01 CDT 2007
 * 
 **********************************************************/

#include <stdint.h>
#include <string>
#include <fstream>

#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>


#include "libpacct_structs.h"
#include "libpacct_parser.h"
#include "map_record.h"
#include "value.h"
#include "value_types.h"

using namespace std;


static const string xmlFileName = "pacct.modschema.xml";
static const string errfn = "libpacct_flaim.errors";

//#define DEBUG // comment this out to remove debugging messages

// LibpacctParser::LibpacctParser(string const & infile, string const & outfile)
// Constructor
//
// Parameters: infile, the file to read from; outfile, the file to write to
// Returns: Constructs a new object
// Modifies: Nothing
// Notes: The new parser will attempt to open input/output streams

LibpacctParser::LibpacctParser(string const & inputfile,
                               string const & outputfile,
                               string const & errorfile) :
		               infileName(inputfile), outfileName(outputfile), 
                               errfileName(errorfile), curSeq(0)
{       
/*
	char curCTime[20];
	sprintf(curCTime, ".%d", time (NULL));
	errfileName.append(curCTime);
*/
	cerr << "*** pacct module: input from "  << "[" << infileName << "]" << endl
	     << "*** pacct module: output to " << "[" << outfileName << "]" << endl
	     << "*** pacct module: errors to " << "[" << errfileName << "]" << endl;

	if ("" == infileName)
	{
 	    cerr << "*** pacct module: This module does not support streaming input." << endl
		 << "*** pacct module: Please specify an input file." << endl;
		exit(2);
	}

	if ("" == outfileName)
	{
		cerr << "*** pacct module: This module does not support streaming output." << endl
		     << "*** pacct module: Please specify an output file." << endl;
                exit(2);
	}

	// open the data files!
	infile.open(infileName.c_str(), ifstream::binary | ifstream::in);
	outfile.open(outfileName.c_str(), ofstream::binary | ofstream::out);
	errfile.open(errfileName.c_str(), ofstream::binary | ofstream::out);

	// make sure nothing failed
	if (infile.fail())
	{
		cerr << "*** pacct module: Error opening input file." << endl;
		exit(2);
	}

	if (outfile.fail())
	{
		cerr << "*** pacct module: Error opening output file." << endl;
		exit(2);
	}

	if (errfile.fail())
	{
		cerr << "*** pacct module: Error opening error file." << endl;
		exit(2);
	}

	infile.exceptions(ifstream::failbit|ifstream::badbit);	
}


// destructor
LibpacctParser::~LibpacctParser()
{
    outfile.flush();
    errfile.flush();
}


// int getModuleSchemaFile(string & schemaFile)
// Return, by alias, the name of the schema file for this module
//
// Parameters: schemaFile, the target to store into
// Returns: 0 for success, nonzero for failure
// Modifies: Nothing except the parameter's referent
int LibpacctParser::getModuleSchemaFile(string & schemaFile)
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
Record * LibpacctParser::getRecord()  throw (exception&, ifstream::failure&)
{      
    // initialize a new record
    Record * r = new MapRecord();
    
   // Read a PA record
    acct_v02_t * pacct_rcd_v02 = new acct_v02_t;
    acct_v3_t * pacct_rcd_v3;

    int offsetCurRec =  infile.tellg(); // offset of current record        

      try
      {
	  infile.read((char*)pacct_rcd_v02, ACCT_RCD_SIZE);
          pacct_rcd_v3 = (acct_v3_t *)pacct_rcd_v02; 

          //handle differently according to version and endianess

          //little endian

          //v3       
          if (pacct_rcd_v3->ac_version == ACCT_V3){ 
              pacctVer = ACCT_V3; 
              ++curSeq;

   	    //cerr << "*** pacct module: getRecord, ACCT_V3, little Endian." << endl;

	      return getPacctV3Rcd(r, pacct_rcd_v3); 
          }
         
          //v2
          if (pacct_rcd_v02->ac_version == ACCT_V2){ 
              pacctVer = ACCT_V2; 
              ++curSeq;

   	    //cerr << "*** pacct module: getRecord, ACCT_V2, little Endian." << endl;

	      return getPacctV02Rcd(r, pacct_rcd_v02); 
          }
         

          //v1
          if (pacct_rcd_v02->ac_version == ACCT_V1){ 
              pacctVer = ACCT_V1; 
              ++curSeq;

     	    //  cerr << "*** pacct module: getRecord, ACCT_V1, little Endian." << endl;

	      return getPacctV02Rcd(r, pacct_rcd_v02); 
          }
         
         //v0
          if (pacct_rcd_v02->ac_version == ACCT_V0){ 
              pacctVer = ACCT_V0; 
              ++curSeq;

   	    //cerr << "*** pacct module: getRecord, ACCT_V0, little Endian." << endl;

	      return getPacctV02Rcd(r, pacct_rcd_v02); 
          }

         
          //big endian
         //v3
         if ((char)(pacct_rcd_v3->ac_version^ACCT_BIG_ENDIAN) == ACCT_V3){
              pacctVer = ACCT_V3; 
              ++curSeq;

     	    // cerr << "*** pacct module: getRecord, ACCT_V3, big Endian. "  << endl;
            // printf("%d \n", (char) (pacct_rcd_v3->ac_version^ACCT_BIG_ENDIAN));

	      return getPacctV3rRcd(r, pacct_rcd_v3); 
          } 
      
          //v2  
          if ((char)(pacct_rcd_v02->ac_version^ACCT_BIG_ENDIAN) == ACCT_V2){
              pacctVer = ACCT_V2; 
              ++curSeq;

    	 //   cerr << "*** pacct module: getRecord, ACCT_V2, big Endian." << endl; 
          //  printf("%d\n", (char)(pacct_rcd_v02->ac_version^ACCT_BIG_ENDIAN));

	      return getPacctV02rRcd(r, pacct_rcd_v02); 
          } 

          //v1
         if ((char)(pacct_rcd_v02->ac_version^ACCT_BIG_ENDIAN) == ACCT_V1){
              pacctVer = ACCT_V1; 
               ++curSeq;

   	    //   cerr << "*** pacct module: getRecord, ACCT_V1, big Endian." << endl;

	      return getPacctV02rRcd(r, pacct_rcd_v02); 
          } 
         

          //v0
         if ((char)(pacct_rcd_v02->ac_version^ACCT_BIG_ENDIAN) == ACCT_V0){
              pacctVer = ACCT_V0; 
               ++curSeq;

   	    //cerr << "*** pacct module: getRecord, ACCT_V0, big Endian." << endl;
            //printf("%d\n", (char)(pacct_rcd_v02->ac_version^ACCT_BIG_ENDIAN));
	      
              return getPacctV02rRcd(r, pacct_rcd_v02); 
          } 
          

            //invalid version
             cerr << "*** pacct module: Invalid log version in "
				<< "getRecord (error code: " <<  1 
				<<  ", record number: " << curSeq << ", offset: " 
				<< offsetCurRec << ")" << endl;

           //write to the errfile
           errfile.write((char*)pacct_rcd_v02, ACCT_RCD_SIZE);			
		             

           //save the invalid record
	   uint8_t* isSafe = new uint8_t(1);
	   r->setField("IS_SAFE", Value(isSafe, VAL_TYPE_INT8, 1));
	   r->setField("SYS_APP_DATA", Value(pacct_rcd_v02, VAL_TYPE_CHAR_PTR, ACCT_RCD_SIZE));
		
           //set the returnCode
           returnCode = 1;

	    return r; 
      }
      catch (ifstream::failure& ex)
      {
	  cerr << "*** pacct module:" << curSeq <<":I/O error in reading record: " 
			<< ex.what() << endl;

          delete pacct_rcd_v02;
	  
          return NULL;
      }
      catch (exception& ex)
      {
	   cerr << "*** pacct module:" << curSeq <<":Fatal error in reading record: " 
			<< ex.what() << endl;

          delete pacct_rcd_v02;
	 
	  return NULL;
       }
       
}


// int getPacctV02Rcd()
// Pack the next available record from the log into a Record object
//
// Parameters: Record *r
// Returns: Record *

Record* LibpacctParser::getPacctV02Rcd(Record * r,acct_v02_t * pacct_rcd_v02)
{
    // static int i = 0;
    // cerr << "get: " << i++ << endl;
    
    acct_t* pacct_rcd = new acct_t;

    //decode data field and insert into the record
    pacct_rcd->ac_flag = pacct_rcd_v02->ac_flag; 
    r->setField("AC_FLAG", Value(&pacct_rcd->ac_flag, VAL_TYPE_UINT8, 1));

    pacct_rcd->ac_version = pacct_rcd_v02->ac_version; 
    r->setField("AC_VERSION", Value(&pacct_rcd->ac_version, VAL_TYPE_UINT8, 1));

    pacct_rcd->ac_tty = pacct_rcd_v02->ac_tty; 
    r->setField("AC_TTY", Value(&pacct_rcd->ac_tty, VAL_TYPE_UINT16, 2));

    pacct_rcd->ac_exitcode = pacct_rcd_v02->ac_exitcode; 
    r->setField("AC_EXITCODE", Value(&pacct_rcd->ac_exitcode, VAL_TYPE_UINT32, 4));

    pacct_rcd->ac_btime = pacct_rcd_v02->ac_btime; 
    r->setField("AC_BTIME", Value(&pacct_rcd->ac_btime, VAL_TYPE_UINT32, 4));

    pacct_rcd->ac_utime = decode_comp_t(pacct_rcd_v02->ac_utime);       
    r->setField("AC_UTIME", Value(&pacct_rcd->ac_utime, VAL_TYPE_UINT32, 4));

    pacct_rcd->ac_stime = decode_comp_t(pacct_rcd_v02->ac_stime);       
    r->setField("AC_STIME", Value(&pacct_rcd->ac_stime, VAL_TYPE_UINT32, 4));

    pacct_rcd->ac_ahz = pacct_rcd_v02->ac_ahz; 
    r->setField("AC_AHZ", Value(&pacct_rcd->ac_ahz, VAL_TYPE_UINT16, 2));

    if (pacctVer == ACCT_V1 || pacctVer == ACCT_V2) { 
        pacct_rcd->ac_uid = pacct_rcd_v02->ac_uid; 
        r->setField("AC_UID", Value(&pacct_rcd->ac_uid, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_gid = pacct_rcd_v02->ac_gid; 
        r->setField("AC_GID", Value(&pacct_rcd->ac_gid, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_etime = decode_comp2_t(pacct_rcd_v02->ac_etime_lo 
                                          | pacct_rcd_v02->ac_etime_hi << 16);            
        r->setField("AC_ETIME", Value(&pacct_rcd->ac_etime, VAL_TYPE_UINT32, 4));   
    } else { //ACCT_V0
        pacct_rcd->ac_uid = 0;
        pacct_rcd->ac_uid = pacct_rcd_v02->ac_uid16; 
        r->setField("AC_UID", Value(&pacct_rcd->ac_uid, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_gid = 0;
        pacct_rcd->ac_gid = pacct_rcd_v02->ac_gid16; 
        r->setField("AC_GID", Value(&pacct_rcd->ac_gid, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_etime = decode_comp_t(pacct_rcd_v02->ac_etime);
        r->setField("AC_ETIME", Value(&pacct_rcd->ac_etime, VAL_TYPE_UINT32, 4));
    }

    pacct_rcd->ac_mem = decode_comp_t(pacct_rcd_v02->ac_mem);
    r->setField("AC_MEM", Value(&pacct_rcd->ac_mem, VAL_TYPE_UINT32, 4));

    pacct_rcd->ac_io = decode_comp_t(pacct_rcd_v02->ac_io);
    r->setField("AC_IO", Value(&pacct_rcd->ac_io, VAL_TYPE_UINT32, 4));

    pacct_rcd->ac_rw = decode_comp_t(pacct_rcd_v02->ac_rw);
    r->setField("AC_RW", Value(&pacct_rcd->ac_rw, VAL_TYPE_UINT32, 4));

    pacct_rcd->ac_minflt = decode_comp_t(pacct_rcd_v02->ac_minflt);
    r->setField("AC_MINFLT", Value(&pacct_rcd->ac_minflt, VAL_TYPE_UINT32, 4));

    pacct_rcd->ac_majflt = decode_comp_t(pacct_rcd_v02->ac_majflt);
    r->setField("AC_MAJFLT", Value(&pacct_rcd->ac_majflt, VAL_TYPE_UINT32, 4));

    pacct_rcd->ac_swaps = decode_comp_t(pacct_rcd_v02->ac_swaps);
    r->setField("AC_SWAPS", Value(&pacct_rcd->ac_swaps, VAL_TYPE_UINT32, 4));

     strcpy(pacct_rcd->ac_comm, pacct_rcd_v02->ac_comm);
    string *ac_comm = new string(pacct_rcd_v02->ac_comm, strlen(pacct_rcd_v02->ac_comm));
    // string* ac_comm = new string(pacct_rcd->ac_comm, strlen(pacct_rcd->ac_comm));
    r->setField("AC_COMM", Value(ac_comm, VAL_TYPE_STRING, 0)); 
    // r->setField("AC_COMM", Value(&pacct_rcd->ac_comm, VAL_TYPE_CHAR_PTR, strlen(pacct_rcd->ac_comm)));
    
    // make note of where the record is
    r->setField("SYS_PACCT_RCD_LOC", Value(pacct_rcd, VAL_TYPE_CHAR_PTR,
                    sizeof(acct_t)));

    // delete ac_comm;
    delete pacct_rcd_v02;

    return r;
}



// int getPacctV02rRcd()
// Pack the next available record from the log into a Record object
//
// Parameters: Record *r
// Returns: Record *

Record* LibpacctParser::getPacctV02rRcd(Record * r,acct_v02_t * pacct_rcd_v02)
{
       acct_t* pacct_rcd = new acct_t;

       //decode data field and insert into the record
        pacct_rcd->ac_flag = pacct_rcd_v02->ac_flag; 
	r->setField("AC_FLAG", Value(&pacct_rcd->ac_flag, VAL_TYPE_UINT8, 1));

        pacct_rcd->ac_version = pacct_rcd_v02->ac_version; 
	r->setField("AC_VERSION", Value(&pacct_rcd->ac_version, VAL_TYPE_UINT8, 1));

        pacct_rcd->ac_tty = ntohs(pacct_rcd_v02->ac_tty); 
	r->setField("AC_TTY", Value(&pacct_rcd->ac_tty, VAL_TYPE_UINT16, 2));

        pacct_rcd->ac_exitcode = ntohl(pacct_rcd_v02->ac_exitcode); 
	r->setField("AC_EXITCODE", Value(&pacct_rcd->ac_exitcode, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_btime = ntohl(pacct_rcd_v02->ac_btime); 
	r->setField("AC_BTIME", Value(&pacct_rcd->ac_btime, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_utime = decode_comp_t(ntohs(pacct_rcd_v02->ac_utime));
	r->setField("AC_UTIME", Value(&pacct_rcd->ac_utime, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_stime = decode_comp_t(ntohs(pacct_rcd_v02->ac_stime));
	r->setField("AC_STIME", Value(&pacct_rcd->ac_stime, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_ahz = ntohs(pacct_rcd_v02->ac_ahz); 
	r->setField("AC_AHZ", Value(&pacct_rcd->ac_ahz, VAL_TYPE_UINT16, 2));

  	
      if( pacctVer == ACCT_V1 || pacctVer == ACCT_V2)
      { 
        pacct_rcd->ac_uid = ntohl(pacct_rcd_v02->ac_uid); 
	r->setField("AC_UID", Value(&pacct_rcd->ac_uid, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_gid = ntohl(pacct_rcd_v02->ac_gid); 
	r->setField("AC_GID", Value(&pacct_rcd->ac_gid, VAL_TYPE_UINT32, 4));
              
        pacct_rcd->ac_etime = decode_comp2_t(ntohs(pacct_rcd_v02->ac_etime_lo) 
                                            | pacct_rcd_v02->ac_etime_hi << 16);            
        r->setField("AC_ETIME", Value(&pacct_rcd->ac_etime, VAL_TYPE_UINT32, 4));   
      }
      else{

        pacct_rcd->ac_uid = ntohs(pacct_rcd_v02->ac_uid16); 
	r->setField("AC_UID", Value(&pacct_rcd->ac_uid, VAL_TYPE_UINT32, 4));
        pacct_rcd->ac_gid = ntohs(pacct_rcd_v02->ac_gid16); 
	r->setField("AC_GID", Value(&pacct_rcd->ac_gid, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_etime = decode_comp_t(ntohs(pacct_rcd_v02->ac_etime));
	r->setField("AC_ETIME", Value(&pacct_rcd->ac_etime, VAL_TYPE_UINT32, 4));
      }

        pacct_rcd->ac_mem = decode_comp_t(ntohs(pacct_rcd_v02->ac_mem));
	r->setField("AC_MEM", Value(&pacct_rcd->ac_mem, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_io = decode_comp_t(ntohs(pacct_rcd_v02->ac_io));
	r->setField("AC_IO", Value(&pacct_rcd->ac_io, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_rw = decode_comp_t(ntohs(pacct_rcd_v02->ac_rw));
	r->setField("AC_RW", Value(&pacct_rcd->ac_rw, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_minflt = decode_comp_t(ntohs(pacct_rcd_v02->ac_minflt));
	r->setField("AC_MINFLT", Value(&pacct_rcd->ac_minflt, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_majflt = decode_comp_t(ntohs(pacct_rcd_v02->ac_majflt));
	r->setField("AC_MAJFLT", Value(&pacct_rcd->ac_majflt, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_swaps = decode_comp_t(ntohs(pacct_rcd_v02->ac_swaps));
	r->setField("AC_SWAPS", Value(&pacct_rcd->ac_swaps, VAL_TYPE_UINT32, 4));

        strcpy(pacct_rcd->ac_comm,pacct_rcd_v02->ac_comm);
        string *ac_comm = new string(pacct_rcd_v02->ac_comm, strlen(pacct_rcd_v02->ac_comm));
	r->setField("AC_COMM", Value(ac_comm, VAL_TYPE_STRING, 0)); 

      // make note of where the record is
	r->setField("SYS_PACCT_RCD_LOC", Value(pacct_rcd, VAL_TYPE_CHAR_PTR,
			sizeof(acct_t)));

   delete pacct_rcd_v02;

   return r;
}


// int getPacctV3Rcd()
// Pack the next available record from the log into a Record object
//
// Parameters: Record *r
// Returns: Record*

Record* LibpacctParser::getPacctV3Rcd(Record * r, acct_v3_t * pacct_rcd_v3) 
{
        acct_t* pacct_rcd = new acct_t;
        
       //decode data fields and insert into the record
        pacct_rcd->ac_flag = pacct_rcd_v3->ac_flag; 
	r->setField("AC_FLAG", Value(&pacct_rcd->ac_flag, VAL_TYPE_UINT8, 1));

        pacct_rcd->ac_version = pacct_rcd_v3->ac_version; 
	r->setField("AC_VERSION", Value(&pacct_rcd->ac_version, VAL_TYPE_UINT8, 1));

        pacct_rcd->ac_tty = pacct_rcd_v3->ac_tty; 
	r->setField("AC_TTY", Value(&pacct_rcd->ac_tty, VAL_TYPE_UINT16, 2));

        pacct_rcd->ac_exitcode = pacct_rcd_v3->ac_exitcode; 
	r->setField("AC_EXITCODE", Value(&pacct_rcd->ac_exitcode, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_uid = pacct_rcd_v3->ac_uid; 
	r->setField("AC_UID", Value(&pacct_rcd->ac_uid, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_gid = pacct_rcd_v3->ac_gid; 
	r->setField("AC_GID", Value(&pacct_rcd->ac_gid, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_pid = pacct_rcd_v3->ac_pid; 
	r->setField("AC_PID", Value(&pacct_rcd->ac_pid, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_ppid = pacct_rcd_v3->ac_ppid; 
	r->setField("AC_PPID", Value(&pacct_rcd->ac_ppid, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_btime = pacct_rcd_v3->ac_btime; 
	r->setField("AC_BTIME", Value(&pacct_rcd->ac_btime, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_utime = decode_comp_t(pacct_rcd_v3->ac_utime);
	r->setField("AC_UTIME", Value(&pacct_rcd->ac_utime, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_stime = decode_comp_t(pacct_rcd_v3->ac_stime);
	r->setField("AC_STIME", Value(&pacct_rcd->ac_stime, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_etime = decode_float(pacct_rcd_v3->ac_etime);
	r->setField("AC_ETIME", Value(&pacct_rcd->ac_etime, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_mem = decode_comp_t(pacct_rcd_v3->ac_mem);
	r->setField("AC_MEM", Value(&pacct_rcd->ac_mem, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_io = decode_comp_t(pacct_rcd_v3->ac_io);
	r->setField("AC_IO", Value(&pacct_rcd->ac_io, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_rw = decode_comp_t(pacct_rcd_v3->ac_rw);
	r->setField("AC_RW", Value(&pacct_rcd->ac_rw, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_minflt = decode_comp_t(pacct_rcd_v3->ac_minflt);
	r->setField("AC_MINFLT", Value(&pacct_rcd->ac_minflt, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_majflt = decode_comp_t(pacct_rcd_v3->ac_majflt);
	r->setField("AC_MAJFLT", Value(&pacct_rcd->ac_majflt, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_swaps = decode_comp_t(pacct_rcd_v3->ac_swaps);
	r->setField("AC_SWAPS", Value(&pacct_rcd->ac_swaps, VAL_TYPE_UINT32, 4));

        strcpy(pacct_rcd->ac_comm,pacct_rcd_v3->ac_comm);
        string *ac_comm = new string(pacct_rcd_v3->ac_comm, strlen(pacct_rcd_v3->ac_comm));
	r->setField("AC_COMM", Value(ac_comm, VAL_TYPE_STRING, 0)); 

      // make note of where the record is
	r->setField("SYS_PACCT_RCD_LOC", Value(pacct_rcd, VAL_TYPE_CHAR_PTR,
			sizeof(acct_t)));

   delete pacct_rcd_v3;

   return r;
}



// int getPacctV3rRcd()
// Pack the next available record from the log into a Record object
//
// Parameters: Record *r
// Returns: Record*

Record* LibpacctParser::getPacctV3rRcd(Record * r, acct_v3_t * pacct_rcd_v3) 
{
       acct_t* pacct_rcd = new acct_t;
        
       //decode data fields and insert into the record
        pacct_rcd->ac_flag = pacct_rcd_v3->ac_flag; 
	r->setField("AC_FLAG", Value(&pacct_rcd->ac_flag, VAL_TYPE_UINT8, 1));

        pacct_rcd->ac_version = pacct_rcd_v3->ac_version; 
	r->setField("AC_VERSION", Value(&pacct_rcd->ac_version, VAL_TYPE_UINT8, 1));

        pacct_rcd->ac_tty = ntohs(pacct_rcd_v3->ac_tty);
	r->setField("AC_TTY", Value(&pacct_rcd->ac_tty, VAL_TYPE_UINT16, 2));

        pacct_rcd->ac_exitcode = ntohl(pacct_rcd_v3->ac_exitcode);
	r->setField("AC_EXITCODE", Value(&pacct_rcd->ac_exitcode, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_uid = ntohl(pacct_rcd_v3->ac_uid);
	r->setField("AC_UID", Value(&pacct_rcd->ac_uid, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_gid = ntohl(pacct_rcd_v3->ac_gid);
	r->setField("AC_GID", Value(&pacct_rcd->ac_gid, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_pid = ntohl(pacct_rcd_v3->ac_pid);
	r->setField("AC_PID", Value(&pacct_rcd->ac_pid, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_ppid = ntohl(pacct_rcd_v3->ac_ppid);
	r->setField("AC_PPID", Value(&pacct_rcd->ac_ppid, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_btime = ntohl(pacct_rcd_v3->ac_btime);
	r->setField("AC_BTIME", Value(&pacct_rcd->ac_btime, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_utime = decode_comp_t(ntohs(pacct_rcd_v3->ac_utime));
	r->setField("AC_UTIME", Value(&pacct_rcd->ac_utime, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_stime = decode_comp_t(ntohs(pacct_rcd_v3->ac_stime));
	r->setField("AC_STIME", Value(&pacct_rcd->ac_stime, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_etime = decode_float(invf(pacct_rcd_v3->ac_etime));
	r->setField("AC_ETIME", Value(&pacct_rcd->ac_etime, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_mem = decode_comp_t(ntohs(pacct_rcd_v3->ac_mem));
	r->setField("AC_MEM", Value(&pacct_rcd->ac_mem, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_io = decode_comp_t(ntohs(pacct_rcd_v3->ac_io));
	r->setField("AC_IO", Value(&pacct_rcd->ac_io, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_rw = decode_comp_t(ntohs(pacct_rcd_v3->ac_rw));
	r->setField("AC_RW", Value(&pacct_rcd->ac_rw, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_minflt = decode_comp_t(ntohs(pacct_rcd_v3->ac_minflt));
	r->setField("AC_MINFLT", Value(&pacct_rcd->ac_minflt, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_majflt = decode_comp_t(ntohs(pacct_rcd_v3->ac_majflt));
	r->setField("AC_MAJFLT", Value(&pacct_rcd->ac_majflt, VAL_TYPE_UINT32, 4));

        pacct_rcd->ac_swaps = decode_comp_t(ntohs(pacct_rcd_v3->ac_swaps));
	r->setField("AC_SWAPS", Value(&pacct_rcd->ac_swaps, VAL_TYPE_UINT32, 4));

        strcpy(pacct_rcd->ac_comm,pacct_rcd_v3->ac_comm);
        string *ac_comm = new string(pacct_rcd_v3->ac_comm, strlen(pacct_rcd_v3->ac_comm));
	r->setField("AC_COMM", Value(ac_comm, VAL_TYPE_STRING, 0)); 

      // make note of where the record is
	r->setField("SYS_PACCT_RCD_LOC", Value(pacct_rcd, VAL_TYPE_CHAR_PTR,
			sizeof(acct_t)));

   delete pacct_rcd_v3;

   return r;
}



// int putRecord(Record * r)
// Write a record to the output file
//
// Parameters: r, the record to write
// Returns: 0 on success, 1 on failure
// Modifies: Data is written to the output file
int LibpacctParser::putRecord(Record * r)
{
 
    Value v;

    // write the invalid record unchanged to the output file
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

    //get a pointer to the version
     r->getField("AC_VERSION", v);
     pacctVer = *(char *)(v.Ptr());

 
      //little endian
     if (pacctVer == ACCT_V3){               
            //cerr << "*** pacct module: putRecord, ACCT_V3, little Endian." << endl;
            return putPacctV3Rcd(r);
      }
     
   
     if (pacctVer == ACCT_V2){               
            // cerr << "*** pacct module: putRecord, ACCT_V2, little Endian." << endl;
            return putPacctV02Rcd(r);
      }

 
     if (pacctVer == ACCT_V1){               
           // cerr << "*** pacct module: putRecord, ACCT_V1, little Endian." << endl;
            return putPacctV02Rcd(r);
      }
 
     if (pacctVer == ACCT_V0){               

   	   // cerr << "*** pacct module: putRecord, ACCT_V0, little Endian." << endl;

            return putPacctV02Rcd(r);
      }

      //big endian
     if ((char)(pacctVer^ACCT_BIG_ENDIAN) == ACCT_V3){
             //cerr << "*** pacct module: putRecord, ACCT_V3, big Endian." << endl;
             pacctVer = ACCT_V3; 
             return putPacctV3rRcd(r);	   
      } 
    

      if ((char)(pacctVer^ACCT_BIG_ENDIAN) == ACCT_V2){
             //cerr << "*** pacct module: putRecord, ACCT_V2, big Endian." << endl;
             pacctVer = ACCT_V2; 
             return putPacctV02rRcd(r);	   
      } 

	
      if ((char)(pacctVer^ACCT_BIG_ENDIAN) == ACCT_V1){
             pacctVer = ACCT_V1; 
             //cerr << "*** pacct module: putRecord, ACCT_V1, big Endian." << endl;
             return putPacctV02rRcd(r);	   
      }
 
      if ((char)(pacctVer^ACCT_BIG_ENDIAN) == ACCT_V0){
             pacctVer = ACCT_V0; 

            //cerr << "*** pacct module: putRecord, ACCT_V0, big Endian." << endl;
             return putPacctV02rRcd(r);	   
      } 


      //error handling      
       cerr << "*** pacct module: Warning: invalid log version " << pacctVer 
		<< "in putRecord ()" << endl;
	
       //more error handling here
	exit(1);
   
}



// int putPacctV02Rcd(Record * r)
// Write a record to the output file
//
// Parameters: r, the record to write
// Returns: 0 on success, 1 on failure
// Modifies: Data is written to the output file
int LibpacctParser::putPacctV02Rcd(Record * r)
{
    // static int i = 0;
    // cerr << "put: " << i++ << endl;
    Value v;
    acct_v02_t* pacct_rcd_v02 = new acct_v02_t;

    //get a pointer to the record
    r->getField("SYS_PACCT_RCD_LOC", v);
    acct_t* pacct_rcd = (acct_t*)(v.Ptr());

    //encode data field    
    pacct_rcd_v02->ac_flag = pacct_rcd->ac_flag; 
    pacct_rcd_v02->ac_version = pacct_rcd->ac_version; 
    pacct_rcd_v02->ac_tty = pacct_rcd->ac_tty; 
    pacct_rcd_v02->ac_exitcode = pacct_rcd->ac_exitcode; 
    pacct_rcd_v02->ac_btime = pacct_rcd->ac_btime;
    pacct_rcd_v02->ac_ahz = pacct_rcd->ac_ahz;

    pacct_rcd_v02->ac_utime = encode_comp_t(pacct_rcd->ac_utime);
    pacct_rcd_v02->ac_stime = encode_comp_t(pacct_rcd->ac_stime);
    pacct_rcd_v02->ac_mem = encode_comp_t(pacct_rcd->ac_mem);
    pacct_rcd_v02->ac_io = encode_comp_t(pacct_rcd->ac_io);
    pacct_rcd_v02->ac_rw = encode_comp_t(pacct_rcd->ac_rw);
    pacct_rcd_v02->ac_minflt = encode_comp_t(pacct_rcd->ac_minflt);
    pacct_rcd_v02->ac_majflt = encode_comp_t(pacct_rcd->ac_majflt);
    pacct_rcd_v02->ac_swaps = encode_comp_t(pacct_rcd->ac_swaps);

    memset(pacct_rcd_v02->ac_comm, '\0', sizeof(pacct_rcd_v02->ac_comm));
    
    if (0 == r->getField("AC_COMM", v)) {                
        strcpy(pacct_rcd_v02->ac_comm, ((string*)(v.Ptr()))->c_str());
        delete (string*)(v.Ptr());
    }    
    // strcpy(pacct_rcd_v02->ac_comm, pacct_rcd->ac_comm);

    if ( pacctVer == ACCT_V1 || pacctVer == ACCT_V2) {
        comp2_t ac_etime = encode_comp2_t(pacct_rcd->ac_etime);
        pacct_rcd_v02->ac_etime = encode_comp_t(ac_etime);       
        pacct_rcd_v02->ac_etime_hi = ac_etime >> 16;
        pacct_rcd_v02->ac_etime_lo = (u_int16_t)ac_etime;

        pacct_rcd_v02->ac_uid = pacct_rcd->ac_uid;
        pacct_rcd_v02->ac_gid = pacct_rcd->ac_gid;

        pacct_rcd_v02->ac_uid16 = (u_int16_t) pacct_rcd->ac_uid;
        pacct_rcd_v02->ac_gid16 = (u_int16_t) pacct_rcd->ac_gid;
    } else {
        pacct_rcd_v02->ac_etime = encode_comp_t(pacct_rcd->ac_etime);
        pacct_rcd_v02->ac_etime_hi = (u_int8_t)0;
        pacct_rcd_v02->ac_etime_lo = (u_int16_t)pacct_rcd_v02->ac_etime;

        pacct_rcd_v02->ac_uid = pacct_rcd->ac_uid;
        pacct_rcd_v02->ac_gid = pacct_rcd->ac_gid;
        pacct_rcd_v02->ac_uid16 = (u_int16_t) pacct_rcd->ac_uid;
        pacct_rcd_v02->ac_gid16 = (u_int16_t) pacct_rcd->ac_gid;
    }    

    //write the record
    outfile.write((char*)pacct_rcd_v02, sizeof(acct_v02_t));

    delete pacct_rcd;
    delete pacct_rcd_v02;
    delete r;

    return 0;
}



// int putPacctV02rRcd(Record * r)
// Write a record to the output file
//
// Parameters: r, the record to write
// Returns: 0 on success, 1 on failure
// Modifies: Data is written to the output file
int LibpacctParser::putPacctV02rRcd(Record * r)
{
     Value v;
     acct_v02_t * pacct_rcd_v02 = new acct_v02_t;

    //get a pointer to the record
     r->getField("SYS_PACCT_RCD_LOC", v);
     acct_t *pacct_rcd = (acct_t*)(v.Ptr());

    //encode data field    
       pacct_rcd_v02->ac_flag = pacct_rcd->ac_flag; 
       pacct_rcd_v02->ac_version = pacct_rcd->ac_version; 
       pacct_rcd_v02->ac_tty = htons(pacct_rcd->ac_tty); 
       pacct_rcd_v02->ac_exitcode = htonl(pacct_rcd->ac_exitcode); 
       pacct_rcd_v02->ac_btime = htonl(pacct_rcd->ac_btime);
       pacct_rcd_v02->ac_ahz = htons(pacct_rcd->ac_ahz);

       pacct_rcd_v02->ac_utime = htons(encode_comp_t(pacct_rcd->ac_utime));
       pacct_rcd_v02->ac_stime = htons(encode_comp_t(pacct_rcd->ac_stime));
       pacct_rcd_v02->ac_mem = htons(encode_comp_t(pacct_rcd->ac_mem));
       pacct_rcd_v02->ac_io = htons(encode_comp_t(pacct_rcd->ac_io));
       pacct_rcd_v02->ac_rw = htons(encode_comp_t(pacct_rcd->ac_rw));
       pacct_rcd_v02->ac_minflt = htons(encode_comp_t(pacct_rcd->ac_minflt));
       pacct_rcd_v02->ac_majflt = htons(encode_comp_t(pacct_rcd->ac_majflt));
       pacct_rcd_v02->ac_swaps = htons(encode_comp_t(pacct_rcd->ac_swaps));
       
       memset(pacct_rcd_v02->ac_comm, '\0', sizeof(pacct_rcd_v02->ac_comm));
       if (0 == r->getField("AC_COMM", v))
       {                
          strcpy(pacct_rcd_v02->ac_comm, ((string*)(v.Ptr()))->c_str());
  	  delete (string*)(v.Ptr());
	}

      if( pacctVer == ACCT_V1 || pacctVer == ACCT_V2){
        comp2_t ac_etime = encode_comp2_t(pacct_rcd->ac_etime);
        pacct_rcd_v02->ac_etime_hi = ac_etime >> 16;
        pacct_rcd_v02->ac_etime_lo = htons((u_int16_t)ac_etime);
        pacct_rcd_v02->ac_etime = htons(encode_comp_t(pacct_rcd->ac_etime));

        pacct_rcd_v02->ac_uid = htonl(pacct_rcd->ac_uid);
        pacct_rcd_v02->ac_gid = htonl(pacct_rcd->ac_gid);

        pacct_rcd_v02->ac_uid16 = (u_int16_t) pacct_rcd->ac_uid;
        pacct_rcd_v02->ac_gid16 = (u_int16_t) pacct_rcd->ac_gid;
      }
      else{//ACCT_V0
        pacct_rcd_v02->ac_etime = htons(encode_comp_t(pacct_rcd->ac_etime));
        
        pacct_rcd_v02->ac_etime_hi = (u_int8_t)0;
        pacct_rcd_v02->ac_etime_lo = (u_int16_t)pacct_rcd_v02->ac_etime;

        pacct_rcd_v02->ac_uid = htonl(pacct_rcd->ac_uid);
        pacct_rcd_v02->ac_gid = htonl(pacct_rcd->ac_gid);

        pacct_rcd_v02->ac_uid16 = (u_int16_t) pacct_rcd->ac_uid;
        pacct_rcd_v02->ac_uid16 = htons(pacct_rcd_v02->ac_uid16);
        pacct_rcd_v02->ac_gid16 = (u_int16_t) pacct_rcd->ac_gid;
        pacct_rcd_v02->ac_gid16 = htons(pacct_rcd_v02->ac_gid16);
      }    

    //write the record
     outfile.write((char*)pacct_rcd_v02, sizeof(acct_v02_t));

     delete pacct_rcd;
     delete pacct_rcd_v02;
     delete r;
     
     return 0;
}




// int putPacctV3Rcd(Record * r)
// Write a record to the output file
//
// Parameters: r, the record to write
// Returns: 0 on success, 1 on failure
// Modifies: Data is written to the output file
int LibpacctParser::putPacctV3Rcd(Record * r)
{
    Value v;
    acct_v3_t * pacct_rcd_v3 = new acct_v3_t;

    // get a pointer to the header in question
    r->getField("SYS_PACCT_RCD_LOC", v);
    acct_t *pacct_rcd = (acct_t*)(v.Ptr());

    //encode to log format    
    pacct_rcd_v3->ac_flag = pacct_rcd->ac_flag; 
    pacct_rcd_v3->ac_version = pacct_rcd->ac_version; 
    pacct_rcd_v3->ac_tty = pacct_rcd->ac_tty; 
    pacct_rcd_v3->ac_exitcode = pacct_rcd->ac_exitcode; 
    pacct_rcd_v3->ac_btime = pacct_rcd->ac_btime;
    pacct_rcd_v3->ac_uid = pacct_rcd->ac_uid; 
    pacct_rcd_v3->ac_gid = pacct_rcd->ac_gid;
    pacct_rcd_v3->ac_pid = pacct_rcd->ac_pid; 
    pacct_rcd_v3->ac_ppid = pacct_rcd->ac_ppid;

    memset(pacct_rcd_v3->ac_comm, '\0', sizeof(pacct_rcd_v3->ac_comm));
    if (0 == r->getField("AC_COMM", v))
    {                
       strcpy(pacct_rcd_v3->ac_comm, ((string*)(v.Ptr()))->c_str());
       delete (string*)(v.Ptr());
    }

    pacct_rcd_v3->ac_utime = encode_comp_t(pacct_rcd->ac_utime);
    pacct_rcd_v3->ac_stime = encode_comp_t(pacct_rcd->ac_stime);
    pacct_rcd_v3->ac_etime = encode_float((unsigned long)pacct_rcd->ac_etime);
    pacct_rcd_v3->ac_mem = encode_comp_t(pacct_rcd->ac_mem);
    pacct_rcd_v3->ac_io = encode_comp_t(pacct_rcd->ac_io);
    pacct_rcd_v3->ac_rw = encode_comp_t(pacct_rcd->ac_rw);
    pacct_rcd_v3->ac_minflt = encode_comp_t(pacct_rcd->ac_minflt);
    pacct_rcd_v3->ac_majflt = encode_comp_t(pacct_rcd->ac_majflt);
    pacct_rcd_v3->ac_swaps = encode_comp_t(pacct_rcd->ac_swaps);

    
    // write the record
    outfile.write((char*)pacct_rcd_v3, sizeof(acct_v3_t));

    delete pacct_rcd;
    delete pacct_rcd_v3;
    delete r;

    return 0;
}



// int putPacctV3rRcd(Record * r)
// Write a record to the output file
//
// Parameters: r, the record to write
// Returns: 0 on success, 1 on failure
// Modifies: Data is written to the output file
int LibpacctParser::putPacctV3rRcd(Record * r)
{
    Value v;
    acct_v3_t * pacct_rcd_v3 = new acct_v3_t;
 
    // get a pointer to the header in question
    r->getField("SYS_PACCT_RCD_LOC", v);
    acct_t *pacct_rcd = (acct_t*)(v.Ptr());

    //encode to log format    
    pacct_rcd_v3->ac_flag = pacct_rcd->ac_flag; 
    pacct_rcd_v3->ac_version = pacct_rcd->ac_version; 
    pacct_rcd_v3->ac_tty = htons(pacct_rcd->ac_tty); 
    pacct_rcd_v3->ac_exitcode = htonl(pacct_rcd->ac_exitcode); 
    pacct_rcd_v3->ac_btime = htonl(pacct_rcd->ac_btime);
    pacct_rcd_v3->ac_uid = htonl(pacct_rcd->ac_uid); 
    pacct_rcd_v3->ac_gid = htonl(pacct_rcd->ac_gid);
    pacct_rcd_v3->ac_pid = htonl(pacct_rcd->ac_pid); 
    pacct_rcd_v3->ac_ppid = htonl(pacct_rcd->ac_ppid);

    memset(pacct_rcd_v3->ac_comm, '\0', sizeof(pacct_rcd_v3->ac_comm));
    if (0 == r->getField("AC_COMM", v))
    {                
       strcpy(pacct_rcd_v3->ac_comm, ((string*)(v.Ptr()))->c_str());
       delete (string*)(v.Ptr());
    }

    pacct_rcd_v3->ac_utime = htons(encode_comp_t(pacct_rcd->ac_utime));
    pacct_rcd_v3->ac_stime = htons(encode_comp_t(pacct_rcd->ac_stime));
    pacct_rcd_v3->ac_etime = invf(encode_float((unsigned long)pacct_rcd->ac_etime));
    pacct_rcd_v3->ac_mem = htons(encode_comp_t(pacct_rcd->ac_mem));
    pacct_rcd_v3->ac_io = htons(encode_comp_t(pacct_rcd->ac_io));
    pacct_rcd_v3->ac_rw = htons(encode_comp_t(pacct_rcd->ac_rw));
    pacct_rcd_v3->ac_minflt = htons(encode_comp_t(pacct_rcd->ac_minflt));
    pacct_rcd_v3->ac_majflt = htons(encode_comp_t(pacct_rcd->ac_majflt));
    pacct_rcd_v3->ac_swaps = htons(encode_comp_t(pacct_rcd->ac_swaps));
    
    // write the record
    outfile.write((char*)pacct_rcd_v3, sizeof(acct_v3_t));

    delete pacct_rcd;
    delete pacct_rcd_v3;
    delete r;

    return 0;
}

// int isEndofLog()
// Test to see if the end of the current log has been reached
//
// Parameters: None
// Returns: 1 if there are no more records in the log, 0 otherwise
// Modifies: Nothing
int LibpacctParser::isEndofLog()
{
	bool eof = true;
	
	try 
	{
	   infile.peek();
	   eof = infile.eof();
	   if (!eof) 
              return 0;
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
int LibpacctParser::getRecordSeq(int & seq)
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
int LibpacctParser::resetFilePointer()
{
     infile.seekg(0, ios::beg);
     curSeq = 0;
    return 0;
}

	


/*****************************
 * Dynamic loading functions *
 *****************************/

extern "C" LogParser * create(string srcLog, string destLog, string errOutLog, string anonyField)
{
	if (errOutLog.compare("") == 0)
	{
		return new LibpacctParser(srcLog, destLog, errfn);
	}
	else
	{
		return new LibpacctParser(srcLog, destLog, errOutLog);
	}
}

extern "C" void destroy(LogParser *p)
{
     delete p;
}
	   	

