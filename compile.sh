#!/bin/sh
ABSPATH=$(readlink -f $0)
ABSDIR=$(dirname $ABSPATH)

cd $ABSDIR

make clean
/usr/lib/qt5/bin/qmake FileShareServer.pro
ERRCODE=$?
if [ $ERRCODE -ne 0 ] ;then
    echo "ERROR qmake $ERRCODE"
    exit $ERRCODE
fi

make -j4

if [ $? -ne 0 ] ;then
    exit $?
    echo "make ERROR"
fi
