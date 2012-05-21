SUBDIRS = src

# autogen.sh and cleanrepo.sh are script for maintainance use. Not for distribution.
# dist_noinst_SCRIPTS = autogen.sh

## pkg-config ##

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = cld.pc

# For testing the library.
check_PROGRAMS = basic_test

basic_test_SOURCES = tests/basic_test.cc
basic_test_CXXFLAGS = -Wall -fPIC -Isrc/ -O2 -DCLD_WINDOWS
basic_test_LDFLAGS = -lcld -Lsrc/ -lstdc++

TESTS = basic_test

