AC_INIT([python-cld], [0.1], [lucene@mikemccandless.com], [python-compact-language-detector], [http://code.google.com/p/chromium-compact-language-detector/])
AC_PREREQ([2.67])
AM_INIT_AUTOMAKE([1.11 -Wall no-define tar-ustar])

if test -z "$PKG_CONFIG"; then
    AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
fi
PKG_CHECK_MODULES([CLD], [cld >= 0.1])
## For the Python Wrapper ##
AM_PATH_PYTHON([2.6],, [:])
# Installation won't be aborted if python is not found.
# In this case PYTHON variable will have value ":"
AM_CONDITIONAL([HAVE_PYTHON], [test "$PYTHON" != :])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
