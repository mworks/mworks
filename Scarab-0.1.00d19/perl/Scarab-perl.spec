Summary: Perl module for Lightweight Distributed Objects
Name: Scarab-perl
Version: @VERSION@
Release: 1
Source: ftp://ftp.casbah.org/pub/casbah/Scarab-perl-@VERSION@.tar.gz
Copyright: MPL or GPL
Group: Networking
URL: http://casbah.org/Scarab/
Packager: ken@bitsko.slc.ut.us (Ken MacLeod)
BuildRoot: /tmp/Scarab-perl

#
# $Id: Scarab-perl.spec,v 1.1 2000/03/04 19:38:34 kmacleod Exp $
#

%description
RPC::Scarab is a Perl module that implements Casbah's Scarab
communications framework.  Scarab can be used to for messaging,
distributed objects, and remote procedure calls between multiple
languages and systems.  Scarab can be used as a transport over HTTP
and TCP/IP or for complex Web/CGI applications.

%prep
%setup

perl Makefile.PL INSTALLDIRS=perl

%build

make

%install

make PREFIX="${RPM_ROOT_DIR}/usr" pure_install

DOCDIR="${RPM_ROOT_DIR}/usr/doc/Scarab-perl-@VERSION@-1"
mkdir -p "$DOCDIR/examples"
for ii in README test.pl; do
  cp $ii "$DOCDIR/$ii"
  chmod 644 "$DOCDIR/$ii"
done
for ii in canvas.pl echo-server.pl scarab.pl marshal.pl time-echo.pl; do
  cp $ii "$DOCDIR/examples/$ii"
  chmod 644 "$DOCDIR/examples/$ii"
done

%files

/usr/doc/Scarab-perl-@VERSION@-1

/usr/lib/perl5/RPC/marshal.pl
/usr/lib/perl5/RPC/Scarab/Connection.pm
/usr/lib/perl5/RPC/Scarab/Proxy.pm
/usr/lib/perl5/RPC/Scarab/Manager.pm
/usr/lib/perl5/RPC/time-echo.pl
/usr/lib/perl5/RPC/canvas.pl
/usr/lib/perl5/RPC/scarab.pl
/usr/lib/perl5/RPC/echo-server.pl
/usr/lib/perl5/Marshal/LDO_XML.pm
/usr/lib/perl5/Marshal/LDO_Binary.pm
/usr/lib/perl5/man/man3/RPC::Scarab.3
/usr/lib/perl5/man/man3/Marshal::LDO_Binary.3
/usr/lib/perl5/man/man3/Marshal::LDO_XML.3
