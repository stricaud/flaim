/**********************************************************
 *
 * libpcap_structs.h
 * Header file containing useful structures for the FLAIM
 * libpcap parser
 * 
 * drafted by Greg Colombo
 * last modified Wed May 17 13:37:34 CDT 2006
 *
 *********************************************************/

#ifndef _LIBPCAP_STRUCTS_H
#define _LIBPCAP_STRUCTS_H

#include <stdint.h>

// Definitions of link layer types
#define ENCAP_ETHERNET 1

// Definitions of network layer types
#define NET_IPV4 0x0800

// Definitions of transport layer types
#define TRANS_UNKNOWN 0xFF
#define TRANS_ICMP 1
#define TRANS_TCP 6
#define TRANS_UDP 17

// The following structures are useful for parsing of various types
// of layers. libpcap logs are not restricted to packets of a particular
// layering structure, so this method allows for relatively easy extension
// to use new formats

// The following two headers are defined by the libpcap format and should
// not be removed
typedef struct pcap_hdr_t { // libpcap global header
    uint32_t magic_number;  // magic number used for endianness detection
    uint16_t version_major; // major version of the log format
    uint16_t version_minor; // minor version of the log format
    int32_t thiszone;       // time zone correction, usually ignored
    uint32_t sigfigs;       // timestamp accuracy, usually ignored
    uint32_t snaplen;       // maximum length of dumped packets
    uint32_t network;       // underlying link layer type
} pcap_hdr_t;

typedef struct pcap_pckt_hdr_t { // libpcap packet header
    uint32_t ts_sec;   // Time since 1 Jan 1970 00:00:00 UTC
    uint32_t ts_usec;  // Time offset in microseconds
    uint32_t incl_len; // Length of data dumped to disk
    uint32_t orig_len; // Original length of packet on the wire
} pcap_pckt_hdr_t;

// Subsequent structs are used for particular layer types. You can remove these
// if you also remove all associated references to them

// Ethernet frame format
typedef struct ethernet_hdr_t {
    uint8_t dest[6]; // destination MAC address
    uint8_t src[6];  // source MAC address
    uint16_t type;   // network type encapsulated
} ethernet_hdr_t;

// IPv4 header format, no extra option support built in
typedef struct ipv4_hdr_t {
    uint8_t vhl;        // IP version [3:0] and hdr len. in 32b words [7:4]
    uint8_t type;       // Type of service
    uint16_t length;    // Length of entire packet (header + data)
    uint16_t id;        // Identification field
    uint16_t offset;    // Flags [18:16] and location of fragment [31:19]
    uint8_t ttl;        // Time to live
    uint8_t protocol;   // Underlying transport protocol
    uint16_t checksum;  // Self-explanatory
    uint32_t src_ip;    // Source IP address
    uint32_t dst_ip;    // Destination IP address
} ipv4_hdr_t;

// TCP header format, no extra option support built in
typedef struct tcp_hdr_t {
    uint16_t src_port;  // Sending port number
    uint16_t dst_port;  // Receiving port number
    uint32_t sequence;  // Sequence number used for synchronization
    uint32_t ack_no;    // Sequence number expected next
    uint8_t offset;     // Size of header in 32-bit words [3:0]; [7:4] are
                        // reserved and should be set to 0
    uint8_t flags;      // Control bits [15:10]; [9:8] are reserved and should
                        // be set to 0
    uint16_t window;    // Number of bytes the sender will receive
    uint16_t checksum;  // Checksum for header and data
    uint16_t urgent;    // 16-bit offset from sequence number if flag is set
} tcp_hdr_t;
  
typedef struct udp_hdr_t {
    uint16_t src_port; // sending port number
    uint16_t dst_port; // receiving port number
    uint16_t length; // length of encapsulated data
    uint16_t checksum; // checksum (header and data)
} udp_hdr_t;

#endif // _LIBPCAP_STRUCTS_H
