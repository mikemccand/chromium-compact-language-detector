## Targets for python wrapper ##

noinst_PYTHON = setup.py \
    src/pycldmodule.cc \
    src/cld_encodings.h

# ${PYTHON_EXEC_PREFIX} defaults to ${exec_prefix}
if HAVE_PYTHON
all-python:
	CFLAGS="`${PKG_CONFIG} --cflags cld`" ${PYTHON} setup.py build
	
install-python:
	CFLAGS="`${PKG_CONFIG} --cflags cld`" ${PYTHON} setup.py install --prefix="$(PYTHON_EXEC_PREFIX)"

else !HAVE_PYTHON
all-python:
	@echo "Python is not installed."

install-python: all-python

endif

all-local: all-python

install-exec-local: install-python

# TODO: make an uninstall target.
