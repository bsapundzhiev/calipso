Build instructuions:
--------------------

1. Edit Makefile un/coment or insert in new file - Config
	#USE_SSL=1 for SSL support
	#USE_POLL=1
	#USE_EPOLL=1

2. Build
	# ./uprev.sh
	# make 
	# make install


Cross compile for armv6 (raspberry pi)
---------------------------------------

export PATH=$PATH:$HOME/cross/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin
export CC=arm-linux-gnueabihf-gcc

or 
              
export TOOL_PREFIX=$HOME/cross/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf
export CXX=$TOOL_PREFIX-g++
export AR=$TOOL_PREFIX-ar
export RANLIB=$TOOL_PREFIX-ranlib
export CC=$TOOL_PREFIX-gcc
export LD=$TOOL_PREFIX-ld
export CCFLAGS="-march=armv6 -mno-thumb-interwork"

1. Calipso
----------
make 

2. PHP
------

$ ./configure --prefix=$HOME/php --with-calipso=$HOME/Desktop/calipso-svn/src \ 
--host=arm-linux-gnueabihf --disable-libxml --disable-dom --disable-simplexml --disable-xmlreader \ 
--disable-xmlwriter --disable-xml --without-pear --without-sqlite3 --without-cdb --without-iconv \
--without-pdo-sqlite

