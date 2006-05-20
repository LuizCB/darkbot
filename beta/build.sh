#! /bin/sh

test ! -r build/configure && ./bootstrap.sh

cd build
./configure -C --prefix=$HOME/darkbot "$@" && make && make install
cd ..
