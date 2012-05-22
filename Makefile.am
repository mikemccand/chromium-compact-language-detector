SUBDIRS = src

# autogen.sh and cleanrepo.sh are script for maintainance use. Not for distribution.
# dist_noinst_SCRIPTS = autogen.sh

## pkg-config ##

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = cld.pc

# For testing the library.
check_PROGRAMS = basic_test

basic_test_SOURCES = tests/basic_test.cc
basic_test_CXXFLAGS = -Wall -fPIC -Isrc/ -I../src -O2 -DCLD_WINDOWS
basic_test_LDFLAGS = -lcld -Lsrc/ -lstdc++

TESTS = basic_test

##################
## RELEASE CODE ##
##################

# Packaging a .deb

pkg-deb: dist
	rm -fr temp_deb/
	mkdir -p temp_deb/
	tar -xzf "$(PACKAGE)-$(VERSION).tar.gz" --directory temp_deb/
	mv "$(PACKAGE)-$(VERSION).tar.gz" temp_deb/"$(PACKAGE)_$(VERSION).orig.tar.gz"
	cp -fr debian temp_deb/"$(PACKAGE)-$(VERSION)"
	cd temp_deb/"$(PACKAGE)-$(VERSION)"/; \
	dpkg-buildpackage -us -uc -rfakeroot
	if [ ! -d "release" ]; then \
		mkdir release/; \
	fi
	mv temp_deb/*.deb release/
	rm -rf temp_deb

