#! /bin/bash
#------------------------------------------------------------------------------
# This bash script regenerates the HTML doxygen version of the
# Lomse API documentation.
#
# Documentation is generated in a folder, named api-docs, at the same
# level than the project root folder. This folder will be created if
# does not exist.
#
# usage: ./build-api-docs.sh
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# main line starts here

E_SUCCESS=0         # success
E_ERROR=1
E_NOARGS=65         # no arguments
E_BADPATH=66        # not running from lomse/trunk/scripts
E_BADARGS=67        # bad arguments

enhanced="\e[7m"
reset="\e[0m"


#get current directory and check we are running from <root>/scripts
#For this I just check that "src" folder exists
LOMSE=$(dirname "${PWD}")
if [[ ! -e "${LOMSE}/src" ]]; then
    echo "Error: not running from <root>/scripts"
    exit $E_BADPATH
fi

#paths
ROOT="${LOMSE}"
PROJECTS=$(dirname "${ROOT}")
DOXY="${ROOT}/docs/api"
HTML="${PROJECTS}/api-docs"
echo "ROOT = ${ROOT}"
echo "PROJECTS = ${PROJECTS}"
echo "DOXY = ${DOXY}"
echo "HTML = ${HTML}"

# Check that doxygen has the correct version as different versions of it are
# unfortunately not always (in fact, practically never) compatible.
doxygen_version=`doxygen --version`
doxygen_version_required=1.8.6
if [[ $doxygen_version != $doxygen_version_required ]]; then
    echo -e "${enhanced}Doxygen version $doxygen_version is not supported.${reset}"
    echo -e "${enhanced}Please use Doxygen $doxygen_version_required or update the doxyfile configuration file.${reset}"
    exit $E_ERROR
fi


# Now run doxygen and generate the documentation
cd ${DOXY}
doxygen doxyfile


# Doxygen has the annoying habit of putting the full path of the affected
# files in the log file; remove it to make the log more readable
cd ${HTML}
srcpath="${ROOT}/include/"
cat doxygen.log | sed -r "s|$srcpath||" > temp
cat temp | sed -r "s|$DOXY||" > doxygen.log
rm temp

echo "Done. Generated at file://${HTML}/html/index.html"
echo "Generation log at ${HTML}/doxygen.log"
gedit "${HTML}/doxygen.log" &
exit $E_SUCCESS


