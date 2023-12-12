#! /bin/sh
wget http://www.ijg.org/files/jpegsrc.v9e.tar.gz
tar -xvf jpegsrc.v9e.tar.gz && rm *.tar.gz
cd jpeg-9e

./configure CC='cc' --disable-static --prefix=`pwd`/out
make && make install
