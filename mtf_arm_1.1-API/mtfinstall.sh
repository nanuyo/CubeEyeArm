#!/bin/sh

# meerecompany driver install.
# product VID, PID setting
# MTF_VIEWER 1.0ver, MTF_ARM 1.0ver
# root authority command 'mtfinstall.sh'

CURDIR=`pwd`
echo "Current directory is $CURDIR. MTF driver will be installed..."
A=`whoami`

if [ $A != 'root' ]; then
   echo "You have to be root to run this script"
   exit 1;
fi

echo " MTF driver install finish."
