#!/bin/bash

PKGS=(glu xext xfixes fontconfig xrender cairo-xlib-xrender xinerama cairo xpm)
PKGOUT=""

for pkg in "${PKGS[@]}"; do
	pkg-config --silence-errors --exists $pkg
	if [ "$?" == "0" ]; then
		PKGOUT="${PKGOUT} `pkg-config $1 $pkg`"
	fi
done

if [[ "$(uname -o)" == "GNU/Linux" && "$1" == "--libs" ]]; then
	PKGOUT+=" -L$(ls -d /usr/lib/$(uname -m)-linux-gnu)"
fi

echo $PKGOUT
