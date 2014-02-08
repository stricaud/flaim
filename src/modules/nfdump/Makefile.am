## Process this file with automake to produce Makefile.in

AM_LDFLAGS = @flagvar@
# LDFLAGS = -flat_namespace -bundle -undefined suppress 

bin_PROGRAMS = libnfdump_flaim.so
libnfdump_flaim_so_SOURCES = flaim.h map_record.cpp map_record.h \
			     nfd_structs.h nfdump.cpp nfdump.h record.h \
			     value.cpp value.h value_types.h nffile.h
data_DATA = nfdump.modschema.xml samplenfdump.apolicy.xml
EXTRA_DIST = nfdump.modschema.xml samplenfdump.apolicy.xml
