#!/bin/bash

# meerecompany driver install.
# product VID, PID setting
# MTF_VIEWER 1.0ver
# root authority command 'mtfinstall.sh'

CURDIR=`pwd`
echo "Current directory is $CURDIR. MTF driver will be installed..."
A=`whoami`

if [ $A != 'root' ]; then
   echo "You have to be root to run this script"
   exit 1;
fi

# Copy rules file
echo "Copy rules file."
cp cube-eye.rules /etc/udev/rules.d/216-cube-eye.rules

# Remove stale versions of the MTF library.
echo "Remove stale versions of the MTF library."
rm -f /usr/lib/libMTF_API.so* /usr/local/lib/libMTF_API.so*

# Copy the library into the system library folders.
echo "Copy the library into the system library folders."
cp lib/libMTF_API.so.1 /usr/local/lib
ln -s /usr/local/lib/libMTF_API.so.1 /usr/local/lib/libMTF_API.so

cp lib/libMTF_API.so.1 /usr/lib
ln -s /usr/lib/libMTF_API.so.1 /usr/lib/libMTF_API.so

echo " MTF driver install finish."
