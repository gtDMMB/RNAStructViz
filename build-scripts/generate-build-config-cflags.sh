#!/bin/bash

#### generate-build-config-cflags.sh : Wrapper script to generate the compile-time defines and other 
####                                   options from the local BuildConfig.cfg file. This script is 
####                                   called from the src/Makefile with a single parameter indicating 
####                                   the path to the BuildConfig.cfg config file to be parsed. 
####                                   Note that this script is necessary on Mac since the version of `make` 
####                                   does not properly implement our shorthand call function, 
####                                   GetBuildConfigOption, quite as intended on Linux and other Unix platforms ... 

BUILD_CONFIG_SRCFILE=$1

GGREP=`which grep`;
GSED=`which sed`;
if [[ "$(uname -s)" == "Darwin" ]]; then
        GGREP=`which ggrep`;
        GSED=`which gsed`;
fi

GetBuildConfigSetting() {
	buildConfigFile=$1;
	buildConfigSetting=$2;
	#echo "GROKING BUILD CONFIG FILE: ${buildConfigFile} // ${buildConfigSetting} ... " >&2;
	settingValue=$(cat $buildConfigFile | $GSED -n -e "s|${buildConfigSetting}=\([0-9][0-9]*\)|\1|p");
	echo -n "${settingValue}"
}

CFGFILE_OPTIONS_LIST=(\
	"BRANCH_TYPE_ID" \
	"EXTERNAL_SAMPLING_SUPPORT" \
	"VIENNARNA_SUPPORT" \
	"RNASTRUCTURE_SUPPORT" \
	"NCBIDB_SUPPORT" \
	"BETA_TESTING_FEATURES_SUPPORT" \
	"USE_LEAK_SANITIZER" \
	"WITH_FASTA_FORMAT_SUPPORT" \
)

DASHD_DEFINES_CFLAG_SPECS=(\
	"PERFORM_BRANCH_TYPE_ID" \
	"BUILD_WITH_EXTERNAL_SAMPLING_SUPPORT" \
	"BUILD_WITH_VIENNARNA" \
	"BUILD_WITH_RNASTRUCTURE" \
	"BUILD_WITH_NCBI_LOOKUP_SUPPORT" \
	"__RNASTRUCTVIZ_ENABLE_BETA_TESTING_FEATURES" \
	"WITHGPERFTOOLS" \
	"WITH_FASTA_FORMAT_SUPPORT" \
)

EXTRA_CFLAGS_LIST=""
numSettings=${#CFGFILE_OPTIONS_LIST[@]};
for sidx in $(seq 1 $numSettings); do
	arrIndex=$(($sidx - 1));
	cfgSetting=$(GetBuildConfigSetting $BUILD_CONFIG_SRCFILE ${CFGFILE_OPTIONS_LIST[$arrIndex]});
	defineName=${DASHD_DEFINES_CFLAG_SPECS[$arrIndex]};
	if [[ "${cfgSetting}" == "" ]]; then
		cfgSetting=0;
	fi
	EXTRA_CFLAGS_LIST="${EXTRA_CFLAGS_LIST} -D${defineName}=${cfgSetting}";
done

echo -n "${EXTRA_CFLAGS_LIST}"
exit 0
