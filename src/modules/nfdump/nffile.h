/*
 *  This file is part of the nfdump project.
 *
 *  Copyright (c) 2004, SWITCH - Teleinformatikdienste fuer Lehre und Forschung
 *  All rights reserved.
 *  
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions are met:
 *  
 *   * Redistributions of source code must retain the above copyright notice, 
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright notice, 
 *     this list of conditions and the following disclaimer in the documentation 
 *     and/or other materials provided with the distribution.
 *   * Neither the name of SWITCH nor the names of its contributors may be 
 *     used to endorse or promote products derived from this software without 
 *     specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *  POSSIBILITY OF SUCH DAMAGE.
 *  
 *  $Author: peter $
 *
 *  $Id: nffile.h 77 2006-06-14 14:52:25Z peter $
 *
 *  $LastChangedRevision: 77 $
 *	
 */

#ifndef _NFFILE_H
#define _NFFILE_H

#define IdentLen	128
#define IdentNone	"none"

/*
 *
 * bit 0: 
 */
typedef struct file_header_s {
	uint16_t	magic;				// magic to recognize endian type
	uint16_t	version;			// version of binary file layout, incl. magic
	uint32_t	flags;				// 
	uint32_t	NumBlocks;			// number of blocks in file
	char		ident[IdentLen];	// identifies this data
} file_header_t;

typedef struct stat_record_s {
	// overall stat
	uint64_t	numflows;
	uint64_t	numbytes;
	uint64_t	numpackets;
	// flow stat
	uint64_t	numflows_tcp;
	uint64_t	numflows_udp;
	uint64_t	numflows_icmp;
	uint64_t	numflows_other;
	// bytes stat
	uint64_t	numbytes_tcp;
	uint64_t	numbytes_udp;
	uint64_t	numbytes_icmp;
	uint64_t	numbytes_other;
	// packet stat
	uint64_t	numpackets_tcp;
	uint64_t	numpackets_udp;
	uint64_t	numpackets_icmp;
	uint64_t	numpackets_other;
	// time window
	uint32_t	first_seen;
	uint32_t	last_seen;
	uint16_t	msec_first;
	uint16_t	msec_last;
	// other
	uint32_t	sequence_failure;
} stat_record_t;

typedef struct data_block_header_s {
	uint32_t	NumBlocks;		// number of data records in data block
	uint32_t	size;			// number of this block in bytes without this header
	uint16_t	id;				// flags for this block
	uint16_t	pad;
} data_block_header_t;

#define DATA_BLOCK_TYPE_1	1

typedef struct common_record_s {
	// the head of each data record
	uint32_t	flags;
	uint16_t	size;
	uint16_t	mark;
	uint16_t	msec_first;
	uint16_t	msec_last;
	uint32_t	first;
	uint32_t	last;

	uint8_t		dir;
	uint8_t		tcp_flags;
	uint8_t		prot;
	uint8_t		tos;
	uint16_t	input;
	uint16_t	output;
	uint16_t	srcport;
	uint16_t	dstport;
	uint16_t	srcas;
	uint16_t	dstas;
	uint8_t		data[4];	// .. more data below
} common_record_t;

#define BYTE_OFFSET_first	12

// number of netflow elements in data_block_record_t: first .. dOctets
#define NumElements 18

typedef struct ipv4_block_s {
#ifdef WORDS_BIGENDIAN
	uint32_t	fill1[3];
	uint32_t	srcaddr;
	uint32_t	fill2[3];
	uint32_t	dstaddr;
#else
	uint32_t	fill1[2];
	uint32_t	srcaddr;
	uint32_t	fill2;
	uint32_t	fill3[2];
	uint32_t	dstaddr;
	uint32_t	fill4;
#endif
} ipv4_block_t;

typedef struct ipv6_block_s {
	uint64_t	srcaddr[2];
	uint64_t	dstaddr[2];
} ipv6_block_t;

typedef struct ip_block_s {
	union {
		ipv4_block_t	_v4;
		ipv6_block_t	_v6;
	} ip_union;
#define v4 ip_union._v4
#define v6 ip_union._v6
	uint8_t		data[4];	// .. more data below
} ip_block_t;

typedef struct value32_s {
	uint32_t	val;
	uint8_t		data[4];	// .. more data below
} value32_t;

typedef struct value64_s {
	union val_s {
		uint64_t	val64;
		uint32_t	val32[2];
	} val;
	uint8_t		data[4];	// .. more data below
} value64_t;

/* the master record contains all possible records unpacked */
typedef struct master_record_s {
	// common information from all netflow versions
	// 							// interpreted as uint64_t[]
	uint32_t	flags;			// index 0	0xffff'ffff'0000'0000
	uint16_t	size;			// index 0	0x0000'0000'ffff'0000
	uint16_t	mark;			// index 0	0x0000'0000'0000'ffff
	uint16_t	msec_first;		// index 1	0xffff'0000'0000'0000
	uint16_t	msec_last;		// index 1	0x0000'ffff'0000'0000
	uint32_t	first;			// index 1	0x0000'0000'ffff'ffff
	uint32_t	last;			// index 2	0xffff'ffff'0000'0000
	uint8_t		dir;			// index 2	0x0000'0000'ff00'0000
	uint8_t		tcp_flags;		// index 2  0x0000'0000'00ff'0000
	uint8_t		prot;			// index 2  0x0000'0000'0000'ff00
	uint8_t		tos;			// index 2  0x0000'0000'0000'00ff
	uint16_t	input;			// index 3	0xffff'0000'0000'0000
	uint16_t	output;			// index 3  0x0000'ffff'0000'0000
	uint16_t	srcport;		// index 3	0x0000'0000'ffff'0000
	uint16_t	dstport;		// index 3	0x0000'0000'0000'ffff
	uint16_t	srcas;			// index 4	0xffff'0000'0000'0000
	uint16_t	dstas;			// index 4	0x0000'ffff'0000'0000
	// align 64bit
	uint32_t	fill;			// index 4	0x0000'0000'ffff'ffff

	// IP address block 
	union {						
		ipv4_block_t	_v4;	// srcaddr      index 6 0x0000'0000'ffff'ffff
								// dstaddr      index 8 0x0000'0000'ffff'ffff
		ipv6_block_t	_v6;	// srcaddr[0-1] index 5 0xffff'ffff'ffff'ffff
								// srcaddr[2-3] index 6 0xffff'ffff'ffff'ffff
								// dstaddr[0-1] index 7 0xffff'ffff'ffff'ffff
								// dstaddr[2-3] index 8 0xffff'ffff'ffff'ffff
	} ip_union;

	// counter block - expanded to 8 bytes
	uint64_t	dPkts;			// index 9	0xffff'ffff'ffff'ffff
	uint64_t	dOctets;		// index 10 0xffff'ffff'ffff'ffff

} master_record_t;

typedef struct type_mask_s {
	union {
		uint8_t		val8[8];
		uint16_t	val16[4];
		uint32_t	val32[2];
		uint64_t	val64;
	} val;
} type_mask_t;


#define AnyMask  	0xffffffffffffffffLL

#ifdef WORDS_BIGENDIAN

#define OffsetRecordFlags 	0
#define MaskRecordFlags  	0xffffffff00000000LL
#define ShiftRecordFlags 	32

#define OffsetDir 		2
#define MaskDir	   		0x00000000ff000000LL
#define ShiftDir  		24

#define OffsetFlags 	2
#define MaskFlags   	0x0000000000ff0000LL
#define ShiftFlags  	16

#define OffsetProto 	2
#define MaskProto   	0x000000000000ff00LL
#define ShiftProto  	8

#define OffsetTos		2
#define MaskTos	   		0x00000000000000ffLL
#define ShiftTos  		0

#define OffsetInOut 	3
#define MaskInput		0xffff000000000000LL
#define MaskOutput		0x0000ffff00000000LL
#define ShiftInput 		48
#define ShiftOutput 	32	

#define OffsetPort 		3
#define MaskDstPort 	0x000000000000ffffLL
#define MaskSrcPort 	0x00000000ffff0000LL
#define ShiftDstPort 	0
#define ShiftSrcPort 	16

#define OffsetAS 		4
#define MaskDstAS 		0x0000ffff00000000LL
#define MaskSrcAS 		0xffff000000000000LL
#define ShiftSrcAS 		48
#define ShiftDstAS 		32

#define OffsetSrcIPv4 	6
#define MaskSrcIPv4  	0x00000000ffffffffLL
#define ShiftSrcIPv4 	0

#define OffsetDstIPv4 	8
#define MaskDstIPv4  	0x00000000ffffffffLL
#define ShiftDstIPv4  	0	

#define OffsetSrcIPv6a 	5
#define OffsetSrcIPv6b 	6
#define OffsetDstIPv6a 	7
#define OffsetDstIPv6b 	8
#define MaskIPv6  		0xffffffffffffffffLL
#define ShiftIPv6 		0

#define OffsetPackets 	9
#define MaskPackets  	0xffffffffffffffffLL
#define ShiftPackets 	0

#define OffsetBytes 	10
#define MaskBytes  		0xffffffffffffffffLL
#define ShiftBytes 		0

#else

#define OffsetRecordFlags 	0
#define MaskRecordFlags  	0x00000000ffffffffLL
#define ShiftRecordFlags 	0

#define OffsetDir 		2
#define MaskDir	   		0x000000ff00000000LL
#define ShiftDir  		32

#define OffsetFlags 	2
#define MaskFlags   	0x0000ff0000000000LL
#define ShiftFlags  	40

#define OffsetProto 	2
#define MaskProto   	0x00ff000000000000LL
#define ShiftProto  	48

#define OffsetTos		2
#define MaskTos	   		0xff00000000000000LL
#define ShiftTos  		56

#define OffsetInOut 	3
#define MaskInput		0x000000000000ffffLL
#define MaskOutput		0x00000000ffff0000LL
#define ShiftInput 		0
#define ShiftOutput 	16

#define OffsetPort 		3
#define MaskDstPort 	0xffff000000000000LL
#define MaskSrcPort 	0x0000ffff00000000LL
#define ShiftDstPort 	48
#define ShiftSrcPort 	32

#define OffsetAS 		4
#define MaskDstAS 		0x00000000ffff0000LL
#define MaskSrcAS 		0x000000000000ffffLL
#define ShiftSrcAS 		0
#define ShiftDstAS 		16

#define OffsetSrcIPv4 	6
#define MaskSrcIPv4  	0xffffffff00000000LL
#define ShiftSrcIPv4 	32

#define OffsetDstIPv4 	8
#define MaskDstIPv4  	0xffffffff00000000LL
#define ShiftDstIPv4  	32

#define OffsetSrcIPv6a 	5
#define OffsetSrcIPv6b 	6
#define OffsetDstIPv6a 	7
#define OffsetDstIPv6b 	8
#define MaskIPv6  		0xffffffffffffffffLL
#define ShiftIPv6 		0

#define OffsetPackets 	9
#define MaskPackets  	0xffffffffffffffffLL
#define ShiftPackets 	0

#define OffsetBytes 	10
#define MaskBytes  		0xffffffffffffffffLL
#define ShiftBytes 		0

#endif


/*
 * flags:
 * bit  0:	0: IPv4				1: IPv6
 * bit  1:	0: 32bit dPkts		1: 64bit dPkts
 * bit  2:	0: 32bit dOctets	1: 64bit dOctets 
 * more to come ...
 *
 * bit 31:	reserved for future use. must be 0.
 */

#define FLAG_IPV6_ADDR	1
#define FLAG_PKG_64		2
#define FLAG_BYTES_64	4

/*
 * offset translation table
 * In netflow v9 values may have a different length, and may or may nor be present.
 * The commmon information ( see data_block_record_t ) is expected to be present
 * unconditionally, and has a fixed size. IP addrs as well as counters for packets and
 * bytes are expexted to exist as well, but may be variable in size. Further information
 * may or may not be present, according the flags. See flags
 * To cope with this situation, the offset translation table gives the offset into an
 * uint32_t array at which offset the requested value start.
 *
 * index:
 *	 0:	dstip
 *				for IPv4 netflow v5/v7	10
 *	 1: dPkts
 * 				for IPv4 netflow v5/v7	11
 *	 2: dOctets
 * 				for IPv4 netflow v5/v7	12
 */
void ExpandRecord(common_record_t *input_record,master_record_t *output_record );
#endif
