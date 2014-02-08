## Process this file with automake to produce Makefile.in

AM_LDFLAGS = @flagvar@ 

bin_PROGRAMS = libpacct_flaim.so
libpacct_flaim_so_SOURCES = \
			   flaim.h libpacct_parser.cpp libpacct_parser.h \
			   libpacct_structs.h map_record.cpp map_record.h \
			   record.h value.cpp value.h value_types.h \
			   libpacct_structs.cpp
			    
data_DATA = pacct.modschema.xml samplepacct.apolicy.xml
EXTRA_DIST = pacct.modschema.xml samplepacct.apolicy.xml
