from distutils.core import setup, Extension

import platform

defines = [('CLD_WINDOWS', None)]

if platform.system() == 'Windows':
  defines.append(('WIN32', None))

module = Extension('cld',
                   language='c++',
                   include_dirs=['.'],
                   library_dirs=['.'],
                   define_macros = defines,
                   sources=['src/pycldmodule.cc'])

setup(name='chromium_compact_language_detector',
      version='0.031415',
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
        ]
      )
