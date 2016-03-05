#! /bin/bash
#------------------------------------------------------------------------------
# Local installation of Lomse library and building lclt
# This script MUST BE RUN from lomse/trunk/scripts/ folder
#
# usage: sudo ./install-lomse.sh
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
# main line starts here

E_SUCCESS=0         # success
E_NOARGS=65         # no arguments
E_BADPATH=66        # not running from lomse/trunk/scripts

#get current directory and check we are running from lomse/trunk/scripts
scripts_path="${PWD}"
trunk_path=$(dirname "${PWD}")
if [ $(basename "$trunk_path") != "trunk" ]; then
    echo "Error: not running from lomse/trunk/scripts"
    exit $E_BADPATH
fi
lomse_build_path="${root_path}/build-area"
build_root=$(dirname "${trunk_path}")
lclt_build_path="${build_root}/lclt/build"

echo "build root = ${build_root}"
echo "lomse build path = ${lomse_build_path}"
echo "lclt build path = ${lclt_build_path}"
exit 0

echo "Local installation of Lomse library and building lclt program"

#prepare package:
cd "${lomse_build_path}"
make package || exit 1

#find new package name
library=`ls |grep 'liblomse_[0-9]*.[0-9]*.[0-9]*-[0-9]*_[a-zA-Z0-9]*.deb'`

#uninstall lclt, if installed
installed=`dpkg -l | grep 'lclt'`
if [ -n "$installed" ]; then
    echo "Removing lclt package"
    dpkg -r lclt
fi

#uninstall current library, if installed
installed=`dpkg -l | grep 'liblomse'`
if [ -n "$installed" ]; then
    echo "Removing old liblomse package"
    sudo dpkg -r liblomse
fi

#install new version of lomse
dpkg -i "${library}"

#build and install lclt
cd "${lclt_build_path}"
rm * -r
cmake -G "Unix Makefiles" ../../../projects/lclt/trunk
make -j2
make install

#reconfigure dynamic linker run time bindings
ldconfig

exit $E_SUCCESS

