#! /bin/bash
#------------------------------------------------------------------------------
# Building and unit testing Lomse library
# This script MUST BE RUN from <root>/scripts/ folder
#
# usage: ./build-lomse.sh [options]*
# examples:
#    ./build-lomse.sh -n -r libpng      # do not build tests and remove support
#                                       # for PNG images
#------------------------------------------------------------------------------


# script settings
#------------------------------------------------------------------------------

# exit this script when a command fails
set -e

# debug mode
#set -x


# colors for messages
#------------------------------------------------------------------------------
RED='\033[0;31m'
GREEN='\033[0;32m'
ORANGE='\033[0;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
LIGHTGRAY='\033[0;37m'
DARKGRAY='\033[1;30m'
LIGHTRED='\033[1;31m'
LIGHTGREEN='\033[1;32m'
YELLOW='\033[1;33m'
LIGHTBLUE='\033[1;34m'
LIGHTPURPLE='\033[1;35m'
LIGHTCYAN='\033[1;36m'
WHITE='\033[1;37m'
WHITEBLACK="\033[7m"        # white background, black chars
NC='\033[0m'                # reset to default. 


# exit codes
#------------------------------------------------------------------------------
E_SUCCESS=0         # success
E_NOARGS=65         # no arguments
E_BADPATH=66        # not running from lomse/scripts
E_BADARGS=67        # bad arguments



#==============================================================================
# Define functions
#==============================================================================

#------------------------------------------------------------------------------
# Display error message
function display_error()
{
    local MESSAGE="$1"
    >&2 echo -e "${RED}$MESSAGE${NC}"
}

#------------------------------------------------------------------------------
# Display error message and terminate script
function abort()
{
    display_error "$1"
    >&2 echo ""
    exit 1
}

#------------------------------------------------------------------------------
# Display warning message
function warning()
{
    local MESSAGE="$1"
    >&2 echo -e "${YELLOW}$MESSAGE${NC}"
}


#------------------------------------------------------------------------------
# Display the help message
function DisplayHelp()
{
    echo "Usage: ./bulid-lomse.sh [option]*"
    echo ""
    echo "Options:"
    echo "    -c --clang       Compile with CLang. Default: use GCC."
    echo "    -h --help        Print this help text."
    echo "    -n --no-tests    Do not buid unit tests."
    echo "    -r --remove      Remove dependency. Lomse will not be linked with"
    echo "                     the specified library and the related functionality"
    echo "                     will not be included in Lomse. Valid values are:"
    echo "                     {zlib | libpng | fontconfig | threads }"
    echo "    -t --only-tests  Do not build. Only run unit tests if enabled."
    echo "    -w --web         Generate test results in local website folder."
    echo "                     If option -w is not specified, the results are"
    echo "                     generated in build folder."
    echo ""
}

#------------------------------------------------------------------------------
# Check values for dependencies and save in array
function mark_for_removing()
{
    #validate package and save option
    if [ "$1" == "fontconfig" ]; then
        OPTIONS="${OPTIONS} -DLOMSE_ENABLE_SYSTEM_FONTS:BOOL=OFF"
    elif [ "$1" == "libpng" ]; then
        OPTIONS="${OPTIONS} -DLOMSE_ENABLE_PNG:BOOL=OFF"
    elif [ "$1" == "unittest++" ]; then
        OPTIONS="${OPTIONS} -DLOMSE_BUILD_TESTS:BOOL=OFF"
    elif [ "$1" == "zlib" ]; then
        OPTIONS="${OPTIONS} -DLOMSE_ENABLE_COMPRESSION:BOOL=OFF"
    elif [ "$1" == "threads" ]; then
        OPTIONS="${OPTIONS} -DLOMSE_ENABLE_THREADS:BOOL=OFF"
    else
        display_error "Invalid package name '$1'. Valid values:"
        abort "    { fontconfig | libpng | unittest++ | zlib }"
    fi
}


#==============================================================================
# main line starts here
#==============================================================================

#get current directory and check we are running from <root>/scripts
#For this I just check that "src" folder exists
scripts_path="${PWD}"
root_path=$(dirname "${PWD}")
if [[ ! -e "${root_path}/src" ]]; then
    echo "Error: not running from <root>/scripts"
    exit $E_BADPATH
fi

#path for building
build_path="${root_path}/zz_build-area"
sources="${root_path}"

#paths for local website
website_root="/datos/cecilio/WebSite/mws"
website_pages="${website_root}/content/lenmus/lomse/html/lomse_en"

fGenerateForWeb=0
fRunTests=1
fBuild=1
fUseClang=0     #use CLang or GCC
OPTIONS=        #Options for building


#parse command line parameters
# See: https://stackoverflow.com/questions/192249/how-do-i-parse-command-line-arguments-in-bash
#
while [[ $# > 0 ]]
do
    key="$1"

    case $key in
        -c|--clang)
        fUseClang=1
        ;;
        -h|--help)
        DisplayHelp
        exit 1
        ;;
        -n|--no-tests)
        fRunTests=0
        OPTIONS="${OPTIONS} -DLOMSE_BUILD_TESTS:BOOL=OFF -DLOMSE_RUN_TESTS:BOOL=OFF"
        ;;
        -r|--remove)
            mark_for_removing "$2"
            shift
            ;;
        -t|--only-tests)
        fBuild=0
        ;;
        -w|--web)
        fGenerateForWeb=1
        ;;
        *) # unknown option 
        DisplayHelp
        exit 1
        ;;
    esac
    shift # past argument or value
done

# define the number of jobs to create (as many as the number of processors)
num_jobs=`getconf _NPROCESSORS_ONLN`

if [ ${fRunTests} -eq 0 ] && [ ${fBuild} -eq 0 ]; then
    echo "You have specified Build=No and RunTests=No. Nothing to do!"
fi

#build the local branch
if [ ${fBuild} -eq 1 ]; then
#create or clear build folder
    if [[ ! -e ${build_path} ]]; then
        #create build folder
        echo -e "${GREEN}Creating build folder${NC}"
        mkdir ${build_path}
        echo "-- Build folders created"
        echo ""
    elif [[ ! -d ${build_path} ]]; then
        #path exists but it is not a folder
        echo "Folder for building (${build_path}) already exists but is not a directory!"
        echo "Build aborted"
        exit $E_BUIL_ERROR
    else
        # clear build folders
        echo -e "${GREEN}Removing last build${NC}"
        cd "${build_path}" || exit $E_BADPATH
        rm * -rf
        echo "-- Build folders now empty"
        echo ""
    fi


    # create the makefile
    cd "${build_path}"
    echo -e "${GREEN}Creating makefile${NC}"
#    echo "options='${OPTIONS}'"
    if [ ${fUseClang} -eq 0 ]; then
        cmake -G "Unix Makefiles" ${OPTIONS} ${sources} || exit 1
    else
        cmake -G "Unix Makefiles" -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++ ${OPTIONS} ${sources} || exit 1
    fi

    # build lomse
    echo -e "${GREEN}Building liblomse. Will use ${num_jobs} jobs.${NC}"
    start_time=$(date -u +"%s")
    make -j$num_jobs || exit 1
    end_time=$(date -u +"%s")
    secs=$(($end_time-$start_time))
    echo "Build time: $(($secs / 60))m:$(($secs % 60))s"
fi


#-------------------------------------------------------------------
# build an html page with the unit tests results
#-------------------------------------------------------------------
echo -e "\nBuilding html page with test results..."


#set output page filename
html_page="${build_path}/unit-tests.htm"
if [ ${fGenerateForWeb} -eq 1 ]; then
    html_page="${website_pages}/unit-tests.htm"
fi

#create page from template
cd "${scripts_path}" || exit $E_BADPATH
sed -e "s/\${today}/`date +%Y-%m-%d`/"                  \
    -e "s/\${date-time}/`date +%Y-%m-%d_%H:%M:%S`/"     \
    unit-tests-template.txt > "${html_page}"

#add results
echo  "<pre class='console-black'><code>" >> "${html_page}"
echo  "${unit_tests}" >> "${html_page}"
echo  "</pre></code>" >> "${html_page}"
echo "</body>" >> "${html_page}"
echo "</html>" >> "${html_page}"

echo "HTML page built at ${html_page}"
exit $E_SUCCESS

