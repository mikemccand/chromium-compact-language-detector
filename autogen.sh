#!/bin/sh

# NOTE: only for maintainer to recreate configure!
automake --add-missing
autoreconf --install

cd bindings/python
autoreconf --install
