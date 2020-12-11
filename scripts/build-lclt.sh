#! /bin/bash
#------------------------------------------------------------------------------
# Building the lclt program for Lomse regression tests
# IMPORTANT: lclt root folder must be in the same folder that Lomse root 
# folder, e.g.:
#
#   my-projects
#       │
#       ├── lomse
#       │     ├── src
#       │     ├── scripts
#       │     ┆
#       │
#       ├── lclt
#       │     ├── src
#       │     ┆
#       ┆
#
# This script MUST BE RUN from lomse/scripts/ folder
#
# usage: ./build-lclt.sh
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
# main line starts here

E_SUCCESS=0         # success
E_NOARGS=65         # no arguments
E_BADPATH=66        # not running from lomse/scripts
E_FAILURE=99        # other failures

enhanced="\e[7m"
reset="\e[0m"

#get current directory and check we are running from <root>/scripts
#For this I just check that "src" folder exists
scripts_path="${PWD}"
root_path=$(dirname "${PWD}")
if [[ ! -e "${root_path}/src" ]]; then
    echo "Error: not running from lomse/scripts"
    exit $E_BADPATH
fi

build_path="${root_path}/zz_build-area"		#AWARE: It is lomse/zz_build-area

#uninstall current version of lclt, if installed
installed=`dpkg -l | grep 'lclt_[0-9]*.[0-9]*.[0-9]*'`
oldapp=$(echo $installed | egrep -o 'lclt_[0-9]*.[0-9]*.[0-9]*' | head -n1)

if [ -n "$oldapp" ]; then
    echo -e "${enhanced}Removing old lclt package ${oldapp}${reset}"
    sudo dpkg -r ${oldapp}
    echo "-- Done"
fi


#build and install lclt
echo -e "${enhanced}Building and installing new lclt package ${library}${reset}"
cd "${build_path}" || exit $E_BADPATH
rm * -rf
cmake -G "Unix Makefiles" ../../lclt
make -j2
sudo make install
echo "-- Done"

exit $E_SUCCESS

