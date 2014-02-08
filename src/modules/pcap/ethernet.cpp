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
// $Id: ethernet.cpp 763 2007-04-14 06:40:47Z byoo3 $
//
// Ethernet layer-related code for pcap module 
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

// int getEthernetHeader(Record * r)
// Read the fields in an Ethernet header and pack them into a Record
//
// Parameters: r, the record to pack into
// Returns: The number of bytes read by this operation
// Modifies: Packs data into r; moves the get pointer for the input stream
int LibpcapParser::getEthernetHeader(Record * r) throw (exception&, ifstream::failure&)
{
    // initialize and read the header
    ethernet_hdr_t *ehdr = new ethernet_hdr_t;
    infile.read((char*)ehdr, sizeof(ethernet_hdr_t));
    //is->read((char*)ehdr, sizeof(ethernet_hdr_t));

    // switch to host byte order
    ehdr->type = ntohs(ehdr->type);
    netTypeGet = ehdr->type;

    // pack into record
    r->setField("DST_MAC", Value(&ehdr->dest, VAL_TYPE_CHAR_PTR, 6));
    r->setField("SRC_MAC", Value(&ehdr->src, VAL_TYPE_CHAR_PTR, 6));
    r->setField("SYS_NET_TYPE", Value(&ehdr->type, VAL_TYPE_UINT16, 2));

    // make note of where the record is
    r->setField("SYS_LINK_LOC", Value(ehdr, VAL_TYPE_CHAR_PTR,
		sizeof(ethernet_hdr_t)));

    return sizeof(ethernet_hdr_t);
}

// int putEthernetHeader(Record * r)
// Unpacks the Ethernet-related fields of a record and writes them to disk
//
// Parameters: r, the record to unpack
// Returns: 0 on success
// Modifies: put pointer, memory is deallocated
int LibpcapParser::putEthernetHeader(Record * r)
{
    Value v;

    // get a pointer to the ethernet header in dynamic memory
    r->getField("SYS_LINK_LOC", v);

    // switch to network byte order
    ethernet_hdr_t *ehdr = (ethernet_hdr_t*)(v.Ptr());
    netTypePut = ehdr->type;
    ehdr->type = htons(ehdr->type);

    // write the data to disk
    outfile.write((char*)(v.Ptr()), sizeof(ethernet_hdr_t));
    
    // clean up
    delete ehdr;

    return 0;
}
