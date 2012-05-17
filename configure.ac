AC_INIT([libcld], [0.1], [lucene@mikemccandless.com], [compact-language-detector], [http://code.google.com/p/chromium-compact-language-detector/])
AC_PREREQ([2.67])
AM_INIT_AUTOMAKE([1.11 -Wall no-define tar-ustar])

# No config header needed.
#AC_CONFIG_HEADERS([config.h])

# Search for C++ compiler.
AC_PROG_CXX
# Shared library always built. Static library can optionally be enabled at configure time.
LT_INIT([disable-static])
# Version numbering for Libtool.
# See http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html for update logics.
AC_SUBST([CLD_LT_VERSION], [0:0:0])
# Public version number.
AC_SUBST([CLD_API_VERSION], [0.1])

AC_CONFIG_FILES([Makefile cld.pc:cld.pc.in])
AC_OUTPUT
