#!/bin/sh

REV=""
VER=0.1.0

if svn --version --quiet >/dev/null 2>&1; then
	REV=`svn info | grep "^Revision:" | cut -d" " -f2`
fi

if [ -z "$REV" ]; then
	REV=`git rev-parse --short HEAD`
fi

echo "rev: $REV"

if [ -n "$REV" ]; then
	echo "$VER-r$REV" > ./VERSION
else
	echo "$VER" > ./VERSION
fi
