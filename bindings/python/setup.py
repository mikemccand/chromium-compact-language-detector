#!/usr/bin/python

from distutils.core import setup, Extension
import distutils.core
import platform
import subprocess
import sys
import os

# Checking existence of pkg-config.
try:
    subprocess.check_call(["pkg-config", '--version'], stdout = subprocess.PIPE, stderr = subprocess.PIPE)
except (subprocess.CalledProcessError, OSError):
    sys.stderr.write('`pkg-config` has not been found but this setup script relies on it.\n')
    sys.exit(os.EX_CONFIG)

# Checking existence and path of `cld` C++ library.
try:
    call = subprocess.Popen(['pkg-config', '--libs', '--cflags', 'cld'], stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    output, error = call.communicate()
    output = output.decode('UTF-8')
    error = error.decode('UTF-8')
    if len(error) != 0:
        sys.stderr.write('`pkg-config --libs --cflags cld` returns an error: \n' + error + '\n')
        raise OSError
except (subprocess.CalledProcessError, OSError):
    sys.stderr.write('The `cld` C++ library is absent from this system. Please install it.\n')
    sys.exit(os.EX_CONFIG)

include_dirs = ['.']
library_dirs = ['.']
for flags in output.split():
    if flags[:2] == '-I':
        include_dirs.append(flags[2:])
    elif flags[:2] == '-L':
        library_dirs.append(flags[2:])

# Setup some define macros for compilation.
defines = [('CLD_WINDOWS', None)]
if platform.system() == 'Windows':
  defines.append(('WIN32', None))

# Test suite
class cldtest(distutils.core.Command):
    # user_options, initialize_options and finalize_options must be overriden.
    user_options = []
    def initialize_options(self):
        pass
    def finalize_options(self):
        pass

    def run(self):
        errno = subprocess.call([sys.executable, 'tests/cld_test.py'])
        raise SystemExit(errno)

#                   include_dirs=['../../src/encodings/compact_lang_det', '../../src'],
#                   library_dirs=['../../src/.libs'],
module = Extension('cld',
                   language='c++',
                   include_dirs = include_dirs,
                   library_dirs = library_dirs,
                   define_macros = defines,
                   libraries = ['cld'],
                   sources=['src/pycldmodule.cc'],
                   )

setup(name='chromium_compact_language_detector',
      version='0.2',
      author='Michael McCandless',
      author_email='mail@mikemccandless.com',
      description='Python bindings around Google Chromium\'s embedded compact language detection library',
      ext_modules = [module],
      license = 'BSD',
      url = 'http://code.google.com/p/chromium-compact-language-detector/',
      classifiers = [
        'License :: OSI Approved :: BSD License',
        'Operating System :: MacOS :: MacOS X',
        'Operating System :: Microsoft :: Windows',
        'Operating System :: POSIX :: Linux',
        'Programming Language :: C++',
        'Programming Language :: Python',
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'Topic :: Text Processing :: Linguistic'
        ],
      cmdclass = {'test': cldtest},
      )
