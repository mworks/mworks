#!/bin/sh

if [ -z $1 ]
    then
    echo "usage: "
    echo "  buildAndUpload.sh <VERSION_NUMBER>"
    exit -1;
fi

MONKEYWORKSINSTALL_DIR=`dirname $0`
cd $MONKEYWORKSINSTALL_DIR

./buildAllPackages.sh $1
./uploadPackages.sh $1
