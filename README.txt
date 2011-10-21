10/21/2011

This package contains the CLD (Compact Language Detection) library,
extracted from the source code for Google's Chromium library at
http://src.chromium.org/svn/trunk/src/third_party/cld
(http://src.chromium.org/viewvc/chrome/trunk/src/third_party/cld),
specifically revision 105735.

There are two components:

  * A standalone C++ library (libcld.a), which you can use
    from any C++ program.

  * Simple Python bindings around that library.

There is also a Python unit test, ported from the unit test from CLD,
verifying that the library identifies languages correctly and also
showing how to use it.

The LICENSE is the same as Chromium's LICENSE.

This was a very simple, fast rote port: I just extracted the
referenced C++ sources from the cld.gyp sources, translated to
build.sh / setup.py, and got things to compile / pass the test.  I
removed one source file
(encodings/compact_lang_det/win/cld_unicodetext.cc): it wraps CLD,
adding a utility method to convert from UTF16 to UTF8, and normalize
text using ICU.  This means such conversion and normalizing will have
to be done by the apps using this library.  Otherwise I made no
changes to Chromium's sources.

Much more can be done, eg build a dynamic library too, use "make" to
compile everything, expose more of the API to Python,
simplify/refactor the code, etc.

I have only tested this on Fedora 13 with Python 2.6.4; it passes all
tests in test.py there (python -u test.py).

This is a simple example.cc showing how to use the library from C++
code.

Michael McCandless [mail a t mikemccandless dot com]
