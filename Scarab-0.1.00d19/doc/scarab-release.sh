#! /bin/sh
#
# NAME
#     scarab-release -- create a release of Scarab
#
# SYNOPSIS
usage="scarab-release RELEASE"
#
# DESCRIPTION
#     `scarab-release' creates a release of Scarab from CVS based on
#     the CVS tag RELEASE.
#
#     `scarab-rtag' is intended to follow `scarab-tag' that applies a
#     tag to the CVS repository.  The convention used for the format
#     of RELEASE is described in `scarab-tag'.
#
#     RELEASE is converted to the form `MAJOR.MINOR' or
#     `MAJOR.MINORPOINT' when releases are created, `0.01' and
#     `0.01d1'.
#
#     `scarab-release' defaults to the Casbah CVS repository and is
#     tied to layout of the Casbah CVS repository.  The majority of
#     source comes from the Scarab CVS module and the Java
#     implementation comes from the Cairo/java CVS module.
#
#     The environment variable CVSROOT must be set and the account
#     used must be logged into prior to running `scarab-rtag'.
#     Otherwise, the anonymous CVS account is used.
#
#     NOTICE: only Scarab maintainers should make official release
#     distributions.  Users or organizations should NOT use
#     `scarab-release' to create their own copies of official
#     distributions (these could leak out and be confused with
#     official releases), please use only the official distributions.
#
#     Note, the above notice applies only to releases made by the
#     Scarab maintainers.  Scarab is open-source, you are free to
#     create your own distributions if you please, we merely ask that
#     you don't create releases that could be confused with other
#     releases.
#
# SEE ALSO
#     scarab-tag
#
# AUTHOR
#     Ken MacLeod
#
# $Id: scarab-release.sh,v 1.1 2000/03/04 19:38:33 kmacleod Exp $
#

CVSROOT=${CVSROOT:-":pserver:anonymous@casbah.org:/usr/local/Casbah/src/cvsroot"}
export CVSROOT

AUTOCONF="/usr/bin/autoconf"
GREP="/usr/bin/grep"
PERL="/usr/bin/perl"
AWK="/usr/bin/awk"
CVS="/usr/bin/cvs"
PWD_CMD="/bin/pwd"
SED="sed"
TR="/usr/bin/tr"

if [ $# != 1 ]; then
  echo "usage: $usage"
  exit 1
fi

RELEASE="$1"

set -e
set -x

original_dir="`$PWD_CMD`"

release="`echo \"$RELEASE\" | $SED -e 's/r\([0-9]\{1,\}\)m\([0-9]\{1,\}\)\(.*\)/\1.\2\3/'`"
release="`echo \"$release\" | $SED -e 's/^r//' | $TR '_' '.'`"

mkdir /tmp/dist-$$

cd /tmp/dist-$$

# HERE: this is the place where it picks up various source from the CVS
# repository
$CVS export -r $RELEASE -d dist Scarab
$CVS export -r $RELEASE -d jdist cairo/java/org/casbah/ldo
mkdir -p dist/java/org/casbah
mv jdist dist/java/org/casbah/ldo
rm dist/java/org/casbah/ldo/.cvsignore

cd dist

mv configure.in ..

# edit the SYMBOLIC_TAG into `configure.in', translating to a
# ``cleaner'' release number
$SED <../configure.in >configure.in \
  -e 's/^VERSION=.*$/VERSION='"$release"'/'

echo "$release" >.release

$AUTOCONF

./configure

make dist

cd ..

chmod a-w *.tar.gz

mv *.tar.gz $original_dir

cd ..

rm -rf dist-$$
