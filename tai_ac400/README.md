AC400 TAI implementation
============================

This folder contains the TAI adapter implementation for the Acacia AC400
transponder. 

After cloning this project, it can be built with:
  1. git submodule init
  2. git submodule update
  3. ./autogen.sh
  4. ./configure --enable-debug --prefix=/usr
  5.  make

The output result is a TAI library, called libtai
User applications can then link with this library, in order to use the TAI AC400 implementation.

More information about TAI is available here: https://github.com/Telecominfraproject/oopt-tai

