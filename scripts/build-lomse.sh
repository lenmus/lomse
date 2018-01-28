#! /bin/bash
#------------------------------------------------------------------------------
# Building and unit testing Lomse library
# This script MUST BE RUN from <root>/scripts/ folder
#
# usage: ./build-lomse.sh
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
# Display the help message
function DisplayHelp()
{
    echo "Usage: ./bulid-lomse.sh [option]*"
    echo ""
    echo "Options:"
    echo "    -h --help        Print this help text."
    echo "    -w --web         Generate test results in local website folder."
    echo "                     If option -w is not specified, the results are"
    echo "                     generated in build folder."
    echo "    -t --only-tests  Do not build. Only run unit tests."
    echo ""
}


#------------------------------------------------------------------------------
# main line starts here

E_SUCCESS=0         # success
E_NOARGS=65         # no arguments
E_BADPATH=66        # not running from lomse/trunk/scripts
E_BADARGS=67        # bad arguments

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

#path for building
build_path="${root_path}/build-area"
sources="${root_path}"

#paths for local website
website_root="/datos/USR/WebSite/mws"
website_pages="${website_root}/content/lenmus/lomse/html/lomse_en"

fGenerateForWeb=0
fOnlyTests=0


#parse command line parameters
# See: https://stackoverflow.com/questions/192249/how-do-i-parse-command-line-arguments-in-bash
#
while [[ $# > 0 ]]
do
    key="$1"

    case $key in
        -h|--help)
        DisplayHelp
        exit 1
        ;;
        -t|--only-tests)
        fOnlyTests=1
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


#build the local branch
if [ ${fOnlyTests} -eq 0 ]; then
#create or clear build folder
    if [[ ! -e ${build_path} ]]; then
        #create build folder
        echo -e "${enhanced}Creating build folder${reset}"
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
        echo -e "${enhanced}Removing last build${reset}"
        echo -e "${enhanced}Removing last build${reset}"
        cd "${build_path}" || exit $E_BADPATH
        rm * -r
        echo "-- Build folders now empty"
        echo ""
    fi

    # create the makefile
    cd "${build_path}"
    echo -e "${enhanced}Creating makefile${reset}"
    cmake -G "Unix Makefiles" ${sources} || exit 1

    # build lomse
    echo -e "${enhanced}Building liblomse. Will use ${num_jobs} jobs.${reset}"
    start_time=$(date -u +"%s")
    make -j$num_jobs || exit 1
    end_time=$(date -u +"%s")
    secs=$(($end_time-$start_time))
    echo "Build time: $(($secs / 60))m:$(($secs % 60))s"
fi

#run unit tests
echo -e "${enhanced}Running unit tests${reset}"
cd "${build_path}"
unit_tests=`./bin/testlib`
echo "${unit_tests}"


#-------------------------------------------------------------------
# build an html page with the unit tests results
#-------------------------------------------------------------------
echo -e "${enhanced}Building html page with test results${reset}"


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

