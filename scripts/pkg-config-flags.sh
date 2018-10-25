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
	PKGOUT+=" -L$(ls -d /usr/lib/$(uname -m)-linux-gnu)"
fi

# Define the target OS (Linux or Mac or Unix)
if [[ "$(uname -s)" == "Linux" ]]; then
	PKGOUT+=" -DTARGETOS_LINUX"
elif [[ "$(uname -s)" == "Darwin" ]]; then
	PKGOUT+=" -DTARGETOS_APPLE"
else 
	PKGOUT+=" -DTARGETOS_GENERIC_UNIX"
fi

echo $PKGOUT
