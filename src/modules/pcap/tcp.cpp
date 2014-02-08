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
// $Id: tcp.cpp 763 2007-04-14 06:40:47Z byoo3 $
//
// TCP transport layer-related code for pcap module 
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

// int getTCPHeader(Record * r)
// Pack a TCP header into a record
//
// Parameters: r, the record to pack
// Returns: The number of bytes read
// Modifies: Packs data into r and moves the get pointer
int LibpcapParser::getTCPHeader(Record * r) throw (exception&, ifstream::failure&)
{
    // read data
    tcp_hdr_t *thdr = new tcp_hdr_t;
    infile.read((char*)(thdr), sizeof(tcp_hdr_t));

    // switch to host byte order
    thdr->src_port = ntohs(thdr->src_port);
    thdr->dst_port = ntohs(thdr->dst_port);
    thdr->sequence = ntohl(thdr->sequence);
    thdr->ack_no = ntohl(thdr->ack_no);
    thdr->window = ntohs(thdr->window);
    thdr->checksum = ntohs(thdr->checksum);
    thdr->urgent = ntohs(thdr->urgent);

    // pack into record
    r->setField("TCP_SRC_PORT", Value(&thdr->src_port, VAL_TYPE_UINT16, 2));
    r->setField("TCP_DST_PORT", Value(&thdr->dst_port, VAL_TYPE_UINT16, 2));
    r->setField("TCP_SEQUENCE", Value(&thdr->sequence, VAL_TYPE_UINT32, 4));
    r->setField("TCP_ACK_NO", Value(&thdr->ack_no, VAL_TYPE_UINT32, 4));
    r->setField("TCP_OFFSET", Value(&thdr->offset, VAL_TYPE_UINT8, 1));
    r->setField("TCP_FLAGS", Value(&thdr->flags, VAL_TYPE_UINT8, 1));
    r->setField("TCP_WINDOW", Value(&thdr->window, VAL_TYPE_UINT16, 2));
    r->setField("TCP_CHECKSUM", Value(&thdr->checksum, VAL_TYPE_UINT16, 2));
    r->setField("TCP_URGENT", Value(&thdr->urgent, VAL_TYPE_UINT16, 2));

    // pack options if they exist
    int optlen = 0;
    int offset = ((thdr->offset) & 0xF0) >> 4;
    if(offset > 5)
    {
	optlen = (offset - 5) * 4;
	char *buf = new char[optlen];
	infile.read(buf, optlen);
	// is->read(buf, optlen);
	r->setField("TCP_OPTIONS", Value(buf, VAL_TYPE_CHAR_PTR, optlen));
    }

    r->setField("SYS_TCP_HDR_LOC", Value(thdr, VAL_TYPE_CHAR_PTR,
		sizeof(tcp_hdr_t)));
    return (sizeof(tcp_hdr_t) + optlen);
}

// int putTCPHeader(Record * r)
// Unpack and write a tcp header from a record
//
// parameters: r, the record to unpack
// returns: 0 on success, 1 on failure
// modifies: unpacks and deallocates data
int LibpcapParser::putTCPHeader(Record * r)
{
    Value v;

    r->getField("SYS_TCP_HDR_LOC", v);
    tcp_hdr_t *thdr = (tcp_hdr_t*)(v.Ptr());

    thdr->src_port = htons(thdr->src_port);
    thdr->dst_port = htons(thdr->dst_port);
    thdr->sequence = htonl(thdr->sequence);
    thdr->ack_no = htonl(thdr->ack_no);
    thdr->window = htons(thdr->window);
    thdr->checksum = htons(thdr->checksum);
    thdr->urgent = htons(thdr->urgent);

    outfile.write((char*)thdr, sizeof(tcp_hdr_t));

    if(0 == r->getField("TCP_OPTIONS", v))
    {
	outfile.write((char*)(v.Ptr()), v.Bytes());
	delete[] (char*)(v.Ptr());
    }

    delete thdr;
    return 0;
}
