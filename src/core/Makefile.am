## Process this file with automake to produce Makefile.in

AM_CFLAGS = $(DEFS) @includevar@ 
INCLUDES = @includevar@ 
AM_LDFLAGS = @libraryvar@ 

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datadir)/globalSchemas
	$(INSTALL_DATA) $(srcdir)/globalSchemas/*.rng $(DESTDIR)$(datadir)/globalSchemas
	$(INSTALL_DATA) $(srcdir)/globalSchemas/*.xsl $(DESTDIR)$(datadir)/globalSchemas
#	$(INSTALL_DATA) $(srcdir)/globalSchemas/*.xml $(DESTDIR)$(datadir)/globalSchemas
#	$(INSTALL_DATA) $(srcdir)/flaim.cfg $(DESTDIR)$(datadir)/flaim.cfg

install-exec-local:
	sed -e s%FLAIMVAR/%$(DESTDIR)$(datadir)/% \
	-e s%FLAIMVAR%$(DESTDIR)$(datadir)% \
	$(DESTDIR)$(bindir)/flaim.sh > $(DESTDIR)$(bindir)/flaim
	chmod a+x $(DESTDIR)$(bindir)/flaim

install-data-hook:
	test -z "/usr/bin/flaim" || rm -f /usr/bin/flaim
	ln -s $(DESTDIR)$(bindir)/flaim$(EXEEXT) \
	/usr/bin/flaim$(EXEEXT)
	rm $(DESTDIR)$(bindir)/flaim.sh

uninstall-local:
	rm -rf $(DESTDIR)$(datadir)/globalSchemas/*
	rm -rf $(DESTDIR)$(datadir)/*
	rmdir $(DESTDIR)$(datadir)
	rm /usr/bin/flaim

install-man1:
	$(mkinstalldirs) $(DESTDIR)$(mandir)/man1
	cp flaim.1 $(DESTDIR)$(mandir)/man1/flaim.1

uninstall-man1:
	rm $(DESTDIR)$(mandir)/man1/flaim.1

bin_PROGRAMS = flaim-core
flaim_core_SOURCES = anony_algos.hpp anony_engine.cpp \
		anony_engine.hpp anony_generic.cpp anony_ipaddr.cpp \
		anony_macaddr.cpp anonyparse.c anonyparse.h \
		anony_timestamps.cpp anony_hash.cpp \
		flaim_core.hpp flaim.hpp main.cpp \
		panonymizer.cpp panonymizer.h policy_manager.cpp \
		policy_manager.h policy_manager_lib.c policy_manager_lib.h \
		record.h rijndael.cpp rijndael.h user_policy.cpp \
		user_policy.h value.cpp value.h value_types.h \
		basic_preprocessor.h basic_postprocessor.h preprocessor.h \
		postprocessor.h core_globals.h listed_record.h \
		basic_preprocessor.cpp basic_postprocessor.cpp \
		core_list.h core_list.cpp anony_hostname.cpp \
		time_enum.h time_enum.cpp
bin_SCRIPTS = flaim.sh
man_MANS = flaim.1
#data_DATA = globalSchemas/ globalSchemas/FLAIM-schema.rng \
#	     globalSchemas/MSL-to-Schematron.xsl \
#    globalSchemas/UserPolicy-Sample.xml globalSchemas/ModuleSchema-MSL.xml \
#	     globalSchemas/schematron-basic.xsl globalSchemas/userpolicy.xml \
#	     globalSchemas/ModuleSchemaSTR.xml globalSchemas/skeleton1-5.xsl \
#data_DATA = flaim.cfg

EXTRA_DIST = globalSchemas/flaim_schema.rng \
             globalSchemas/MSL-to-Schematron.xsl \
	     globalSchemas/schematron-basic.xsl \
	     globalSchemas/skeleton1-5.xsl \
	     flaim.1 flaim.sh #flaim.cfg
