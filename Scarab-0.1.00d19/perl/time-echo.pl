#! /usr/bin/perl

use lib 'lib';

use RPC::Scarab::Manager;

die "usage: $0 [--xml] URL [COUNT]\n"
    if ($#ARGV < 1);

my @marshal_args = ();
if ($ARGV[0] eq '--xml') {
    @marshal_args = (marshal => 'xml');
    shift;
}
my ($url, $count) = @ARGV;

$host = 'localhost' if !defined $host;

$server = RPC::Scarab::Manager->new_rootproxy(connect_to => {PeerAddr => $url,
							  Proto => 'tcp'},
					   @marshal_args);

$object = "object";

$time_zero = time();
for ($ii = 0; $ii < $count; $ii ++) {
    $server->echo($object);
}
$total_time = time() - $time_zero;
$ms = ($total_time / $count) * 1000;

print "$count calls in $total_time seconds ${ms}ms/call\n";
