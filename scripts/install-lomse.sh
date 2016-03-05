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
E_ERROR=99          # any other error

enhanced="\e[7m"
reset="\e[0m"

#get current directory and check we are running from <root>/scripts
#For this I just check that "src" folder exists
scripts_path="${PWD}"
root_path=$(dirname "${PWD}")
if [[ ! -e "${root_path}/src" ]]; then
    echo "Error: not running from <root>/scripts"
    exit $E_BADPATH
fi

build_root="${root_path}/build-area"

#prepare package:
echo -e "${enhanced}Building lomse package${reset}"
cd "${build_root}"
make package || exit $E_ERROR
echo "-- Done"

#find new package name
library=`ls | grep 'liblomse_[0-9]*.[0-9]*.[0-9]*_[a-zA-Z0-9]*.deb'`
if [ -z "$library" ]; then
    echo -e "${enhanced}New package not found!. Aborted.${reset}"
    exit $E_ERROR
fi

#uninstall current library, if installed
installed=`dpkg -l | grep 'liblomse'`
if [ -n "$installed" ]; then
    echo -e "${enhanced}Removing old package${reset}"
    sudo dpkg -r liblomse
    echo "-- Done"
fi

#install new version of lomse
echo -e "${enhanced}Installing new lomse package ${library}${reset}"
sudo dpkg -i "${library}"
echo "-- Done"

#reconfigure dynamic linker run time bindings
echo -e "${enhanced}reconfiguring dynamic linker${reset}"
sudo ldconfig
echo "-- Done"

exit $E_SUCCESS

