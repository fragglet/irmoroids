#!/bin/sh

aclocal
automake -a 
autoconf
automake -a

./configure $@

