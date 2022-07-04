#! /bin/bash
#------------------------------------------------------------------------------
# Lomse library visual tests for MusicXML exporter
# This script MUST BE RUN from 'lomse/scripts/' folder
# It creates a folder 'zz_musicxml/' at the same level than lomse root folder
# with the following content:
#
#  lomse-project
#       │
#       ├── lomse/
#       │     ├── src/
#       │     ├── scripts/
#       │     ┆
#       │
#       └── zz_musicxml/
#               ├── generated/
#               ├── failures/
#               ├── musicxml.htm
#               └── musicxml.css
#
# - Folder 'generated' contains the images for the exported scores
# - Folder 'failures' contains the GIF images for the test failures
# - File 'musicxml.htm' is the report
# - File 'musicxml.css' is a style sheet for the report.
#
#
# usage: ./musicxml-export-tests.sh [options]
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
# Display the help message
function DisplayHelp()
{
    echo "Usage: musicxml-export-tests.sh [option]*"
    echo ""
    echo "Options:"
    echo "    -h --help        Print this help text."
    echo "    -m --messages    Print message for each test file."
    echo ""
}


#------------------------------------------------------------------------------
# Generate images for files with the given pattern
function GenerateImages()
{
    for f in $1
    do
        testfile=$(basename "$f")
        if [ ${fDisplayMessages} -eq 1 ]; then
            echo "*** Processing test: ${testfile}"
        fi
        testfile="${testfile%.*}"
        IMAGE="${generated_path}${testfile}"
        msg=$(lclt -import "$f" -musicxml "z_${f}.xml" -import "z_${f}.xml" -jpg "${IMAGE}") || {
            echo "crash in $f"
            cp "$scripts_path/crash.jpg" "${IMAGE}-1.jpg"
        }
        if [ ! -z "${msg}" -a "${msg}" != " " ]; then
            echo -e "\n${testfile}"
            echo -e "${msg}" | sed 's/^/     /'
        fi
        rm "z_${f}.xml"
    done
}


#------------------------------------------------------------------------------
# add target images to html page
function AddTargetImagesFor()
{
    cd "${generated_path}"
    prev_test=""
    for f in $1
    do
        imagename="${f%.*}"
        testname="${f%-*}"

        #determine if failure
        if [[ " ${failures[@]} " =~ " ${imagename} " ]]; then
            #echo "Failure in ${testname}"
 
            # add test name if new test
            if [ "${prev_test}" != "${testname}" ]; then
                prev_test="${testname}"
                test_id="${testname%%-*}"
                echo "<a name='${test_id}'> </a>" >> "${html_page}"
                echo "<h4>${testname}<a class='gotop' href='#top'>[Go to top]</a></h4>" >> "${html_page}"
            fi

            # determine full path for image in webpage
            expected_img="${target_path}${f}"
            flicker_img="${results_path}${imagename}.gif"

            # add images to webpage
            echo "<img src='${expected_img}' />" >> "${html_page}"
            echo "<div style='display:inline-block'><img src='${flicker_img}' style='border:1px solid red' /><br /><span style='color:red'>Test failed: images comparison</span></div>" >> "${html_page}"

        fi

    done
}


#------------------------------------------------------------------------------
# main line starts here

E_SUCCESS=0         # success
E_NOARGS=65         # no arguments
E_BADPATH=66        # not running from lomse/trunk/scripts

#get current directory and check we are running from lomse/trunk/scripts
scripts_path="${PWD}"
root_path=$(dirname "${PWD}")
if [[ ! -e "${root_path}/src" ]]; then
    echo "Error: not running from <root>/scripts"
    exit $E_BADPATH
fi

#paths
root_parent_path=$(dirname "${root_path}")
vregress_path="${root_parent_path}/vregress"
scores_path="${root_path}/test-scores/"
extra_scores_path="${vregress_path}/scores"
css_path="${vregress_path}"
target_path="${vregress_path}/target/"
outpath="${root_parent_path}/zz_musicxml"
generated_path="${outpath}/generated/"
results_path="${outpath}/failures/"

#path for generated html page
html_path="${outpath}"

#echo "root_parent_path=${root_parent_path}"
#echo "vregress_path=${vregress_path}"
#echo "scores_path=${scores_path}"
#echo "extra_scores_path=${extra_scores_path}"
#echo "css_path=${css_path}"
#echo "target_path=${target_path}"
#echo "outpath=${outpath}"
#echo "generated_path=${generated_path}"
#echo "results_path=${results_path}"
#echo "html_path=${html_path}"

fDisplayMessages=0


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
        -m|--messages)
        fDisplayMessages=1
        ;;
        *) # unknown option
        DisplayHelp
        exit 1
        ;;
    esac
    shift # past argument or value
done


echo "Lomse library regression tests for MusicXML exporter"

FAILURES=0
SUCCESS=0

# declare an array variable for the names of failed tests
declare -a failures=()

#get Lomse version
lomse=$(lclt -vers)
lomse="${lomse##*library v}"
echo "Using lomse ${lomse}"

#create folders if they do not exist
if [ ! -d "${outpath}" ]; then
  mkdir -p "${outpath}"
fi
if [ ! -d "${generated_path}" ]; then
  mkdir -p "${generated_path}"
fi
if [ ! -d "${results_path}" ]; then
  mkdir -p "${results_path}"
fi


# clear results from previous tests
cd "${results_path}" || exit $E_BADPATH
rm * -f
cd "${generated_path}" || exit $E_BADPATH
rm * -f

# generate images
shopt -s nullglob
if [ 1 -eq 0 ]; then    #===== Commented out =====================
cd "${scores_path}"
GenerateImages "0*"
GenerateImages "5*"
fi    #===== End of commented out block =====================

cd "${extra_scores_path}/lilypond"
GenerateImages "*.xml"

if [ 1 -eq 0 ]; then    #===== Commented out =====================
cd "${extra_scores_path}/recordare"
GenerateImages "*.musicxml"
cd "${scores_path}"
cd "./mnx"
GenerateImages "*.mnx"
fi    #===== End of commented out block =====================

# trim images
cd "${generated_path}" || exit $E_BADPATH
for f in *.jpg
do
    convert -trim "$f" trimmed.jpg
    rm "$f"
    convert -bordercolor White -border 20x20 trimmed.jpg "$f"
    rm trimmed.jpg
done


# compare the generated images with the 'good' ones
cd "${generated_path}"
for f in *.jpg
do
    TARGET="${target_path}${f}"
    #echo "Compating ${TARGET} with ${f}"
    if [ ! -f "${TARGET}" ]; then
        #file not found!. Copy generated to target
        cp "$f" "${TARGET}"
        echo "No target image for ${f}. Copied from generated"
        SUCCESS=$((SUCCESS+1))
    else
        outname="${results_path}${f}"
        outname=${outname%.*}
		#compare sizes
		IFS=x read w h < <(identify "$f" | grep -oP '\d+x\d+(?=\+)')
		size1=$((w*h))
		#echo "${size1} is the size for file ${f}"
		IFS=x read w h < <(identify "${TARGET}" | grep -oP '\d+x\d+(?=\+)')
		size2=$((w*h))
		#echo "${size2} is the size for file ${TARGET}"
		if [ $size1 -ne $size2 ]; then
			#they differ in size
            echo "Files $f and ${RESULT} have different sizes"
            testname="${f%.*}"
            failures+=("${testname}")
            convert -delay 50 "$f" "${TARGET}" -loop 0 "${outname}.gif"
            FAILURES=$((FAILURES+1))
        else
		    RESULT=$(compare -metric AE -fuzz 5% "$f" "${TARGET}" -compose Src differences.jpg  2>&1)
		    #RESULT=$?
		    #echo "Compating ${TARGET} with ${generated_path}${f}. Result: ${RESULT}"
		    if [ $RESULT -ne 0 ]; then
		        #There are differences. Generate flicker image
		        echo "File $f differs in ${RESULT} pixels."
		        testname="${f%.*}"
		        failures+=("${testname}")
		        convert -delay 50 "$f" "${TARGET}" -loop 0 "${outname}.gif"
		        FAILURES=$((FAILURES+1))
		    else
		        SUCCESS=$((SUCCESS+1))
		    fi
        	rm differences.jpg
		fi
    fi
done


#-------------------------------------------------------------------
# build an html page with the failures
#-------------------------------------------------------------------

#set output page filename
html_page="${outpath}/musicxml.htm"

#create page from template
cd "${scripts_path}"
sed -e "s/\${today}/`date +%Y-%m-%d`/"                  \
    -e "s/\${lomse-version}/${lomse}/"                  \
    -e "s/\${date-time}/`date +%Y-%m-%d_%H:%M:%S`/"     \
    musicxml-template.txt > "${html_page}"

#add results statistics
tests="test"
if [ $SUCCESS -gt 1 ]; then
    tests="tests"
fi
if [ $FAILURES -eq 0 ]; then
    echo  "<li>Results: <span style='color:Green'>All tests (${SUCCESS}) passed. </span></li>" >> "${html_page}"
else
    printf "<li>Results: <span style='color:Green'>${SUCCESS} ${tests} passed. </span>" >> "${html_page}"
    tests="test"
    if [ $FAILURES -gt 1 ]; then
        tests="tests"
    fi
    printf " <span style='color:Red'>${FAILURES} ${tests} failed.</span>" >> "${html_page}"
    echo "</li>" >> "${html_page}"
fi

#add links to failures
if [ $FAILURES -ne 0 ]; then
    echo "<li>Tests failed:" >> "${html_page}"
    echo "<ul>" >> "${html_page}"
    i="0"
    while [ $i -lt $FAILURES ]
    do
        test_id="${failures[$i]}"
        test_id="${test_id%%-*}"
        echo "<li><a href='#${test_id}'>${failures[$i]}.jpg</a></li>" >> "${html_page}"
        i=$[$i+1]
    done
    echo "</ul></li>" >> "${html_page}"
fi

echo "</ul>" >> "${html_page}"

echo "<h2>Generated images</h2>" >> "${html_page}"
echo "<p>For each test, the first image is the expected result. If the test has failed, the test title is displayed in red and an additional flicker image is added. This image flips between the expected one and the generated one, so that any differences are easily spotted.</p>" >> "${html_page}"

echo "<h2>Failures</h2>" >> "${html_page}"
AddTargetImagesFor "0001*.jpg"
AddTargetImagesFor "0002*.jpg"
AddTargetImagesFor "0003*.jpg"
AddTargetImagesFor "0004*.jpg"
AddTargetImagesFor "0005*.jpg"
AddTargetImagesFor "0006*.jpg"
AddTargetImagesFor "0007*.jpg"
AddTargetImagesFor "0008*.jpg"
AddTargetImagesFor "0009*.jpg"
AddTargetImagesFor "0010*.jpg"
AddTargetImagesFor "0011*.jpg"
AddTargetImagesFor "0012*.jpg"
AddTargetImagesFor "0013*.jpg"
AddTargetImagesFor "0014*.jpg"
AddTargetImagesFor "0018*.jpg"
AddTargetImagesFor "0019*.jpg"
AddTargetImagesFor "0020*.jpg"
AddTargetImagesFor "0021*.jpg"
AddTargetImagesFor "0022*.jpg"
AddTargetImagesFor "0023*.jpg"
AddTargetImagesFor "0024*.jpg"
AddTargetImagesFor "0025*.jpg"
AddTargetImagesFor "006*.jpg"

AddTargetImagesFor "0101*.jpg"
AddTargetImagesFor "0102*.jpg"
AddTargetImagesFor "0103*.jpg"
AddTargetImagesFor "0104*.jpg"

AddTargetImagesFor "0201*.jpg"
AddTargetImagesFor "0202*.jpg"
AddTargetImagesFor "0203*.jpg"
AddTargetImagesFor "0204*.jpg"
#AddTargetImagesFor "0205*.jpg"
#AddTargetImagesFor "0206*.jpg"
AddTargetImagesFor "0207*.jpg"
AddTargetImagesFor "0208*.jpg"
AddTargetImagesFor "0209*.jpg"

AddTargetImagesFor "070*.jpg"

AddTargetImagesFor "0801*.jpg"
AddTargetImagesFor "0802*.jpg"
AddTargetImagesFor "0803*.jpg"
AddTargetImagesFor "0804*.jpg"

AddTargetImagesFor "090*.jpg"

AddTargetImagesFor "50*.jpg"

# Lilypond tests
AddTargetImagesFor "01a*.jpg"
AddTargetImagesFor "01b*.jpg"
AddTargetImagesFor "01c*.jpg"
AddTargetImagesFor "01d*.jpg"
AddTargetImagesFor "01e*.jpg"
AddTargetImagesFor "01f*.jpg"
AddTargetImagesFor "02a*.jpg"
AddTargetImagesFor "02b*.jpg"
AddTargetImagesFor "02c*.jpg"
AddTargetImagesFor "02d*.jpg"
AddTargetImagesFor "02e*.jpg"
AddTargetImagesFor "03a*.jpg"
AddTargetImagesFor "03b*.jpg"
AddTargetImagesFor "03c*.jpg"
AddTargetImagesFor "03d*.jpg"
AddTargetImagesFor "1*.jpg"
AddTargetImagesFor "2*.jpg"
AddTargetImagesFor "3*.jpg"
AddTargetImagesFor "4*.jpg"
AddTargetImagesFor "51*.jpg"
AddTargetImagesFor "52*.jpg"
AddTargetImagesFor "6*.jpg"
AddTargetImagesFor "7*.jpg"
AddTargetImagesFor "8*.jpg"
AddTargetImagesFor "9*.jpg"

# Recordare tests
AddTargetImagesFor "A*.jpg"
AddTargetImagesFor "B*.jpg"
AddTargetImagesFor "C*.jpg"
AddTargetImagesFor "D*.jpg"
AddTargetImagesFor "E*.jpg"
AddTargetImagesFor "F*.jpg"
AddTargetImagesFor "M*.jpg"
AddTargetImagesFor "S*.jpg"
AddTargetImagesFor "T*.jpg"

# MNX importer tests
AddTargetImagesFor "mnx*.jpg"


echo "</div>  <!-- main -->" >> "${html_page}"
echo "</body>" >> "${html_page}"
echo "</html>" >> "${html_page}"


#copy the css file
cp "${css_path}/musicxml.css" "${outpath}/musicxml.css"

# upload the html page and the folder with the images to the server
#echo "Uploading results to local server"

#echo "done"


#-------------------------------------------------------------------
#Inform about results
#-------------------------------------------------------------------

if [ $FAILURES -eq 0 ]; then
    msg="All tests (${SUCCESS}) passed."
	echo -e "\n${msg}"
else
    msg="${FAILURES} test(s) failed. ${SUCCESS} test(s) passed."
	echo -e "\n${msg}"
	printf '%s\n' "${failures[@]}"
fi

#echo "Results at http://localhost/en/lomse/regression"
echo "Results at file://${html_path}/musicxml.htm"

exit $E_SUCCESS
