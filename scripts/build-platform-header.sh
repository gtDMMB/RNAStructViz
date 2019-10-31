#!/bin/bash

#### build-platform-header.sh : Sets up the defines for the header file (specified as the 
####                            first argument to the script) that describe the build platform 
####                            target and the compile-time environment details. 
#### Author: Maxie D. Schmidt (maxieds@gmail.com)
#### Created: 2018.10.31

ReplaceHeaderComponent() {
     headerFile=$1;
     replacePattern=$2;
     replacementStr=$3;
     echo "REPLACING \"$replacePattern\" WITH \"$replacementStr\" IN \"$headerFile\"";
     sed -i -e "s|${replacePattern}|${replacementStr}|" "$headerFile";
}

OUTPUT_HEADER_FILE=$1
LOCAL_SCRIPTS_DIR=`echo $1 | sed -e 's/\/[a-zA-Z]*\/[a-zA-Z]*.h//'`
HEADER_SKELETON_FILE="${LOCAL_SCRIPTS_DIR}/scripts/BuildTargetInfo.h.in"

GIT_COMMITREV_HASHNUM=$(git show | head -n 1 | sed -e 's/commit //')
GIT_COMMITREV_HASHNUM_SHORT=$(echo $GIT_COMMITREV_HASHNUM | cut -c-12)
GIT_COMMITREV_DATE=$(git show | grep Date: | head -n 1 | sed -e 's/Date:[ ]*//')
GIT_DESCRIBE_REVSTRING=$(git describe --all --abbrev=6 HEAD^)
BUILD_PLATFORM_IDSTRING=$(printf "%s (%s) [%s] @@ %s" $(uname -s) $(uname -r) $(uname -m) $(uname -n))
LOCAL_BUILD_TIME=$(date +"%c")
BUILD_FLTK_CONFIG=$2

REPL_PATTERNS=("##__GIT_COMMITREV_HASHNUM_SHORT__##" "##__GIT_COMMITREV_HASHNUM__##" \
	"##__GIT_COMMITREV_DATE__##" "##__GIT_DESCRIBE_REVSTRING__##" \
	"##__BUILD_PLATFORM_ID__##" "##__LOCAL_BUILD_TIME__##"\
	"##__BUILD_FLTK_CONFIG__##"\
)
REPLACEMENTS=("${GIT_COMMITREV_HASHNUM_SHORT}" "${GIT_COMMITREV_HASHNUM}" \
	"${GIT_COMMITREV_DATE}" "${GIT_DESCRIBE_REVSTRING}" \
	"${BUILD_PLATFORM_IDSTRING}" "${LOCAL_BUILD_TIME}"\
	"${BUILD_FLTK_CONFIG}"\
)

echo -e "\n"
cp $HEADER_SKELETON_FILE $OUTPUT_HEADER_FILE;

for ridx in $(seq 0 6); do
	ReplaceHeaderComponent $OUTPUT_HEADER_FILE "${REPL_PATTERNS[$ridx]}" "${REPLACEMENTS[$ridx]}";
done
echo -e "\n"

echo -e "LOCAL BUILD HEADER \"$OUTPUT_HEADER_FILE\" CONTENTS:\n\n";
cat $OUTPUT_HEADER_FILE;
echo -e "\n";
printf "FOR A TOTAL OF %s LINES (%s CHARACTERS) WRITTEN.\n\n" \
	$(wc -l $OUTPUT_HEADER_FILE | grep -o -e "[0-9]*") \
	$(wc -m $OUTPUT_HEADER_FILE | grep -o -e "[0-9]*");


