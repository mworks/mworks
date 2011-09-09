#
# Copyright (C) 1998 The Casbah Project
# See the file COPYING for distribution terms.
#
# $Id: Makefile.PL,v 1.2 2000/03/04 19:38:34 kmacleod Exp $
#

use ExtUtils::MakeMaker;

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'	=> 'RPC::Scarab',
    'VERSION_FROM' => 'lib/RPC/Scarab/Manager.pm',
    dist        => {'COMPRESS'  => 'gzip', 'SUFFIX'    => '.gz'},
);
