#! /bin/bash
#------------------------------------------------------------------------------
# Lomse library visual regression test
# This script MUST BE RUN from 'lomse/scripts/' folder
# It creates a folder 'zz_regression/' at the same level than lomse root folder
# with the following content:
#
#  lomse-project
#       │
#       ├── lomse/
#       │     ├── src/
#       │     ├── scripts/
#       │     ┆
#       │
#       └── zz_regression/
#               ├── generated/
#               ├── failures/
#               ├── regression.htm
#               └── regression.css
#
# - Folder 'generated' contains the images for the rendered scores
# - Folder 'failures' contains the GIF images for the test failures
# - File 'regression.htm' is the report
# - File 'regression.css' is a style sheet for the report.
#
#
# usage: ./regression.sh [options]
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
# Display the help message
function DisplayHelp()
{
    echo "Usage: regression.sh [option]*"
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
        msg=$(lclt -import "$f" -export "${IMAGE}")
        if [ ! -z "${msg}" -a "${msg}" != " " ]; then
            echo -e "\n${testfile}"
            echo -e "${msg}" | sed 's/^/     /'
        fi
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
        fTestFailed=0
        if [[ " ${failures[@]} " =~ " ${imagename} " ]]; then
            #echo "Failure in ${testname}"
            fTestFailed=1
        fi

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
        if [ ${fTestFailed} -eq 1 ]; then
            echo "<img src='${expected_img}' />" >> "${html_page}"
            echo "<div style='display:inline-block'><img src='${flicker_img}' style='border:1px solid red' /><br /><span style='color:red'>Test failed: images comparison</span></div>" >> "${html_page}"
        else
            echo "<img src='${expected_img}' />" >> "${html_page}"
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
outpath="${root_parent_path}/zz_regression"
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


echo "Lomse library visual regression tests"

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
cd "${scores_path}"
GenerateImages "0*"
GenerateImages "5*"
cd "${extra_scores_path}/lilypond"
GenerateImages "*.xml"
cd "${extra_scores_path}/recordare"
GenerateImages "*.musicxml"
cd "${scores_path}"
cd "./mnx"
GenerateImages "*.mnx"


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
html_page="${outpath}/regression.htm"

#create page from template
cd "${scripts_path}"
sed -e "s/\${today}/`date +%Y-%m-%d`/"                  \
    -e "s/\${lomse-version}/${lomse}/"                  \
    -e "s/\${date-time}/`date +%Y-%m-%d_%H:%M:%S`/"     \
    regression-template.txt > "${html_page}"

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

echo "<h2>Scores. Basic layout<a name='00000'></a></h2>" >> "${html_page}"
echo "<h3>Empty scores<a name='0001x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0001*.jpg"
echo "<h3>Score prolog<a name='0002x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0002*.jpg"
echo "<h3>Notes<a name='0003x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0003*.jpg"
AddTargetImagesFor "0004*.jpg"
echo "<h3>Accidentals<a name='0005x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0005*.jpg"
echo "<h3>Rests<a name='0006x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0006*.jpg"
echo "<h3>Chords<a name='0007x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0007*.jpg"
echo "<h3>Chords<a name='0008x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0008*.jpg"
echo "<h3>Spacing non-timed objs.<a name='0009x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0009*.jpg"
echo "<h3>Clefs<a name='0010x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0010*.jpg"
echo "<h3>Key signatures<a name='0011x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0011*.jpg"
echo "<h3>Time signatures<a name='0012x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0012*.jpg"
echo "<h3>Vertical alignment<a name='0013x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0013*.jpg"
AddTargetImagesFor "0014*.jpg"
echo "<h3>Instructions<a name='0018x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0018*.jpg"
echo "<h3>Barlines / measures<a name='0019x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0019*.jpg"
echo "<h3>Systems justification<a name='0020x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0020*.jpg"
echo "<h3>Instruments<a name='0021x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0021*.jpg"
AddTargetImagesFor "0022*.jpg"
echo "<h3>Staves spacing<a name='0023x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0023*.jpg"
echo "<h3>justification/truncation<a name='0024x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0024*.jpg"
AddTargetImagesFor "0025*.jpg"
echo "<h3>Spacing algorithm<a name='006xx'></a></h3>" >> "${html_page}"
AddTargetImagesFor "006*.jpg"

echo "<h2>Relations<a name='01xxx'></a></h2>" >> "${html_page}"
echo "<h3>Tuplets<a name='0101x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0101*.jpg"
echo "<h3>Beams<a name='0102x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0102*.jpg"
echo "<h3>Ties<a name='0103x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0103*.jpg"
echo "<h3>Slurs<a name='0104x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0104*.jpg"

echo "<h2>Attachments<a name='02xxx'></a></h2>" >> "${html_page}"
echo "<h3>Lines<a name='0201x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0201*.jpg"
echo "<h3>Fermatas<a name='0202x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0202*.jpg"
echo "<h3>Metronome marks<a name='0203x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0203*.jpg"
echo "<h3>Attached texts<a name='0204x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0204*.jpg"
#echo "<h3>Text boxes<a name='0205x'></a></h3>" >> "${html_page}"
#AddTargetImagesFor "0205*.jpg"
#echo "<h3>Figured bass<a name='0206x'></a></h3>" >> "${html_page}"
#AddTargetImagesFor "0206*.jpg"
echo "<h3>Dynamics<a name='0207x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0207*.jpg"
echo "<h3>Articulations<a name='0208x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0208*.jpg"
echo "<h3>Lyrics<a name='0209x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0209*.jpg"

echo "<h2>TimeGrid<a name='070xx'></a></h2>" >> "${html_page}"
AddTargetImagesFor "070*.jpg"

echo "<h2>Other top level objects<a name='080xx'></a></h2>" >> "${html_page}"
echo "<h3>Paragraphs<a name='0801x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0801*.jpg"
echo "<h3>Tables<a name='0802x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0802*.jpg"
echo "<h3>Widgets<a name='0803x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0803*.jpg"
echo "<h3>Images<a name='0804x'></a></h3>" >> "${html_page}"
AddTargetImagesFor "0804*.jpg"

echo "<h2>Full documents<a name='090xx'></a></h2>" >> "${html_page}"
AddTargetImagesFor "090*.jpg"

echo "<h2>MusicXML importer<a name='5xxxx'></a></h2>" >> "${html_page}"
echo "<h3>Lomse tests<a name='lomse'></a></h2>" >> "${html_page}"
AddTargetImagesFor "50*.jpg"
echo "<h3>Lilypond tests<a name='lilypond'></a></h2>" >> "${html_page}"
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
echo "<h3>Recordare tests<a name='recordare'></a></h2>" >> "${html_page}"
AddTargetImagesFor "A*.jpg"
AddTargetImagesFor "B*.jpg"
AddTargetImagesFor "C*.jpg"
AddTargetImagesFor "D*.jpg"
AddTargetImagesFor "E*.jpg"
AddTargetImagesFor "F*.jpg"
AddTargetImagesFor "M*.jpg"
AddTargetImagesFor "S*.jpg"
AddTargetImagesFor "T*.jpg"

echo "<h2>MNX importer<a name='mnx'></a></h2>" >> "${html_page}"
AddTargetImagesFor "mnx*.jpg"


echo "</div>  <!-- main -->" >> "${html_page}"
echo "</body>" >> "${html_page}"
echo "</html>" >> "${html_page}"


#copy the css file
cp "${css_path}/regression.css" "${outpath}/regression.css"

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
echo "Results at file://${html_path}/regression.htm"

exit $E_SUCCESS
