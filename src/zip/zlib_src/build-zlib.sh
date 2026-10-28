#!/bin/bash

ZLIB=zlib-1.3.1
DISTDIR=../../lnx

# CFLAGS=-m32


# ----------------------
# 1. unpack sources

if [ ! -d ${ZLIB} ]; then
    if [ ! -f ${ZLIB}.zip ]; then
        echo "${ZLIB}.zip does not found"
        exit 1
    fi
    unzip ${ZLIB}.zip || exit 1
fi

cd $ZLIB || exit 1
mkdir -p $DISTDIR
chmod +x configure


# ------------------------
# 2. make static library

make distclean
./configure --static || exit 1
make || exit 1

cp -fvP libz.a $DISTDIR/libz-64.a


# ------------------------
# 3. make shared libraries

make distclean
./configure --shared || exit 1
make || exit 1

rm -f $DISTDIR/libz-64.so.1*
cp -fvP libz.so.1.3.1 $DISTDIR/libz-64.so.1.3.1
(cd $DISTDIR ; ln -sf libz-64.so.1.3.1 libz-64.so.1)
(cd $DISTDIR ; ln -sf libz-64.so.1.3.1 libz-64.so)

# ------------------------
# 6. grab standard zip wrappers code

cp -v contrib/minizip/ioapi.c ../../
cp -v contrib/minizip/ioapi.h ../../
cp -v contrib/minizip/iowin32.c ../../
cp -v contrib/minizip/iowin32.h ../../
cp -v contrib/minizip/unzip.c ../../
cp -v contrib/minizip/unzip.h ../../
cp -v zconf.h ../../
cp -v zlib.h ../../
