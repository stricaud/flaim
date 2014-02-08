#!/bin/sh
THEARGS=$*
FLAIM_ROOT=FLAIMVAR
export FLAIM_ROOT

if [ "uname -s | grep NetBSD" ] 
then
 export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/pkg/lib
fi

FLAIMVAR/flaim-core $THEARGS
