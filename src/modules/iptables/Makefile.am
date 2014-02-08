## Process this file with automake to produce Makefile.in

AM_LDFLAGS = @flagvar@

bin_PROGRAMS = libiptable_flaim.so
libiptable_flaim_so_SOURCES = \
			      flaim.h libiptable.h net_structs.h map_record.h \
			      map_record.cpp libiptable.cpp value.cpp value.h \
			      value_types.h record.h streams.cpp
data_DATA = iptable.modschema.xml sampleiptable.apolicy.xml
EXTRA_DIST = iptable.modschema.xml sampleiptable.apolicy.xml
