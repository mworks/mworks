#! /bin/sh
#
# NAME
#     scarab-web -- update web page with an Scarab release
#
# SYNOPSIS
usage="scarab-web RELEASE DESTDIR"
#
# DESCRIPTION
#     `scarab-web' creates a web mirror using an Scarab release tar
#     file.
#
#     `scarab-web' is intended to follow `scarab-release' or be used
#     by mirror sites.
#
#     `scarab-web' pulls files from the tar file to create the web
#     page.  `scarab-web' searches HTML files for the string @VERSION@
#     and replaces it with the converted RELEASE.
#
#     RELEASE is converted to the form `MAJOR.MINOR' or
#     `MAJOR.MINORPOINT' when releases are created, `0.01' and
#     `0.01d1'.
#
#     `scarab-web' installs the web pages in DESTDIR.
#
#     CAUTION: `scarab-web' removes the contents of DESTDIR before
#     copying files to it.
#
# SEE ALSO
#     scarab-tag, scarab-release
#
# AUTHOR
#     Ken MacLeod
#
# $Id: scarab-web.sh,v 1.1 2000/03/04 19:38:34 kmacleod Exp $
#

PWD_CMD="/bin/pwd"
SED="sed"
TR="/usr/bin/tr"

if [ $# != 2 ]; then
  echo "usage: $usage"
  exit 1
fi

RELEASE="$1"
DESTDIR="$2"

release="`echo \"$RELEASE\" | $SED -e 's/r\([0-9]\{1,\}\)m\([0-9]\{1,\}\)\(.*\)/\1.\2\3/'`"
release="`echo \"$release\" | $SED -e 's/^r//' | $TR '_' '.'`"

set -e
set -x

rm -rf $DESTDIR
mkdir -p $DESTDIR

cp Scarab-${release}.tar.gz $DESTDIR

cd $DESTDIR

tar xzvf Scarab-${release}.tar.gz

for ii in Scarab-${release}/doc/*.html Scarab-${release}/doc/*.txt; do
  $SED <$ii >`basename $ii` \
    -e "s/@VERSION@/$release/g"
done

rm -rf Scarab-${release}
