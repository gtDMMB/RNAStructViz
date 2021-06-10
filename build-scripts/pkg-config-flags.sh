#!/bin/bash

GREP=$(which grep);
SED=$(which sed);
if [[ "$(uname -s)" == "Darwin" ]]; then
     GREP=$(which ggrep);
     SED=$(which gsed);
fi

PKGCONFIG=`which pkg-config`
if [[ "$(uname -s)" == "Darwin" ]]; then 
	export PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:/usr/local/opt/libffi/lib/pkgconfig:/usr/local/opt/viennarna/lib/pkgconfig";
fi


PKGS=(glu xext xfixes fontconfig xrender cairo-xlib-xrender xinerama cairo xpm RNAlib2 libpng)
PKGOUT=""

for pkg in "${PKGS[@]}"; do
	$PKGCONFIG --silence-errors --exists $pkg
	if [ "$?" == "0" ]; then
		PKGOUT="${PKGOUT} `pkg-config $1 $pkg`"
	fi
done

# Conditional library flags for Linux and Mac platform:
if [[ "$(uname -s)" == "Linux" && "$1" == "--libs" ]]; then
	if stat --printf='' /usr/lib/*-linux-gnu 2>/dev/null
     then
          LINUX_EXTRA_DIR=$(echo $(ls -d /usr/lib/*-linux-gnu) | tr " " "\n" | head -n 1);
     elif stat --printf='' /usr/lib/gcc/*-linux-gnu 2>/dev/null
     then
          LINUX_EXTRA_DIR=$(echo $(ls -d /usr/lib/gcc/*-linux-gnu) | tr " " "\n" | head -n 1);
     fi
	if [[ "${LINUX_EXTRA_DIR}" != "" ]]; then
		PKGOUT+=" -L${LINUX_EXTRA_DIR} -LX11";
	fi
fi

# Define the target OS (Linux or Mac or Unix)
if [[ "$(uname -s)" == "Linux" ]]; then
	PKGOUT+=" -DTARGETOS_LINUX"
elif [[ "$(uname -s)" == "Darwin" ]]; then
	PKGOUT+=" -DTARGETOS_MACOSX" #-mmacosx-version-min=10.6"
else 
	PKGOUT+=" -DTARGETOS_GENERIC_UNIX"
fi

PKGOUT=`echo $PKGOUT | $SED 's/-fno-lto -Wl,-fno-lto//g' | $SED 's/-Wl,-fno-lto//g' | $SED 's/-fno-lto//g'`
#PKGOUT=`echo $PKGOUT | $SED 's/ -fno-lto//g'`

echo $PKGOUT
