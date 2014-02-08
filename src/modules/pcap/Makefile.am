## Process this file with automake to produce Makefile.in

AM_LDFLAGS = @flagvar@ 

bin_PROGRAMS = libpcap_flaim.so
libpcap_flaim_so_SOURCES = \
			   flaim.h pcap.cpp libpcap_parser.h \
			   libpcap_structs.h map_record.cpp map_record.h \
			   record.h value.cpp value.h value_types.h \
			   icmp.cpp tcp.cpp udp.cpp ethernet.cpp ipv4.cpp
data_DATA = pcap.modschema.xml samplepcap.apolicy.xml
EXTRA_DIST = pcap.modschema.xml samplepcap.apolicy.xml
