from distutils.core import setup, Extension

module = Extension('cld',
                   language='c++',
                   libraries=['cld'],
                   include_dirs=['.'],
                   library_dirs=['.'],
                   sources=['pycldmodule.cc'])

setup(name='cld',
      version='0.031415',
      description='Python bindings around Google Chromium\'s embedded compact language detection library',
      ext_modules = [module])
