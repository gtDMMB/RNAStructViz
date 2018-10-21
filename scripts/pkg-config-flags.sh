#!/bin/bash

PKGS=(glu xext xfixes fontconfig xrender cairo-xlib-xrender xinerama cairo xpm)
PKGOUT=""

for pkg in "${PKGS[@]}"; do
	pkg-config --silence-errors --exists $pkg
	if [ "$?" == "0" ]; then
		PKGOUT="${PKGOUT} `pkg-config $1 $pkg`"
	fi
done

echo $PKGOUT
