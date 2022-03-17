#! /bin/bash
#------------------------------------------------------------------------------
# Script 'build-api-docs.sh' regenerates the HTML doxygen version of the
# Lomse API documentation. Documentation is generated in a folder, named
# 'api-docs', at the same level than the project root folder. 
#
# The, this script copies the files from doxygen output dir to local repo, 
# commits the changes and pushes them to master repo for updating the API
# documentation
#
# usage: ./api-docs-to-repo.sh
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

#paths
branch_name=$(basename ${root_path})
projects=${scripts_path%/lomse/${branch_name}/scripts}
local_repo="${projects}/lenmus-io/lomse/"
from="${projects}/lomse/${branch_name}/zz_build-api/html"
echo "branch_name = ${branch_name}"
echo "root_path = ${root_path}"
echo "scripts_path = ${scripts_path}"
echo "projects = ${projects}"
echo "local_repo = ${local_repo}"
echo "from = ${from}"

#delete current content
echo -e "${enhanced}Delete old content${reset}"
cd "${local_repo}" || exit $E_BADPATH
rm * -r
echo "-- Local repo is now empty"
echo ""
cd ${local_repo}

#copy the new content
echo -e "${enhanced}Copy the new content${reset}"
echo "cp -ar ${from}/*.* ."
cp -ar ${from}/* .
echo "-- Files copied"
echo ""

#commit changes and push to origin
echo -e "${enhanced}Commit changes and push to origin${reset}"
git add .
git add -u
git commit -m"Update API documentation"
git push origin gh-pages
echo "-- Remote repo updated"
echo ""

echo "Done. Visit https://lenmus.github.io/lomse/"
exit $E_SUCCESS


