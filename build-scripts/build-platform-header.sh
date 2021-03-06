#!/bin/bash

#### build-platform-header.sh : Sets up the defines for the header file (specified as the 
####                            first argument to the script) that describe the build platform 
####                            target and the compile-time environment details. 
#### Author: Maxie D. Schmidt (maxieds@gmail.com)
#### Created: 2018.10.31

grepCmd=$(which grep);
sedCmd=$(which sed);
if [[ "$(uname -s)" == "Darwin" ]]; then
	grepCmd=$(which ggrep);
	sedCmd=$(which gsed);
fi

ReplaceHeaderComponent() {
     headerFile=$1;
     replacePattern=$2;
     replacementStr=$3;
     printVerbose=$4;
     if [[ $printVerbose == "1" ]]; then
     	echo "REPLACING \"${replacePattern}\" WITH \"${replacementStr}\" IN \"${headerFile}\"";
     fi
     #sedReplPattern=$(printf "\'s|%s|%s|\'" $replacementPattern $replacementStr);
     #echo "${sedReplPattern}"
     $sedCmd -i "s|${replacePattern}|${replacementStr}|" "${headerFile}";
}

OUTPUT_HEADER_FILE=$2
HEADER_SKELETON_FILE=$1

GIT_COMMITREV_HASHNUM=$(git show | head -n 1 | $sedCmd -e 's/commit //')
GIT_COMMITREV_HASHNUM_SHORT=$(echo "${GIT_COMMITREV_HASHNUM}" | cut -c-12)
GIT_COMMITREV_DATE=$(git show | $grepCmd Date: | head -n 1 | $sedCmd -e 's/Date:[ ]*//')
GIT_DESCRIBE_REVSTRING=$(git show --oneline --no-color | head -n 1 | $sedCmd -n 's/[a-zA-Z0-9][a-zA-Z0-9]* \(.*\)/\1/p')
BUILD_PLATFORM_IDSTRING=$(printf '%s (%s) [%s] @@ %s' $(uname -s) $(uname -r) $(uname -m) $(uname -n))
LOCAL_BUILD_TIME=$(date +"%c")
BUILD_FLTK_CONFIG=$3

REPL_PATTERNS=("##__GIT_COMMITREV_HASHNUM_SHORT__##" "##__GIT_COMMITREV_HASHNUM__##" \
	"##__GIT_COMMITREV_DATE__##" "##__GIT_DESCRIBE_REVSTRING__##" \
	"##__BUILD_PLATFORM_ID__##" \
	"##__BUILD_FLTK_CONFIG__##" \
	"##__LOCAL_BUILD_TIME__##" \
)
REPLACEMENTS=($GIT_COMMITREV_HASHNUM_SHORT $GIT_COMMITREV_HASHNUM \
	"${GIT_COMMITREV_DATE}" "${GIT_DESCRIBE_REVSTRING}" \
	"${BUILD_PLATFORM_IDSTRING}" \
	"${BUILD_FLTK_CONFIG}" \
	"${LOCAL_BUILD_TIME}" \
)

updatedStringData="";
for ridx in $(seq 0 5); do
	updatedStringData="${updatedStringData}:${REPLACEMENTS[$ridx]}";
done
updateHash=$(echo $updatedStringData | sha256sum);

PrintVerbose=$(cat ../build-scripts/BuildConfig.cfg | $sedCmd -n "s/VERBOSE=\([0-9][0-9]*\)/\1/p")
if [[ "$PrintVerbose" == "1" ]]; then
	echo -e "\n";
fi

if [[ -f "$OUTPUT_HEADER_FILE.hash" ]] && [[ "$(echo $updateHash | diff - $OUTPUT_HEADER_FILE.hash)" == "" ]]; then
	echo -e "${OUTPUT_HEADER_FILE} information already up-to-date ... \n";
else 
	echo "Copying \"${HEADER_SKELETON_FILE}\" to \"${OUTPUT_HEADER_FILE}\" ... "
	cp $HEADER_SKELETON_FILE $OUTPUT_HEADER_FILE
	for ridx in $(seq 0 6); do
		ReplaceHeaderComponent "${OUTPUT_HEADER_FILE}" "${REPL_PATTERNS[$ridx]}" "${REPLACEMENTS[$ridx]}" "$PrintVerbose";
	done
	if [[ "$PrintVerbose" == "1" ]]; then
		echo -e "\n";
	fi
	echo $updateHash > $OUTPUT_HEADER_FILE.hash;
fi

PrintLocalBuildHeaderFile=$(cat ../build-scripts/BuildConfig.cfg | $sedCmd -n "s/ECHO_CONFIG_HEADER=\([0-9][0-9]*\)$/\1/p")
if [[ "$PrintLocalBuildHeaderFile" == "1" ]]; then
	echo -e "LOCAL BUILD HEADER \"${OUTPUT_HEADER_FILE}\" CONTENTS:\n\n";
	cat $OUTPUT_HEADER_FILE;
	echo -e "\n";
	printf "FOR A TOTAL OF %s LINES (%s CHARACTERS) WRITTEN.\n\n" \
		$(wc -l $OUTPUT_HEADER_FILE | $grepCmd -o -e "[0-9]*") \
		$(wc -m $OUTPUT_HEADER_FILE | $grepCmd -o -e "[0-9]*");
fi

exit 0
