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
// $Id: nfd_structs.h 263 2006-08-03 17:59:20Z gcolombo $
//
// Defines structures which are useful in crafting records for the
// nfdump module 
//
//********************************************************************

#ifndef _NFDUMP_STRUCTS_H
#define _NFDUMP_STRUCTS_H

#include <stdint.h>

#if ( SIZEOF_VOID_P == 8 )
	typedef uint64_t	pointer_addr_t;
#else
	typedef uint32_t	pointer_addr_t;
#endif

typedef struct nfv5_hdr_t {
    uint16_t version;	// export format version number
    uint16_t count;	// number of flows exported
    uint32_t uptime;	// time since export device booted
    uint32_t unix_secs;	// seconds since 0000 UTC 1970
    uint32_t unix_nsecs;// residual nanoseconds
    uint32_t seq;	// sequence counter of flows seen
    uint8_t engine_type;// type of flow-switching engine
    uint8_t engine_id;	// slot number of flow-switching engine
    uint16_t reserved;	// must be 0
} nfv5_hdr_t;

typedef struct nfv5_rcd_t {
    uint32_t srcaddr;	// source IP address
    uint32_t dstaddr;	// destination IP address
    uint32_t nexthop;	// IP address of next hop router
    uint16_t input;	// SNMP index of input interface
    uint16_t output;	// SNMP index of output interface
    uint32_t dpkts;	// packets in the flow
    uint32_t doctets;	// layer 3 bytes in packets
    uint32_t first;	// uptime at start of flow
    uint32_t last;	// uptime at end of flow
    uint16_t srcport;	// source port
    uint16_t dstport;	// destination port
    uint8_t pad1;	// unused, reserved as 0 bytes
    uint8_t tcp_flags;	// cumulative OR of TCP flags
    uint8_t prot;	// IP protocol type
    uint8_t tos;	// IP type of service
    uint16_t src_as;	// autonomous system number of source
    uint16_t dst_as;	// autonomous system number of destination
    uint8_t src_mask;	// source address prefix mask bits
    uint8_t dst_mask;	// destination address prefix mask bits
    uint16_t pad2;	// unused, reserved as 0 bytes
} nfv5_rcd_t;

typedef struct nfv7_hdr_t {
    uint16_t version;
    uint16_t count;
    uint32_t uptime;
    uint32_t unix_secs;
    uint32_t unix_nsecs;
    uint32_t seq;
    uint32_t reserved;
} nfv7_hdr_t;

typedef struct nfv7_rcd_t {
    uint32_t srcaddr;
    uint32_t dstaddr;
    uint32_t nexthop;
    uint16_t input;
    uint16_t output;
    uint32_t dpkts;
    uint32_t doctets;
    uint32_t first;
    uint32_t last;
    uint16_t srcport;
    uint16_t dstport;
    uint8_t flags1;
    uint8_t tcp_flags;
    uint8_t prot;
    uint8_t tos;
    uint16_t src_as;
    uint16_t dst_as;
    uint8_t src_mask;
    uint8_t dst_mask;
    uint16_t flags2;
    uint32_t router_sc;
} nfv7_rcd_t;

#endif // _NFDUMP_STRUCTS_H
