#!/bin/bash

PKGS=(glu xext xfixes fontconfig xrender cairo-xlib-xrender xinerama cairo xpm)
PKGOUT=""

for pkg in "${PKGS[@]}"; do
	pkg-config --silence-errors --exists $pkg
	if [ "$?" == "0" ]; then
		PKGOUT="${PKGOUT} `pkg-config $1 $pkg`"
	fi
done

# Conditional library flags for Linux and Mac platform:
if [[ "$(uname -s)" == "Linux" && "$1" == "--libs" ]]; then
	LINUX_EXTRA_DIR=$(echo $(ls -d /usr/lib/*-linux-gnu) | tr " " "\n" | head -n 1);
	if [[ "${LINUX_EXTRA_DIR}" != "" ]]; then
		PKGOUT+=" -L${LINUX_EXTRA_DIR} -LX11";
	fi
fi

# Define the target OS (Linux or Mac or Unix)
if [[ "$(uname -s)" == "Linux" ]]; then
	PKGOUT+=" -DTARGETOS_LINUX"
elif [[ "$(uname -s)" == "Darwin" ]]; then
	PKGOUT+=" -DTARGETOS_APPLE" #-mmacosx-version-min=10.6"
else 
	PKGOUT+=" -DTARGETOS_GENERIC_UNIX"
fi

echo $PKGOUT
