#!/bin/bash

#### get-build-config-setting.sh : Grok a build config setting out of the specified BuildConfig.cfg files;
#### Usage: ./get-build-config-setting.sh <BUILD-CONFIG-CFGFILE-PATH> <CONFIG-SETTING-STRING-SPEC>

if [[ $# -lt 2 ]]; then
	echo "Invalid parameters passed to script $0 ... Exiting! (-1)"
	exit -1;
fi

$buildConfigFilePath=$1
$configSettingSpec=$2

GGREP=`which grep`;
GSED=`which sed`;
if [[ "$(uname -s)" == "Darwin" ]]; then
        GGREP=`which ggrep`;
        GSED=`which gsed`;
fi

GetBuildConfigSetting() {
        buildConfigFile=$0;
        buildConfigSetting=$1;
        settingValue=$(cat $buildConfigFile | $GSED -n "s/${buildConfigSetting}=\([0-9][0-9]*\)/\1/p");
        echo -n "${settingValue}";
}

$configSettingValue=$(GetBuildConfigSetting $buildConfigFilePath $configSettingPath)
if [[ "$configSettingValue" == "" ]]; then
	echo "Unable to find build config setting \"${buildConfigSetting}\" ... Exiting! (-2)";
	exit -2;
fi

echo -n $configSettingValue;
exit 0;
