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

OUTPUT_HEADER_FILE=$1
HEADER_SKELETON_FILE="../scripts/BuildTargetInfo.h.in"

GIT_COMMITREV_HASHNUM=$(git show | head -n 1 | $sedCmd -e 's/commit //')
GIT_COMMITREV_HASHNUM_SHORT=$(echo "${GIT_COMMITREV_HASHNUM}" | cut -c-12)
GIT_COMMITREV_DATE=$(git show | $grepCmd Date: | head -n 1 | $sedCmd -e 's/Date:[ ]*//')
GIT_DESCRIBE_REVSTRING=$(git show --oneline --no-color | head -n 1 | $sedCmd -n 's/[a-zA-Z0-9][a-zA-Z0-9]* \(.*\)/\1/p')
BUILD_PLATFORM_IDSTRING=$(printf '%s (%s) [%s] @@ %s' $(uname -s) $(uname -r) $(uname -m) $(uname -n))
LOCAL_BUILD_TIME=$(date +"%c")
BUILD_FLTK_CONFIG=$2

REPL_PATTERNS=("##__GIT_COMMITREV_HASHNUM_SHORT__##" "##__GIT_COMMITREV_HASHNUM__##" \
	"##__GIT_COMMITREV_DATE__##" "##__GIT_DESCRIBE_REVSTRING__##" \
	"##__BUILD_PLATFORM_ID__##" "##__LOCAL_BUILD_TIME__##" \
	"##__BUILD_FLTK_CONFIG__##" \
)
REPLACEMENTS=($GIT_COMMITREV_HASHNUM_SHORT $GIT_COMMITREV_HASHNUM \
	"${GIT_COMMITREV_DATE}" "${GIT_DESCRIBE_REVSTRING}" \
	"${BUILD_PLATFORM_IDSTRING}" "${LOCAL_BUILD_TIME}" \
	"${BUILD_FLTK_CONFIG}" \
)

PrintVerbose=$(cat ../scripts/BuildConfig.cfg | $sedCmd -n "s/VERBOSE=\([0-9][0-9]*\)/\1/p")
if [[ "$PrintVerbose" == "1" ]]; then
	echo -e "\n";
fi

cp $HEADER_SKELETON_FILE $OUTPUT_HEADER_FILE;

for ridx in $(seq 0 6); do
	ReplaceHeaderComponent "${OUTPUT_HEADER_FILE}" "${REPL_PATTERNS[$ridx]}" "${REPLACEMENTS[$ridx]}" "$PrintVerbose";
done
if [[ "$PrintVerbose" == "1" ]]; then
	echo -e "\n";
fi

PrintLocalBuildHeaderFile=$(cat ../scripts/BuildConfig.cfg | $sedCmd -n "s/ECHO_CONFIG_HEADER=\([0-9][0-9]*\)$/\1/p")
if [[ "$PrintLocalBuildHeaderFile" == "1" ]]; then
	echo -e "LOCAL BUILD HEADER \"${OUTPUT_HEADER_FILE}\" CONTENTS:\n\n";
	cat $OUTPUT_HEADER_FILE;
	echo -e "\n";
	printf "FOR A TOTAL OF %s LINES (%s CHARACTERS) WRITTEN.\n\n" \
		$(wc -l $OUTPUT_HEADER_FILE | $grepCmd -o -e "[0-9]*") \
		$(wc -m $OUTPUT_HEADER_FILE | $grepCmd -o -e "[0-9]*");
fi

exit 0
