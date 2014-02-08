/******************************************************************************
 *
 * streams.cpp
 *
 * Defines libiptable member functions which stream input/output
 *
 * last modified by Greg Colombo
 * Tue Jul 18 14:32:02 GMT 2006
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

#define INLINE_LENGTH 512
static char tmpinstr[INLINE_LENGTH];

// int putRecordStreaming(Record * r)
// Unpack and write a record to the output file
//
// Parameters: r, the record to unpack
// Returns: 0 on success
// Modifies: Data is written to the output file
// Notes: This function destroys *r
int IptableParser::putRecordStreaming(Record * r)
{
    Value v;

  
    // get transport layer type information
    int transType = TRANS_UNKNOWN;
    if(0 == r->getField("SYS_TRANS_TYPE", v))
    {
	transType = (int)(*(uint8_t*)v.Ptr());
	delete (uint8_t*)v.Ptr();
    }

    // get the system time
    struct tm timestamp;
    r->getField("PCKT_TS_SEC", v);
    gmtime_r((time_t*)((uint32_t*)(v.Ptr())), &timestamp);
    delete (uint32_t*)(v.Ptr());
  
  
    // unpack the timestamp
    switch(timestamp.tm_mon)
    {
	case 0: cout << "Jan "; break;
	case 1: cout << "Feb "; break;
	case 2: cout << "Mar "; break;
	case 3: cout << "Apr "; break;
	case 4: cout << "May "; break;
	case 5: cout << "Jun "; break;
	case 6: cout << "Jul "; break;
	case 7: cout << "Aug "; break;
	case 8: cout << "Sep "; break;
	case 9: cout << "Oct "; break;
	case 10: cout << "Nov "; break;
	case 11: cout << "Dec "; break;
	default: cout << "*** "; break;
    }
    if(timestamp.tm_mday < 10) { cout << " "; }
    cout << timestamp.tm_mday << " ";

    if(timestamp.tm_hour < 10) { cout << "0"; }
    cout << timestamp.tm_hour << ":";
    if(timestamp.tm_min < 10) { cout << "0"; }
    cout << timestamp.tm_min << ":";
    if(timestamp.tm_sec < 10) { cout << "0"; }
    cout << timestamp.tm_sec << " ";


    // machine name
    r->getField("PCKT_MACHINE_NAME", v);
    cout << *(string*)(v.Ptr()) << " ";
    delete (string*)(v.Ptr());

    // log prefix
    r->getField("PCKT_LOG_PREFIX", v);
    cout << *(string*)(v.Ptr());
    delete (string*)(v.Ptr());

    // see if there is an input interface value set
    cout << "IN=";
    if(0 == r->getField("PCKT_IN_INTERFACE", v))
    {
	cout << *(string*)(v.Ptr());
	delete (string*)(v.Ptr());
    }
    cout << " ";

    // same thing for the output interface
    cout << "OUT=";
    if(0 == r->getField("PCKT_OUT_INTERFACE", v))
    {
	cout << *(string*)(v.Ptr());
	delete (string*)(v.Ptr());
    }
    cout << " ";



    // print the MAC address if it exists
    if(0 == r->getField("ETHER_DST_MAC", v))
    {
	char *ptr = (char*)v.Ptr();

	// switch to hexadecimal printing and start printing the MAC address
	// destination part
	cout << "MAC=";
	for(int i = 0; i < 6; i++)
	{
	    switch((uint8_t)(ptr[i]) >> 4)
	    {
		case 0: cout << "0"; break;
		case 1: cout << "1"; break;
		case 2: cout << "2"; break;
		case 3: cout << "3"; break;
		case 4: cout << "4"; break;
		case 5: cout << "5"; break;
		case 6: cout << "6"; break;
		case 7: cout << "7"; break;
		case 8: cout << "8"; break;
		case 9: cout << "9"; break;
		case 10: cout << "a"; break;
		case 11: cout << "b"; break;
		case 12: cout << "c"; break;
		case 13: cout << "d"; break;
		case 14: cout << "e"; break;
		case 15: cout << "f"; break;
		default: cout << "*"; break;
	    }
	    switch(ptr[i] & 0x0F)
	    {
		case 0: cout << "0"; break;
		case 1: cout << "1"; break;
		case 2: cout << "2"; break;
		case 3: cout << "3"; break;
		case 4: cout << "4"; break;
		case 5: cout << "5"; break;
		case 6: cout << "6"; break;
		case 7: cout << "7"; break;
		case 8: cout << "8"; break;
		case 9: cout << "9"; break;
		case 10: cout << "a"; break;
		case 11: cout << "b"; break;
		case 12: cout << "c"; break;
		case 13: cout << "d"; break;
		case 14: cout << "e"; break;
		case 15: cout << "f"; break;
		default: cout << "*"; break;
	    }
	    cout << ":";
	}
	delete[] ptr;
        
     

	// source part
	r->getField("ETHER_SRC_MAC", v);
	ptr = (char*)v.Ptr();
	for(int i = 0; i < 6; i++)
	{
	    switch((uint8_t)(ptr[i]) >> 4)
	    {
		case 0: cout << "0"; break;
		case 1: cout << "1"; break;
		case 2: cout << "2"; break;
		case 3: cout << "3"; break;
		case 4: cout << "4"; break;
		case 5: cout << "5"; break;
		case 6: cout << "6"; break;
		case 7: cout << "7"; break;
		case 8: cout << "8"; break;
		case 9: cout << "9"; break;
		case 10: cout << "a"; break;
		case 11: cout << "b"; break;
		case 12: cout << "c"; break;
		case 13: cout << "d"; break;
		case 14: cout << "e"; break;
		case 15: cout << "f"; break;
		default: cout << "*"; break;
	    }
	    switch(ptr[i] & 0x0F)
	    {
		case 0: cout << "0"; break;
		case 1: cout << "1"; break;
		case 2: cout << "2"; break;
		case 3: cout << "3"; break;
		case 4: cout << "4"; break;
		case 5: cout << "5"; break;
		case 6: cout << "6"; break;
		case 7: cout << "7"; break;
		case 8: cout << "8"; break;
		case 9: cout << "9"; break;
		case 10: cout << "a"; break;
		case 11: cout << "b"; break;
		case 12: cout << "c"; break;
		case 13: cout << "d"; break;
		case 14: cout << "e"; break;
		case 15: cout << "f"; break;
		default: cout << "*"; break;
	    }
	    cout << ":";
	}
	delete[] ptr;

	// network layer type
	r->getField("SYS_NET_TYPE", v);
	cout << hex <<  
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
    cout << "SRC=" << inet_ntoa(ipaddr) << " ";
    delete (uint32_t*)(v.Ptr());

    r->getField("IPV4_DST_IP", v);
    ipaddr.s_addr = htonl(*(uint32_t*)(v.Ptr()));
    cout << "DST=" << inet_ntoa(ipaddr) << " ";
    delete (uint32_t*)(v.Ptr());

    // IP length
    r->getField("IPV4_LENGTH", v);
    cout << "LEN=" << *(uint16_t*)(v.Ptr()) << " ";
    delete (uint16_t*)(v.Ptr());

    // IP TOS
    r->getField("IPV4_TOS", v);
    cout << hex << "TOS=0x" << ((*(uint8_t*)(v.Ptr()) & 0xF0) >> 4)
	<< (*(uint8_t*)(v.Ptr()) & 0x0F) << " ";
    delete (uint8_t*)(v.Ptr());
  

    // IP precision
    r->getField("IPV4_PRECEDENCE", v);
    cout << hex << "PREC=0x" << uppercase 
	<< ((*(uint8_t*)(v.Ptr()) & 0xF0) >> 4)
	<< (*(uint8_t*)(v.Ptr()) & 0x0F) << nouppercase << " ";
    delete (uint8_t*)(v.Ptr());    


    // IP TTL
    r->getField("IPV4_TTL", v);
    cout << dec << "TTL=" << (uint32_t)*(uint8_t*)(v.Ptr()) << " ";
    delete (uint8_t*)(v.Ptr());

    // IP ID
    r->getField("IPV4_ID", v);
    cout << "ID=" << *(uint16_t*)(v.Ptr()) << " ";
    delete (uint16_t*)(v.Ptr());


    // various flags, if they exist and were not anonymized
    if(0 == r->getField("IPV4_CE", v) && 1 == *(uint8_t*)(v.Ptr()))
    {
	cout << "CE ";
	delete (uint8_t*)(v.Ptr());
    }
    if(0 == r->getField("IPV4_DF", v) && 1 == *(uint8_t*)(v.Ptr()))
    {
	cout << "DF ";
	delete (uint8_t*)(v.Ptr());
    }
    if(0 == r->getField("IPV4_MF", v) && 1 == *(uint8_t*)(v.Ptr()))
    {
	cout << "MF ";
	delete (uint8_t*)(v.Ptr());
    }
    if(0 == r->getField("IPV4_FRAG", v))
    {
	cout << "FRAG=" << *(uint16_t*)(v.Ptr()) << " ";
	delete (uint16_t*)(v.Ptr());
    }
    if(0 == r->getField("IPV4_OPT", v))
    {
	cout << "OPT (";
	for(int i = 0; i < v.Bytes(); i++)
	{
	    char *ptr = (char*)(v.Ptr());
	    cout << hex << uppercase << ((*(uint8_t*)(ptr + i) & 0xF0) >> 4)
		<< (*(uint8_t*)(ptr + i) & 0x0F) << nouppercase << dec;
	}
	cout << ") ";
	delete[] (char*)(v.Ptr());
    }

    // transport layer type
    switch(transType)
    {
	case TRANS_TCP:
	    cout << "PROTO=TCP ";
	    putTCPHeaderStr(r);
	    break;
	case TRANS_UDP:
	    cout << "PROTO=UDP ";
	    putUDPHeaderStr(r);
	    break;
	case TRANS_ICMP:
	    cout << "PROTO=ICMP ";
	    putICMPHeaderStr(r);
	    break;
	case TRANS_UNKNOWN:
	    putGenericDataStr(r);
	    break;
	default:
	    return 1;
	    break;
    }
    
    cout << endl;
    return 0;
}

// int putTCPHeaderStr(Record * r)
// Unpack the TCP portion of a record and print it
//
// Parameters: r, the record to unpack
// Returns: 0 on success, 1 on failure
// Modifies: Data is removed from r and written to the output file
int IptableParser::putTCPHeaderStr(Record * r)
{
    Value v;

    if(r->getField("TCP_INCOMPLETE", v) == 0)
    {
	cout << "INCOMPLETE [" << *(uint32_t*)(v.Ptr()) << " bytes] ";
	delete (uint32_t*)(v.Ptr());
	return 0;
    }

    r->getField("TCP_SRC_PORT", v);
    cout << "SPT=" << *(uint16_t*)(v.Ptr()) << " ";
    delete (uint16_t*)(v.Ptr());

    r->getField("TCP_DST_PORT", v);
    cout << "DPT=" << *(uint16_t*)(v.Ptr()) << " ";
    delete (uint16_t*)(v.Ptr());

    r->getField("TCP_SEQUENCE", v);
    cout << "SEQ=" << *(uint32_t*)(v.Ptr()) << " ";
    delete (uint32_t*)(v.Ptr());

    r->getField("TCP_ACK_NO", v);
    cout << "ACK=" << *(uint32_t*)(v.Ptr()) << " ";
    delete (uint32_t*)(v.Ptr());

    r->getField("TCP_WINDOW", v);
    cout << "WINDOW=" << *(uint16_t*)(v.Ptr()) << " ";
    delete (uint16_t*)(v.Ptr());

    r->getField("TCP_RESERVED", v);
    cout << "RES=0x" << hex << (uint32_t)(*(uint8_t*)(v.Ptr()) >> 4)
	<< (uint32_t)(*(uint8_t*)(v.Ptr()) & 0x0F) << " " << dec;
    delete (uint8_t*)(v.Ptr());

    // optional flags
    if(0 == r->getField("TCP_FLAG_URGENT", v) && 1 == *(uint8_t*)(v.Ptr()))
    {
	cout << "URG ";
	delete (uint8_t*)(v.Ptr());
    }
    if(0 == r->getField("TCP_FLAG_ACK", v) && 1 == *(uint8_t*)(v.Ptr()))
    {
	cout << "ACK ";
	delete (uint8_t*)(v.Ptr());
    }
    if(0 == r->getField("TCP_FLAG_PSH", v) && 1 == *(uint8_t*)(v.Ptr()))
    {
	cout << "PSH ";
	delete (uint8_t*)(v.Ptr());
    }
    if(0 == r->getField("TCP_FLAG_RST", v) && 1 == *(uint8_t*)(v.Ptr()))
    {
	cout << "RST ";
	delete (uint8_t*)(v.Ptr());
    }
    if(0 == r->getField("TCP_FLAG_SYN", v) && 1 == *(uint8_t*)(v.Ptr()))
    {
	cout << "SYN ";
	delete (uint8_t*)(v.Ptr());
    }
    if(0 == r->getField("TCP_FLAG_FIN", v) && 1 == *(uint8_t*)(v.Ptr()))
    {
	cout << "FIN ";
	delete (uint8_t*)(v.Ptr());
    }

    // back to our normal programming
    r->getField("TCP_URGENT_PTR", v);
    cout << "URGP=" << *(uint32_t*)(v.Ptr()) << " ";
    delete (uint32_t*)(v.Ptr());

    if(0 == r->getField("TCP_OPTIONS", v))
    {
	cout << "OPT (";
	for(int i = 0; i < v.Bytes(); i++)
	{
	    char *ptr = (char*)(v.Ptr());
	    cout << hex << uppercase << ((*(uint8_t*)(ptr + i) & 0xF0) >> 4)
		<< (*(uint8_t*)(ptr + i) & 0x0F) << nouppercase << dec;
	}
	cout << ") ";
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
int IptableParser::putUDPHeaderStr(Record * r)
{
    Value v;

    r->getField("UDP_SRC_PORT", v);
    cout << dec << "SPT=" << *(uint16_t*)(v.Ptr()) << " ";
    delete (uint16_t*)(v.Ptr());

    r->getField("UDP_DST_PORT", v);
    cout << "DPT=" << *(uint16_t*)(v.Ptr()) << " ";
    delete (uint16_t*)(v.Ptr());

    r->getField("UDP_LENGTH", v);
    cout << "LEN=" << *(uint16_t*)(v.Ptr()) << " ";
    delete (uint16_t*)(v.Ptr());
    
    return 0;
}

// int putICMPHeader(Record * r)
// Unpack and output an ICMP header
//
// Parameters: r, the record to unpack
// Returns: 0 on success, 1 on failure
// Modifies: data is written to the output file and r has data destroyed
int IptableParser::putICMPHeaderStr(Record * r)
{
    Value v;

    if(0 == r->getField("ICMP_TOTAL_INCOMPLETE", v))
    {
	cout << "INCOMPLETE [" << *(uint32_t*)(v.Ptr())
	    << " bytes] ";
	delete (uint32_t*)(v.Ptr());
	return 0;
    }

    r->getField("ICMP_TYPE", v);
    cout << "TYPE=" << (uint32_t)(*(uint8_t*)(v.Ptr())) << " ";
    delete (uint8_t*)(v.Ptr());

    r->getField("ICMP_CODE", v);
    cout << "CODE=" << (uint32_t)(*(uint8_t*)(v.Ptr())) << " ";
    delete (uint8_t*)(v.Ptr());

    // incomplete
    if(0 == r->getField("ICMP_INCOMPLETE", v))
    {
	cout << "INCOMPLETE [" << *(uint32_t*)(v.Ptr()) << " bytes] ";
	delete (uint32_t*)(v.Ptr());
	return 0;
    }

    // external ID and SEQ fields but no additional data
    else if(0 == r->getField("ICMP_EXT_ID", v))
    {
	cout << "ID=" << *(uint32_t*)(v.Ptr()) << " ";
	delete (uint32_t*)(v.Ptr());
	
	r->getField("ICMP_EXT_SEQ", v);
	cout << "SEQ=" << *(uint32_t*)(v.Ptr()) << " ";
	delete (uint32_t*)(v.Ptr());
    }

    else if(0 == r->getField("ICMP_SRC_IP", v))
    {
	r->getField("ICMP_SRC_IP", v);
	struct in_addr ipaddr;
	ipaddr.s_addr = htonl(*(uint32_t*)(v.Ptr()));
	cout << "[SRC=" << inet_ntoa(ipaddr) << " ";
	delete (uint32_t*)(v.Ptr());

	r->getField("ICMP_DST_IP", v);
	ipaddr.s_addr = htonl(*(uint32_t*)(v.Ptr()));
	cout << "DST=" << inet_ntoa(ipaddr) << " ";
	delete (uint32_t*)(v.Ptr());

	// IP length
	r->getField("ICMP_LENGTH", v);
	cout << "LEN=" << *(uint16_t*)(v.Ptr()) << " ";
	delete (uint16_t*)(v.Ptr());

	// IP TOS
	r->getField("ICMP_TOS", v);
	cout << hex << "TOS=0x" << ((*(uint8_t*)(v.Ptr()) & 0xF0) >> 4)
	    << (*(uint8_t*)(v.Ptr()) & 0x0F) << " ";
	delete (uint8_t*)(v.Ptr());
	
	// IP precision
	r->getField("ICMP_PRECISION", v);
	cout << hex << 
	    uppercase <<
	    "PREC=0x" << ((*(uint8_t*)(v.Ptr()) & 0xF0) >> 4)
	    << (*(uint8_t*)(v.Ptr()) & 0x0F) << nouppercase << " ";
	delete (uint8_t*)(v.Ptr());    

	// IP TTL
	r->getField("ICMP_TTL", v);
	cout << dec << "TTL=" << (uint32_t)*(uint8_t*)(v.Ptr()) << " ";
	delete (uint8_t*)(v.Ptr());

	// IP ID
	r->getField("ICMP_ID", v);
	cout << "ID=" << *(uint16_t*)(v.Ptr()) << " ";
	delete (uint16_t*)(v.Ptr());

	// various flags, if they exist and were not anonymized
	if(0 == r->getField("ICMP_CE", v) && 1 == *(uint8_t*)(v.Ptr()))
	{
	    cout << "CE ";
	    delete (uint8_t*)(v.Ptr());
	}
	if(0 == r->getField("ICMP_DF", v) && 1 == *(uint8_t*)(v.Ptr()))
	{
	    cout << "DF ";
	    delete (uint8_t*)(v.Ptr());
	}
	if(0 == r->getField("ICMP_MF", v) && 1 == *(uint8_t*)(v.Ptr()))
	{
	    cout << "MF ";
	    delete (uint8_t*)(v.Ptr());
	}
	if(0 == r->getField("ICMP_FRAG", v))
	{
	    cout << "FRAG=" << *(uint16_t*)(v.Ptr()) << " ";
	    delete (uint16_t*)(v.Ptr());
	}
	if(0 == r->getField("ICMP_OPT", v))
	{
	    cout << "OPT (";
	    for(int i = 0; i < v.Bytes(); i++)
	    {
		char *ptr = (char*)(v.Ptr());
		cout << hex << uppercase << 
		    ((*(uint8_t*)(ptr + i) & 0xF0) >> 4)
		    << (*(uint8_t*)(ptr + i) & 0x0F) << nouppercase << dec;
	    }
	    cout << ") ";
	    delete[] (char*)(v.Ptr());
	}

	// determine the protocol type
	r->getField("ICMP_TRANS_TYPE", v);
	uint8_t ttype = *(uint8_t*)(v.Ptr());
	delete (uint8_t*)(v.Ptr());

	if(ttype == TRANS_TCP)
	{
	    cout << "PROTO=TCP ";
	    
	    if(r->getField("ICMP_TCP_INCOMPLETE", v) == 0)
	    {
		cout << "INCOMPLETE [" << 
		    *(uint32_t*)(v.Ptr()) << " bytes] ";
		delete (uint32_t*)(v.Ptr());
	    }
	    else {
	    r->getField("ICMP_TCP_SRC_PORT", v);
	    cout << "SPT=" << *(uint16_t*)(v.Ptr()) << " ";
	    delete (uint16_t*)(v.Ptr());

	    r->getField("ICMP_TCP_DST_PORT", v);
	    cout << "DPT=" << *(uint16_t*)(v.Ptr()) << " ";
	    delete (uint16_t*)(v.Ptr());

	    r->getField("ICMP_TCP_SEQUENCE", v);
	    cout << "SEQ=" << *(uint32_t*)(v.Ptr()) << " ";
	    delete (uint32_t*)(v.Ptr());

	    r->getField("ICMP_TCP_ACK_NO", v);
	    cout << "ACK=" << *(uint32_t*)(v.Ptr()) << " ";
	    delete (uint32_t*)(v.Ptr());

	    r->getField("ICMP_TCP_WINDOW", v);
	    cout << "WINDOW=" << *(uint16_t*)(v.Ptr()) << " ";
	    delete (uint16_t*)(v.Ptr());

	    r->getField("ICMP_TCP_RESERVED", v);
	    cout << "RES=0x" << hex << (uint32_t)(*(uint8_t*)(v.Ptr()) >> 4)
		<< (uint32_t)(*(uint8_t*)(v.Ptr()) & 0x0F) << " " << dec;
	    delete (uint8_t*)(v.Ptr());

	    // optional flags
	    if(0 == r->getField("ICMP_TCP_FLAG_URGENT", v) && 1 == 
		    *(uint8_t*)(v.Ptr()))
	    {
		cout << "URG ";
		delete (uint8_t*)(v.Ptr());
	    }
	    if(0 == r->getField("ICMP_TCP_FLAG_ACK", v) && 1 == 
		    *(uint8_t*)(v.Ptr()))
	    {
		cout << "ACK ";
		delete (uint8_t*)(v.Ptr());
	    }
	    if(0 == r->getField("ICMP_TCP_FLAG_PSH", v) && 1 == 
		    *(uint8_t*)(v.Ptr()))
	    {
		cout << "PSH ";
		delete (uint8_t*)(v.Ptr());
	    }
	    if(0 == r->getField("ICMP_TCP_FLAG_RST", v) && 1 == 
		    *(uint8_t*)(v.Ptr()))
	    {
		cout << "RST ";
		delete (uint8_t*)(v.Ptr());
	    }
	    if(0 == r->getField("ICMP_TCP_FLAG_SYN", v) && 1 == 
		    *(uint8_t*)(v.Ptr()))
	    {
		cout << "SYN ";
		delete (uint8_t*)(v.Ptr());
	    }
	    if(0 == r->getField("ICMP_TCP_FLAG_FIN", v) && 1 == 
		    *(uint8_t*)(v.Ptr()))
	    {
		cout << "FIN ";
		delete (uint8_t*)(v.Ptr());
	    }

	    // back to our normal programming
	    r->getField("ICMP_TCP_URGENT_PTR", v);
	    cout << "URGP=" << *(uint32_t*)(v.Ptr()) << " ";
	    delete (uint32_t*)(v.Ptr());

	    if(0 == r->getField("ICMP_TCP_OPTIONS", v))
	    {
		cout << "OPT (";
		for(int i = 0; i < v.Bytes(); i++)
		{
		    char *ptr = (char*)(v.Ptr());
		    cout << hex << uppercase << 
			((*(uint8_t*)(ptr + i) & 0xF0) >> 4)
			<< (*(uint8_t*)(ptr + i) & 0x0F) << nouppercase << dec;
		}
		cout << ") ";
		delete[] (char*)(v.Ptr());
	    }
	    }
	} // if(internal protocol is TCP)
	else if(ttype == TRANS_UDP)
	{
	    cout << "PROTO=UDP ";
	    
	    r->getField("ICMP_UDP_SRC_PORT", v);
	    cout << dec << "SPT=" << *(uint16_t*)(v.Ptr()) << " ";
	    delete (uint16_t*)(v.Ptr());

	    r->getField("ICMP_UDP_DST_PORT", v);
	    cout << "DPT=" << *(uint16_t*)(v.Ptr()) << " ";
	    delete (uint16_t*)(v.Ptr());

	    r->getField("ICMP_UDP_LENGTH", v);
	    cout << "LEN=" << *(uint16_t*)(v.Ptr()) << " ";
	    delete (uint16_t*)(v.Ptr());
	}
	else if(ttype == TRANS_ICMP)
	{
	    cout << "PROTO=ICMP ";

	    r->getField("ICMP_TYPE_INTERNAL", v);
	    cout << "TYPE=" << (uint32_t)(*(uint8_t*)(v.Ptr())) << " ";
	    delete (uint8_t*)(v.Ptr());

	    r->getField("ICMP_CODE_INTERNAL", v);
	    cout << "CODE=" << (uint32_t)(*(uint8_t*)(v.Ptr())) << " ";
	    delete (uint8_t*)(v.Ptr());

	    r->getField("ICMP_ID_INTERNAL", v);
	    cout << "ID=" << *(uint32_t*)(v.Ptr()) << " ";
	    delete (uint32_t*)(v.Ptr());

	    r->getField("ICMP_SEQ_INTERNAL", v);
	    cout << "SEQ=" << *(uint32_t*)(v.Ptr()) << " ";
	    delete (uint32_t*)(v.Ptr());
	}
	else
	    return 1;
	cout << "] ";
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
int IptableParser::putGenericDataStr(Record * r)
{
    Value v;

    r->getField("SYS_TRANS_TYPE_STR", v);
    cout << "PROTO=" << *(string*)(v.Ptr()) << " ";
    delete (string*)(v.Ptr());

    r->getField("SYS_GENERIC_DATA", v);
    cout << *(string*)(v.Ptr());
    delete (string*)(v.Ptr());

    return 0;
}

