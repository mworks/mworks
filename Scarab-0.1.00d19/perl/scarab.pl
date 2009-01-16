#
# Copyright (C) 1998 The Casbah Project
# See the file COPYING for distribution terms.
#
# $Id: scarab.pl,v 1.1 2000/03/04 19:38:34 kmacleod Exp $
#

=head1 NAME

scarab -- send requests to an Scarab object

=head1 SYNOPSIS

 rpc-client URL OBJECT METHOD [ARGS ...]

=head1 DESCRIPTION

`C<scarab>' exercises an Scarab server.  I<URL> is the URL of the server
to contact.  I<OBJECT> is the object to send the request to
(`C<root>', for example).  I<METHOD> is the procedure to call on the
remote server.

`C<ARGS>' is the argument list to be sent to the remote server.
I<ARGLIST> may include Perl array and hash constructors.

The result returned from the server is displayed using Perl's
`dumpvar.pl' utility, showing the internal structure of the objects
returned from the server.

=head1 EXAMPLES

 scarab localhost:1234 root echo "'Yeah baby.'"

=cut

use lib 'lib';

use RPC::Scarab::Manager;

die "usage: scarab URL OBJECT METHOD [ARGS ...]\n"
    if ($#ARGV < 2);

my $url = shift @ARGV;
my $object = shift @ARGV;
my $method = shift @ARGV;

$server = RPC::Scarab::Manager->new_rootproxy(connect_to => {PeerAddr => $url,
							  Proto => 'tcp'});


my @arglist;
my $arg;
foreach $arg (@ARGV) {
    my $arg_val;
    if ($arg =~ /\w/) {
        if ($arg =~ /^\s*['"[({]/) {
            eval "\$arg_val = $arg";
        } else {
            $arg_val = $arg;
        }
    } else {
        eval "\$arg_val = $arg";
    }
        
    push (@arglist, $arg_val);
}

$result = $server->$method (@arglist);

require 'dumpvar.pl';
dumpvar ('main', 'result');
