## DO NOT EDIT! GENERATED AUTOMATICALLY!
## Process this file with automake to produce Makefile.in.
# Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2010 Free
# Software Foundation, Inc.
#
# This file is free software, distributed under the terms of the GNU
# General Public License.  As a special exception to the GNU General
# Public License, this file may be distributed as part of a program
# that contains a configuration script generated by Autoconf, under
# the same distribution terms as the rest of that program.
#
# Generated by gnulib-tool.

AUTOMAKE_OPTIONS = 1.5 foreign

SUBDIRS =
TESTS =
TESTS_ENVIRONMENT =
noinst_PROGRAMS =
check_PROGRAMS =
noinst_HEADERS =
noinst_LIBRARIES =
EXTRA_DIST =
BUILT_SOURCES =
SUFFIXES =
MOSTLYCLEANFILES = core *.stackdump
MOSTLYCLEANDIRS =
CLEANFILES =
DISTCLEANFILES =
MAINTAINERCLEANFILES =

AM_CPPFLAGS = \
  -I. -I$(srcdir) \
  -I../.. -I$(srcdir)/../.. \
  -I../../gl -I$(srcdir)/../../gl

LDADD = ../../gl/libgnu.la

## begin gnulib module arpa_inet-tests

TESTS += test-arpa_inet
check_PROGRAMS += test-arpa_inet

EXTRA_DIST += test-arpa_inet.c

## end   gnulib module arpa_inet-tests

## begin gnulib module c-ctype-tests

TESTS += test-c-ctype
check_PROGRAMS += test-c-ctype

EXTRA_DIST += test-c-ctype.c

## end   gnulib module c-ctype-tests

## begin gnulib module getaddrinfo-tests

TESTS += test-getaddrinfo
check_PROGRAMS += test-getaddrinfo
test_getaddrinfo_LDADD = $(LDADD) @LIBINTL@

EXTRA_DIST += test-getaddrinfo.c

## end   gnulib module getaddrinfo-tests

## begin gnulib module getdelim-tests

TESTS += test-getdelim
check_PROGRAMS += test-getdelim
MOSTLYCLEANFILES += test-getdelim.txt
EXTRA_DIST += test-getdelim.c

## end   gnulib module getdelim-tests

## begin gnulib module getline-tests

TESTS += test-getline
check_PROGRAMS += test-getline
MOSTLYCLEANFILES += test-getline.txt
EXTRA_DIST += test-getline.c

## end   gnulib module getline-tests

## begin gnulib module netdb-tests

TESTS += test-netdb
check_PROGRAMS += test-netdb

EXTRA_DIST += test-netdb.c

## end   gnulib module netdb-tests

## begin gnulib module netinet_in-tests

TESTS += test-netinet_in
check_PROGRAMS += test-netinet_in

EXTRA_DIST += test-netinet_in.c

## end   gnulib module netinet_in-tests

## begin gnulib module strerror-tests

TESTS += test-strerror
check_PROGRAMS += test-strerror
EXTRA_DIST += test-strerror.c

## end   gnulib module strerror-tests

# Clean up after Solaris cc.
clean-local:
	rm -rf SunWS_cache

mostlyclean-local: mostlyclean-generic
	@for dir in '' $(MOSTLYCLEANDIRS); do \
	  if test -n "$$dir" && test -d $$dir; then \
	    echo "rmdir $$dir"; rmdir $$dir; \
	  fi; \
	done; \
	:
