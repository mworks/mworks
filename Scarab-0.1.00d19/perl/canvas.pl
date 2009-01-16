use lib 'lib';

use RPC::Scarab::Manager;
use Tk;

my $mw = MainWindow->new;
$mw->title('Bounce');

my $canvas = $mw->Canvas(qw/-height 3i -width 3i/);
$canvas->pack (qw/-side top -fill both/);

my $server = RPC::Scarab::Manager->new('root' => $canvas);
print "connect to me on port " . $server->receive_port->sockport . "\n";
$server->tk_fileevent($mw);

MainLoop;
