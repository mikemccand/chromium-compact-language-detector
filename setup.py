from distutils.core import setup, Extension

import platform

defines = [('CLD_WINDOWS', None)]

if platform.system() == 'Windows':
  defines.append(('WIN32', None))

module = Extension('cld',
                   language='c++',
                   libraries=['cld'],
                   include_dirs=['.'],
                   library_dirs=['.'],
                   define_macros = defines,
                   sources=['pycldmodule.cc'])

setup(name='cld',
      version='0.031415',
      description='Python bindings around Google Chromium\'s embedded compact language detection library',
      ext_modules = [module])
