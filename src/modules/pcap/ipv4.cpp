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
// $Id: ipv4.cpp 763 2007-04-14 06:40:47Z byoo3 $
//
// IP version 4 layer-related code for pcap module 
//
//********************************************************************

#include <istream>
#include <iostream>
#include <fstream>
#include <netinet/in.h>

#include "record.h"
#include "map_record.h"
#include "value.h"
#include "value_types.h"
#include "libpcap_structs.h"
#include "libpcap_parser.h"

// int getIPv4Header(Record * r)
// Pack an IPv4 header into a record
//
// Parameters: r, the record to pack into
// Returns: the number of bytes read
// Modifies: adds fields to the record and moves the get pointer
int LibpcapParser::getIPv4Header(Record * r) throw (exception&, ifstream::failure&)
{
    // read IPv4 data
    ipv4_hdr_t *ihdr = new ipv4_hdr_t;
    infile.read((char*)ihdr, sizeof(ipv4_hdr_t));
    // is->read((char*)ihdr, sizeof(ipv4_hdr_t));

    // convert to host byte order
    ihdr->length = ntohs(ihdr->length);
    ihdr->id = ntohs(ihdr->id);
    ihdr->offset = ntohs(ihdr->offset);
    ihdr->checksum = ntohs(ihdr->checksum);
    ihdr->src_ip = ntohl(ihdr->src_ip);
    ihdr->dst_ip = ntohl(ihdr->dst_ip);

    transTypeGet = ihdr->protocol;

    // write to the record
    r->setField("IPV4_VHL", Value(&ihdr->vhl, VAL_TYPE_UINT8, 1));
    r->setField("IPV4_TYPE", Value(&ihdr->type, VAL_TYPE_UINT8, 1));
    r->setField("IPV4_LENGTH", Value(&ihdr->length, VAL_TYPE_UINT16, 2));
    r->setField("IPV4_ID", Value(&ihdr->id, VAL_TYPE_UINT16, 2));
    r->setField("IPV4_OFFSET", Value(&ihdr->offset, VAL_TYPE_UINT16, 2));
    r->setField("IPV4_TTL", Value(&ihdr->ttl, VAL_TYPE_UINT8, 1));
    r->setField("IPV4_PROTOCOL", Value(&ihdr->protocol, VAL_TYPE_UINT8, 1));
    r->setField("IPV4_CHECKSUM", Value(&ihdr->checksum, VAL_TYPE_UINT16, 2));
    r->setField("IPV4_SRC_IP", Value(&ihdr->src_ip, VAL_TYPE_UINT32, 4));
    r->setField("IPV4_DST_IP", Value(&ihdr->dst_ip, VAL_TYPE_UINT32, 4));
    r->setField("SYS_IPV4_HDR_LOC", Value(ihdr, VAL_TYPE_CHAR_PTR,
		sizeof(ipv4_hdr_t)));
    
    // if there are any options, read those as well
    int optlen = 0;
    int headlen = ihdr->vhl & 0x0F;
    if(headlen > 5)
    {
	optlen = 4 * (headlen - 5);
	char *buf = new char[optlen];
	infile.read(buf, optlen);
	// is->read(buf, optlen);
	r->setField("IPV4_OPTIONS", Value(buf, VAL_TYPE_CHAR_PTR, optlen));
    }

    return (sizeof(ipv4_hdr_t) + optlen);
}

// int putIPv4Header(record * r)
// Unpacks the IPv4-related fields of a record and writes them to disk
//
// Parameters: r, the record to unpack
// Returns: 0 on success
// Modifies: the put pointer is moved and memory is deallocated from the
// record
int LibpcapParser::putIPv4Header(Record * r)
{
    Value v;

    // get a pointer to the header in question
    r->getField("SYS_IPV4_HDR_LOC", v);
    ipv4_hdr_t *ihdr = (ipv4_hdr_t*)(v.Ptr());

    // switch to network byte order    
    ihdr->length = htons(ihdr->length);
    ihdr->id = htons(ihdr->id);
    ihdr->offset = htons(ihdr->offset);
    ihdr->checksum = htons(ihdr->checksum);
    ihdr->src_ip = htonl(ihdr->src_ip);
    ihdr->dst_ip = htonl(ihdr->dst_ip);

    transTypePut = ihdr->protocol;
    
    // write the record
    outfile.write((char*)ihdr, sizeof(ipv4_hdr_t));
    
    // write options if they exist
    if(!r->getField("IPV4_OPTIONS", v))
    {
	outfile.write((char*)(v.Ptr()), v.Bytes());
	delete[] (char*)(v.Ptr());
    }

    delete ihdr;
    return 0;
}
