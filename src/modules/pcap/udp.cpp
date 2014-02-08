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
// $Id: udp.cpp 763 2007-04-14 06:40:47Z byoo3 $
//
// UDP layer functionality for pcap module 
//
//********************************************************************

#include <iostream>
#include <fstream>
#include <netinet/in.h>

#include "record.h"
#include "map_record.h"
#include "value.h"
#include "value_types.h"
#include "libpcap_structs.h"
#include "libpcap_parser.h"

// int getUDPHeader(Record * r)
// Pack a UDP header into a record
//
// Parameters: r, the record to pack
// Returns: the number of bytes read by this operation
// Modifies: packs data into r and moves the get pointer
int LibpcapParser::getUDPHeader(Record * r) throw (exception&, ifstream::failure&)
{
    udp_hdr_t *uhdr = new udp_hdr_t;
    infile.read((char*)uhdr, sizeof(udp_hdr_t));

    // switch to host byte order
    uhdr->src_port = ntohs(uhdr->src_port);
    uhdr->dst_port = ntohs(uhdr->dst_port);
    uhdr->length = ntohs(uhdr->length);
    uhdr->checksum = ntohs(uhdr->checksum);

    // pack the record
    r->setField("UDP_SRC_PORT", Value(&uhdr->src_port, VAL_TYPE_UINT16, 2));
    r->setField("UDP_DST_PORT", Value(&uhdr->dst_port, VAL_TYPE_UINT16, 2));
    r->setField("UDP_LENGTH", Value(&uhdr->length, VAL_TYPE_UINT16, 2));
    r->setField("UDP_CHECKSUM", Value(&uhdr->checksum, VAL_TYPE_UINT16, 2));

    r->setField("SYS_UDP_HDR_LOC", Value(uhdr, VAL_TYPE_CHAR_PTR,
		sizeof(udp_hdr_t)));

    return sizeof(udp_hdr_t);
}

// int putUDPHeader(Record * r)
// Unpack and write a UDP header
//
// Parameters: r, the record to unpack
// Returns: 0 on success
// Modifies: the put pointer, output file, and record
int LibpcapParser::putUDPHeader(Record * r)
{
    Value v;

    r->getField("SYS_UDP_HDR_LOC", v);
    udp_hdr_t *uhdr = (udp_hdr_t*)(v.Ptr());

    uhdr->src_port = htons(uhdr->src_port);
    uhdr->dst_port = htons(uhdr->dst_port);
    uhdr->length = htons(uhdr->length);
    uhdr->checksum = htons(uhdr->checksum);

    outfile.write((char*)uhdr, sizeof(udp_hdr_t));
    delete uhdr;
    return 0;
}
