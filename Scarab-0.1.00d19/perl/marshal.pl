use lib 'lib';

use IO::File;
require 'dumpvar.pl';

my $file = new IO::File "+>myfile";



my $marshal;
if ($ARGV[0] eq '--xml') {
    require Marshal::LDO_XML;
    import Marshal::LDO_XML;
    $marshal = Marshal::LDO_XML->new ( stream => $file );
} else {
    require Marshal::LDO_Binary;
    import Marshal::LDO_Binary;
    $marshal = Marshal::LDO_Binary->new ( stream => $file );
}

$marshal->freeze( [
		    {
			method => '__cg_get__',
			object => 'root',
			binary => "\x01\x02\x82",
		    }
		    ] );

$file->seek(0, 0);

$obj = $marshal->thaw;
dumpvar('main', 'obj');
