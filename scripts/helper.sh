# some helper functions

joinStrings () { local IFS="$1"; shift; echo "$*"; }

parseDescription () {
    local ver
    local description="$1"

    IFS='-' read -a ver <<< "${description}"
    local pattern="([0-9]+)(\.([0-9]+)(\.([0-9]+))?)?"
    if [[ ${ver[0]} =~ $pattern ]]; then
	major=${BASH_REMATCH[1]}
	minor=${BASH_REMATCH[3]}
	patch=${BASH_REMATCH[5]}
	package=$(joinStrings "." ${major} ${minor} ${patch} )
    else
	echo "error while parsing git version string '$description'" 1>&2
	major=; minor=; patch=; package=
    fi
    sha1=${ver[2]:1}
}
