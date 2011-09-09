#! /bin/sh
#
# NAME
#     scarab-tag -- tag a release of Scarab in CVS
#
# SYNOPSIS
usage="scarab-tag RELEASE"
#
# DESCRIPTION
#     `scarab-tag' tags a release of Scarab in CVS with the tag
#     RELEASE.
#
#     `scarab-tag' is intended to be followed by `scarab-release' to
#     create a release using that tag.
#
#     By convention, RELEASE is of the form `rMAJOR_MINOR' or
#     `rMAJOR_MINORPOINT' where MAJOR is the major release number,
#     MINOR is the minor release number, and POINT is the point
#     release.  POINT releases are typically used in development and
#     beta stages only, but may occasionally be used for patch
#     releases.  POINT is usually of the form `dNNN' for development
#     or build snapshots, `bNNN' for betas, and `pNNN' for patch
#     levels.  Examples of release tags are `r0_01' and `r0_01d1'.
#
#     RELEASE is converted to the form `MAJOR.MINOR' or
#     `MAJOR.MINORPOINT' when releases are created, `0.01' and
#     `0.01d1'.
#
#     `scarab-tag' is tied to the layout of the Casbah CVS repository.
#     The majority of source comes from the Scarab CVS module and the
#     Java implementation comes from the Cairo/java CVS module.
#
#     The environment variable CVSROOT must be set and the account
#     used must be logged into prior to running `scarab-tag'.
#
#     `scarab-tag' exists primarily because the Scarab distribution is
#     built from multiple CVS modules that must be tagged with the
#     same RELEASE.
#
# SEE ALSO
#     scarab-release
#
# AUTHOR
#     Ken MacLeod
#
# $Id: scarab-tag.sh,v 1.1 2000/03/04 19:38:34 kmacleod Exp $
#

if [ "X$CVSROOT" = "X" ]; then
  echo "CVSROOT environment variable not set."
  exit 1
fi

if [ $# != 1 ]; then
  echo "usage: $usage"
  exit 1
fi

RELEASE="$1"

cvs rtag $RELEASE Scarab
cvs rtag $RELEASE cairo/java/org/casbah/ldo
