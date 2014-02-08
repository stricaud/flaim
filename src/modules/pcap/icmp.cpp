/******************************************************************************
 *
 * icmp.cpp
 *
 * Libpcap module functions to handle ICMP data
 *
 * last modified by Greg Colombo
 * Wed Jul 19 14:50:05 GMT 2006
 *
 *****************************************************************************/

#include "libpcap_parser.h"
#include "value.h"
#include "value_types.h"
#include <netinet/in.h>

// int getICMPHeader(Record * r)
// Pack an ICMP header into a record
//
// Parameters: r, the record to pack
// Returns: the number of bytes read by this operation
// Modifies: packs data into r and moves the get pointer
int LibpcapParser::getICMPHeader(Record * r) throw (exception&, ifstream::failure&)
{
    icmp_hdr_t ihdr;
    int bytesRead = 0;
    
    infile.read((char*)(&ihdr), sizeof(icmp_hdr_t));
    bytesRead += sizeof(icmp_hdr_t);

    // the first four bytes are always a type, code, and checksum
    uint8_t *type = new uint8_t(ihdr.type);
    uint8_t *systype = new uint8_t(ihdr.type);
    uint8_t *tmptype = new uint8_t(ihdr.type);
    r->setField("ICMP_TYPE", Value(type, VAL_TYPE_UINT8, 1));

    // fix: this allows the put function to discern the original type of
    // the ICMP packet, in case the type field was anonymized
    r->setField("SYS_ICMP_TYPE", Value(tmptype, VAL_TYPE_UINT8, 1));
    
    uint8_t *code = new uint8_t(ihdr.code);
    r->setField("ICMP_CODE", Value(code, VAL_TYPE_UINT16, 1));
    uint16_t tmp = ntohs(ihdr.checksum);
    uint16_t *checksum = new uint16_t(tmp);
    r->setField("ICMP_CHECKSUM", Value(checksum, VAL_TYPE_UINT16, 1));

    uint8_t *util8 = NULL;
    uint16_t *util16 = NULL;
    uint32_t *util32 = NULL;
    uint16_t util16_stack;
    uint32_t util32_stack;

    // what happens next depends on the type field
    switch(ihdr.type)
    {
	case 0: // echo reply
	case 8: //echo
	    util16_stack = ntohs(ihdr.extra.s.hi);
	    util16 = new uint16_t(util16_stack);
	    r->setField("ICMP_IDENTIFIER", Value(util16, VAL_TYPE_UINT16, 2));
	    util16_stack = ntohs(ihdr.extra.s.lo);
	    util16 = new uint16_t(util16_stack);
	    r->setField("ICMP_SEQUENCE", Value(util16, VAL_TYPE_UINT16, 2));
	    // bytesRead += getICMPEcho(r);
	    break;
	case 3: // destination unreachable
	    bytesRead += getICMPGeneric(r);
	    break;
	case 4: // source quench
	    bytesRead += getICMPGeneric(r);
	    break;
	case 5: // redirect
	    bytesRead += getICMPGeneric(r);
	    util32_stack = ntohl(ihdr.extra.l);
	    util32 = new uint32_t(util32_stack);
	    r->setField("ICMP_GATEWAY", Value(util32, VAL_TYPE_UINT32, 4));
	    break;
	case 11: // time exceeded
	    bytesRead += getICMPGeneric(r);
	    break;
	case 12: // parameter problem
	    bytesRead += getICMPGeneric(r);
	    util8 = new uint8_t(ihdr.extra.c[0]);
	    r->setField("ICMP_POINTER", Value(util8, VAL_TYPE_UINT8, 1));
	    break;
	case 13: // timestamp
	case 14: // timestamp reply
	    util16_stack = ntohs(ihdr.extra.s.hi);
	    util16 = new uint16_t(util16_stack);
	    r->setField("ICMP_IDENTIFIER", Value(util16, VAL_TYPE_UINT16, 2));
	    util16_stack = ntohs(ihdr.extra.s.lo);
	    util16 = new uint16_t(util16_stack);
	    r->setField("ICMP_SEQUENCE", Value(util16, VAL_TYPE_UINT16, 2));
	    bytesRead += getICMPTimestamp(r);
	    break;
	case 15: // information request
	case 16: // information reply
	    util16_stack = ntohs(ihdr.extra.s.hi);
	    util16 = new uint16_t(util16_stack);
	    r->setField("ICMP_IDENTIFIER", Value(util16, VAL_TYPE_UINT16, 2));
	    util16_stack = ntohs(ihdr.extra.s.lo);
	    util16 = new uint16_t(util16_stack);
	    r->setField("ICMP_SEQUENCE", Value(util16, VAL_TYPE_UINT16, 2));
	    break;
	default:
	    //cerr << "*** libpcap_flaim: Unrecognized ICMP code type" << endl;
	    break;
    }

    return bytesRead;
}

// int putICMPHeader(Record *)
// put an ICMP header back to disk
//
// parameters: r, the record to unpack
// returns: 0 on success, 1 on failure
// modifies: deletes data from the record and writes data out to disk
int LibpcapParser::putICMPHeader(Record * r)
{
    icmp_hdr_t ihdr;
    Value v;
    
    r->getField("ICMP_TYPE", v);
    ihdr.type = *(uint8_t*)(v.Ptr());
    delete (uint8_t*)(v.Ptr());

    r->getField("ICMP_CODE", v);
    ihdr.code = *(uint8_t*)(v.Ptr());
    delete (uint8_t*)(v.Ptr());

    r->getField("ICMP_CHECKSUM", v);
    ihdr.checksum = htons(*(uint16_t*)(v.Ptr()));
    delete (uint16_t*)(v.Ptr());

    r->getField("SYS_ICMP_TYPE", v);
    uint8_t tmpType = *(uint8_t*)(v.Ptr());
    delete (uint8_t*)(v.Ptr());

    // fill in the union in the ICMP header which contains any extra
    // information; if this information is not present, the union should
    // be set to 0
    ihdr.extra.l = 0;
    switch(tmpType)
    {
	case 0:
	case 8:
	case 13:
	case 14:
	case 15:
	case 16:
	    r->getField("ICMP_IDENTIFIER", v);
	    ihdr.extra.s.hi = htons(*(uint16_t*)(v.Ptr()));
	    delete (uint16_t*)(v.Ptr());
	    r->getField("ICMP_SEQUENCE", v);
	    ihdr.extra.s.lo = htons(*(uint16_t*)(v.Ptr()));
	    delete (uint16_t*)(v.Ptr());
	    break;
	case 5:
	    r->getField("ICMP_GATEWAY", v);
	    ihdr.extra.l = htonl(*(uint32_t*)(v.Ptr()));
	    delete (uint32_t*)(v.Ptr());
	    break;
	case 12:
	    r->getField("ICMP_POINTER", v);
	    ihdr.extra.c[0] = *(uint8_t*)(v.Ptr());
	    delete (uint8_t*)(v.Ptr());
	    break;
	default: break;
    }

    // write the ICMP header to disk
    outfile.write((char*)(&ihdr), sizeof(icmp_hdr_t));

    // now handle any extra information
    switch(tmpType)
    {
	case 3:
	case 4:
	case 5:
	case 11:
	case 12:
	    putICMPGeneric(r);
	    break;

	case 13:
	case 14:
	    putICMPTimestamp(r);
	    break;
	    
	default: break;
    }

    return 0;
}

int LibpcapParser::getICMPGeneric(Record * r) throw (exception&, ifstream::failure&)
{
    ipv4_hdr_t ihdr;
    char* buf = new char[8];

    // read the internet header
    infile.read((char*)(&ihdr), sizeof(ipv4_hdr_t));
    
    // plus 64 bits of the original datagram
    infile.read(buf, 8);
    // is->read(buf, 8);

    // copy over into dynamic memory
    uint8_t *version = new uint8_t((ihdr.vhl & 0xF0) >> 4);
    uint8_t *headlen = new uint8_t(ihdr.vhl & 0x0F);
    uint8_t *type = new uint8_t(ihdr.type);
    uint16_t tmp = ntohs(ihdr.length);
    uint16_t *length = new uint16_t(tmp);
    tmp = ntohs(ihdr.id);
    uint16_t *id = new uint16_t(tmp);
    tmp = ntohs(ihdr.offset);
    uint16_t *offset = new uint16_t(tmp);
    uint8_t *ttl = new uint8_t(ihdr.ttl);
    uint8_t *protocol = new uint8_t(ihdr.protocol);
    tmp = ntohs(ihdr.checksum);
    uint16_t *checksum = new uint16_t(tmp);
    uint32_t srcipt = ntohl(ihdr.src_ip);
    uint32_t *src_ip = new uint32_t(srcipt);
    uint32_t dstipt = ntohl(ihdr.dst_ip);
    uint32_t *dst_ip = new uint32_t(dstipt);

    // write to the record
    r->setField("ICMP_IPV4_VERSION", Value(version, VAL_TYPE_UINT8, 1));
    r->setField("ICMP_IPV4_HEADLEN", Value(headlen, VAL_TYPE_UINT8, 1));
    r->setField("ICMP_IPV4_TYPE", Value(type, VAL_TYPE_UINT8, 1));
    r->setField("ICMP_IPV4_LENGTH", Value(length, VAL_TYPE_UINT16, 2));
    r->setField("ICMP_IPV4_ID", Value(id, VAL_TYPE_UINT16, 2));
    r->setField("ICMP_IPV4_OFFSET", Value(offset, VAL_TYPE_UINT16, 2));
    r->setField("ICMP_IPV4_TTL", Value(ttl, VAL_TYPE_UINT8, 1));
    r->setField("ICMP_IPV4_PROTOCOL", Value(protocol, VAL_TYPE_UINT8, 1));
    r->setField("ICMP_IPV4_CHECKSUM", Value(checksum, VAL_TYPE_UINT16, 2));
    r->setField("ICMP_IPV4_SRC_IP", Value(src_ip, VAL_TYPE_UINT32, 4));
    r->setField("ICMP_IPV4_DST_IP", Value(dst_ip, VAL_TYPE_UINT32, 4));

    // also provide original data
    r->setField("ICMP_ORIG_DATA", Value(buf, VAL_TYPE_CHAR_PTR, 8));

    return (8 + sizeof(ipv4_hdr_t));

}

int LibpcapParser::getICMPTimestamp(Record * r) throw (exception&, ifstream::failure&)
{
    // define a structure to hold the timestamp data
    struct {
	uint32_t orig;
	uint32_t rec;
	uint32_t trans;
    } timestamps;

    infile.read((char*)(&timestamps), 12);

    uint32_t orig = ntohl(timestamps.orig);
    uint32_t rec = ntohl(timestamps.rec);
    uint32_t trans = ntohl(timestamps.trans);

    uint32_t *orig_p = new uint32_t(orig);
    uint32_t *rec_p = new uint32_t(rec);
    uint32_t *trans_p = new uint32_t(trans);

    r->setField("ICMP_TS_ORIG", Value(orig_p, VAL_TYPE_UINT32, 4));
    r->setField("ICMP_TS_REC", Value(orig_p, VAL_TYPE_UINT32, 4));
    r->setField("ICMP_TS_TRANS", Value(orig_p, VAL_TYPE_UINT32, 4));

    return 12; // sizeof(timestamps)
}

int LibpcapParser::putICMPGeneric(Record * r)
{
    ipv4_hdr_t ihdr;
    Value v;

    r->getField("ICMP_IPV4_VERSION", v);
    uint8_t version = *(uint8_t*)(v.Ptr());
    delete (uint8_t*)(v.Ptr());

    r->getField("ICMP_IPV4_HEADLEN", v);
    uint8_t headlen = *(uint8_t*)(v.Ptr());
    delete (uint8_t*)(v.Ptr());
    ihdr.vhl = ((version << 4) | headlen);

    r->getField("ICMP_IPV4_TYPE", v);
    ihdr.type = *(uint8_t*)(v.Ptr());
    delete (uint8_t*)(v.Ptr());

    r->getField("ICMP_IPV4_LENGTH", v);
    ihdr.length = htons(*(uint16_t*)(v.Ptr()));
    delete (uint16_t*)(v.Ptr());

    r->getField("ICMP_IPV4_ID", v);
    ihdr.id = htons(*(uint16_t*)(v.Ptr()));
    delete (uint16_t*)(v.Ptr());

    r->getField("ICMP_IPV4_OFFSET", v);
    ihdr.offset = htons(*(uint16_t*)(v.Ptr()));
    delete (uint16_t*)(v.Ptr());

    r->getField("ICMP_IPV4_TTL", v);
    ihdr.ttl = *(uint8_t*)(v.Ptr());
    delete (uint8_t*)(v.Ptr());

    r->getField("ICMP_IPV4_PROTOCOL", v);
    ihdr.protocol = *(uint8_t*)(v.Ptr());
    delete (uint8_t*)(v.Ptr());

    r->getField("ICMP_IPV4_CHECKSUM", v);
    ihdr.checksum = htons(*(uint16_t*)(v.Ptr()));
    delete (uint16_t*)(v.Ptr());

    r->getField("ICMP_IPV4_SRC_IP", v);
    ihdr.src_ip = htonl(*(uint32_t*)(v.Ptr()));
    delete (uint32_t*)(v.Ptr());

    r->getField("ICMP_IPV4_DST_IP", v);
    ihdr.dst_ip = htonl(*(uint32_t*)(v.Ptr()));
    delete (uint32_t*)(v.Ptr());

    outfile.write((char*)(&ihdr), sizeof(ipv4_hdr_t));

    r->getField("ICMP_ORIG_DATA", v);
    outfile.write((char*)(v.Ptr()), 8);
    delete[] (char*)(v.Ptr());

    return 0;
}

int LibpcapParser::putICMPTimestamp(Record * r)
{
    Value v;

    struct {
	uint32_t orig;
	uint32_t rec;
	uint32_t trans;
    } timestamps;

    r->getField("ICMP_TS_ORIG", v);
    timestamps.orig = *(uint32_t*)(v.Ptr());
    delete (uint32_t*)(v.Ptr());

    r->getField("ICMP_TS_REC", v);
    timestamps.rec = *(uint32_t*)(v.Ptr());
    delete (uint32_t*)(v.Ptr());

    r->getField("ICMP_TS_TRANS", v);
    timestamps.trans = *(uint32_t*)(v.Ptr());
    delete (uint32_t*)(v.Ptr());

    return 0;
}
