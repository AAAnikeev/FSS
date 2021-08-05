#!/bin/sh

ABSPATH=$(readlink -f $0)
ABSDIR=$(dirname $ABSPATH)

cd $ABSDIR

PATHDIR=./install/astra/opt/rbt/FSS
rm -r $PATHDIR/bin
mkdir $PATHDIR/bin

SYSTEMDPATH=./install/astra/etc/systemd/system
BINPATH=./install/astra/opt/rbt/FSS/bin
DBPATH=./install/astra/opt/rbt/FSS/database

mkdir -p $SYSTEMDPATH
mkdir -p $BINPATH
mkdir -p $DBPATH

cp  ./etc/fsserver.service ./install/astra/etc/systemd/system
cp ./etc/file_share.sql ./install/astra/opt/rbt/FSS/database/


cp -r ./bin/fsserver $PATHDIR/bin
rm -r artifacts
mkdir -p artifacts

fakeroot chmod 775 ./install/astra/DEBIAN/postinst

fakeroot dpkg-deb --build ./install/astra artifacts
