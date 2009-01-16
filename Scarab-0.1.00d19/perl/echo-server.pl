use lib 'lib';

use IO::Socket;
use RPC::Scarab::Manager;

my $object = Echo->new;

my $server = RPC::Scarab::Manager->new('root' => $object);

print "connect to me on port " . $server->receive_port->sockport . "\n";

$server->run_loop;


######################################################################
###
### This is the class that the client talks to
###

# Echo has one method, `echo'.  `echo' returns it's first argument
# back to the caller.

package Echo;

sub new {
    my $type = shift;

    bless {}, $type;
}

sub echo {
    my $self = shift;
    my $string = shift;

    return $string;
}
