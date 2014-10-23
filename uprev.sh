#!/bin/sh

REV=""
VER=0.1.0

if [ -z "$REV" ]; then
	REV=`git rev-parse --short HEAD`
fi

if [ -z "$REV" -a svn --version --quiet >/dev/null 2>&1 ]; then
	REV=`svn info | grep "^Revision:" | cut -d" " -f2`
fi

echo "rev: $REV"

if [ -n "$REV" ]; then
	echo "$VER-r$REV" > ./VERSION
else
	echo "$VER" > ./VERSION
fi
