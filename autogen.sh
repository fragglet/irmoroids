#!/bin/sh

aclocal
libtoolize
automake -a 
autoconf
automake -a

./configure $@

