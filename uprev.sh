#!/bin/sh

REV=0
#VER=`cat ./VERSION | cut -c1-5`
VER=0.1.0

if svn --version --quiet >/dev/null 2>&1; then
	REV=`svn info | grep "^Revision:" | cut -d" " -f2`
fi

echo "$VER-r$REV" > ./VERSION
