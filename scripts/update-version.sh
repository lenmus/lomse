#! /bin/bash
#------------------------------------------------------------------------------
# Update lomse version information in several places:
#   include/lomse_version.h
#   debian/changelog
#
# This script MUST BE RUN from <root>/scripts/ folder
#
# usage: ./update-version.sh <major.minor.patch>
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
# main line starts here

E_SUCCESS=0         # success
E_NOARGS=65         # no arguments
E_BADPATH=66        # not running from <root>/scripts


# check that new version is present
if [ -z "$1" ]
then
    echo "Usage: `basename $0` major.minor.patch"
    exit $E_NOARGS
fi

#get current directory and check we are running from <root>/scripts.
#For this I just check that "src" folder exists
scripts_path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
lomse_path=$(dirname "${scripts_path}")
if [[ ! -e "${lomse_path}/src" ]]; then
    echo "Error: cannot find src folder" 1>&2
    exit $E_BADPATH
fi

source ${scripts_path}/helper.sh

description="$1"
parseDescription "$description"

## get lomse version from repo tags
#echo "Getting lomse version"
#cd "${lomse_path}"

#description="$(git describe --tags --long)"
#parseDescription "$description"
#echo "-- git description = ${description}"
#echo "-- package = ${package}"
#echo "-- major=${major}, minor=${minor}, patch=${patch}, sha1=${sha1}"

echo "-- major=${major}, minor=${minor}, patch=${patch}"

# update default version for building from cmake
file="${lomse_path}/build-version.cmake"
if [ -f $file ]; then
    echo "Updating version in file ${file}"
    sed -i -e 's/\(set( LOMSE_VERSION_MAJOR \)\([01-9]*\)\(.*\)/\1'$major'\3/' \
	-e 's/\(set( LOMSE_VERSION_MINOR \)\([01-9]*\)\(.*\)/\1'$minor'\3/' \
	-e 's/\(set( LOMSE_VERSION_PATCH \)\([01-9]*\)\(.*\)/\1'$patch'\3/' \
	$file
    echo "-- Done"
else
    echo "ERROR: File ${file} not found. Aborted."
    echo ""    
    exit $E_BADPATH
fi

#Update version and date in debian changelog
file="${lomse_path}/debian/changelog"
if [ -f $file ]; then
    echo "Updating version and date in ${file}"
    cd "${lomse_path}/debian"
    FILE="lomse (${package}) stable; urgency=low"
    FILE+=$'\n'
    FILE+=$'\n'
    FILE+="  * Latest release"
    FILE+=$'\n'
    FILE+=$'\n'
    today=`date -R`
    FILE+=" -- Cecilio Salmeron <s.cecilio@gmail.com>  ${today}"
    FILE+=$'\n'
    echo "$FILE" > ${file}
    echo "-- Done"
else
    echo "ERROR: File ${file} not found. Aborted."
    echo ""    
    exit $E_BADPATH
fi

exit $E_SUCCESS

