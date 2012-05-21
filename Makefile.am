AUTOMAKE_OPTIONS = subdir-objects

ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

lib_LTLIBRARIES = libcld.la

libcld_la_SOURCES = encodings/compact_lang_det/cldutil.cc \
        encodings/compact_lang_det/cldutil_dbg_empty.cc \
        encodings/compact_lang_det/compact_lang_det.cc \
        encodings/compact_lang_det/compact_lang_det_impl.cc \
        encodings/compact_lang_det/ext_lang_enc.cc \
        encodings/compact_lang_det/getonescriptspan.cc \
        encodings/compact_lang_det/letterscript_enum.cc \
        encodings/compact_lang_det/tote.cc \
        encodings/compact_lang_det/generated/cld_generated_score_quadchrome_0406.cc \
        encodings/compact_lang_det/generated/compact_lang_det_generated_cjkbis_0.cc \
        encodings/compact_lang_det/generated/compact_lang_det_generated_ctjkvz.cc \
        encodings/compact_lang_det/generated/compact_lang_det_generated_deltaoctachrome.cc \
        encodings/compact_lang_det/generated/compact_lang_det_generated_quadschrome.cc \
        encodings/compact_lang_det/win/cld_htmlutils_windows.cc \
        encodings/compact_lang_det/win/cld_unilib_windows.cc \
        encodings/compact_lang_det/win/cld_utf8statetable.cc \
        encodings/compact_lang_det/win/cld_utf8utils_windows.cc \
        encodings/internal/encodings.cc \
        languages/internal/languages.cc

libcld_la_CXXFLAGS = -Wall -fPIC -I. -O2 -DCLD_WINDOWS
libcld_la_LDFLAGS = -version-info $(CLD_LT_VERSION)
# libcld_la_LDADD =
#libcld_includedir = $(includedir)/cld-$(CLD_API_VERSION)/

cld_includedir = $(includedir)/cld/
nobase_cld_include_HEADERS = encodings/compact_lang_det/win/cld_utf8statetable.h \
    encodings/compact_lang_det/win/cld_basictypes.h \
    base/basictypes.h \
    base/port.h \
    base/build_config.h \
    languages/proto/languages.pb.h \
    languages/public/languages.h \
    encodings/proto/encodings.pb.h \
    encodings/public/encodings.h \
    encodings/compact_lang_det/letterscript_enum.h

# For simplicity, these are the 2 only files that people should be including in their source, this way:
# #include "cld/compact_lang_det.h"
# #include "cld/lang_enc.h"
public_includedir = $(includedir)/cld/
public_include_HEADERS = encodings/compact_lang_det/compact_lang_det.h \
    encodings/lang_enc.h \
    encodings/compact_lang_det/ext_lang_enc.h

## Distribution files which won't be installed ##

dist_noinst_HEADERS = encodings/compact_lang_det/cldutil.h \
    encodings/compact_lang_det/tote.h \
    encodings/compact_lang_det/win/cld_commandlineflags.h \
    encodings/compact_lang_det/cldutil_dbg.h \
    encodings/compact_lang_det/generated/compact_lang_det_generated_meanscore.h \
    encodings/compact_lang_det/utf8propletterscriptnum.h \
    encodings/compact_lang_det/win/cld_logging.h \
    encodings/compact_lang_det/win/cld_unilib.h \
    encodings/compact_lang_det/win/cld_utf.h \
    encodings/compact_lang_det/compact_lang_det_impl.h \
    encodings/compact_lang_det/getonescriptspan.h \
    encodings/compact_lang_det/utf8propjustletter.h \
    encodings/compact_lang_det/utf8scannotjustletterspecial.h \
    encodings/compact_lang_det/win/cld_google.h \
    encodings/compact_lang_det/win/cld_macros.h \
    base/macros.h \
    base/type_traits.h \
    base/template_util.h \
    encodings/compact_lang_det/win/cld_strtoint.h \
    encodings/compact_lang_det/win/cld_htmlutils.h \
    encodings/compact_lang_det/win/cld_utf8utils.h \
    base/string_util.h \
    encodings/compact_lang_det/unittest_data.h

# autogen.sh and cleanrepo.sh are script for maintainance use. Not for distribution.
# dist_noinst_SCRIPTS = autogen.sh

## pkg-config ##

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = cld.pc

# For testing the library.
check_PROGRAMS = basic_test

basic_test_SOURCES = tests/basic_test.cc
basic_test_CXXFLAGS = -Wall -fPIC -I. -O2 -DCLD_WINDOWS
basic_test_LDFLAGS = -lcld -lstdc++

TESTS = basic_test

## Targets for python wrapper ##

noinst_PYTHON = setup.py \
    pycldmodule.cc \
    cld_encodings.h

# ${PYTHON_EXEC_PREFIX} defaults to ${exec_prefix}

if HAVE_PYTHON
all-python:
	${PYTHON} setup.py build
	
install-python:
	${PYTHON} setup.py install --prefix="$(PYTHON_EXEC_PREFIX)"

else !HAVE_PYTHON
all-python:
	@echo "Python is not installed."

install-python: all-python

endif
