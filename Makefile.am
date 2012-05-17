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
    encodings/public/encodings.h
# For simplicity, these are the 2 only files that people should be including in their source, this way:
# #include "cld/compact_lang_det.h"
# #include "cld/lang_enc.h"
public_includedir = $(includedir)/cld/
public_include_HEADERS = encodings/compact_lang_det/compact_lang_det.h \
    encodings/lang_enc.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = cld.pc

dist_noinst_SCRIPTS = autogen.sh

# For testing the library.
check_PROGRAMS = basic_test

basic_test_SOURCES = tests/basic_test.cc
basic_test_CXXFLAGS = -Wall -fPIC -I. -O2 -DCLD_WINDOWS
basic_test_LDFLAGS = -lcld -lstdc++

TESTS = basic_test

