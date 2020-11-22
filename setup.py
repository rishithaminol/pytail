#!/usr/bin/env python

from distutils.core import setup, Extension

setup(name = 'pytail', version = '0.1',  \
    ext_modules = [Extension('pytail', ['pytail.c'])], \
    author = "Rishitha Minol", \
    author_email = "rishithaminol@gmail.com", \
    description  = "A tail implementation for Python language")
