#!/Library/MonkeyWorks/bin/perl

# gonna need some of our C++ objects. 
use MonkeyScript;
# using Twig to parse the XML file.
use XML::Twig;
use FileHandle;

# need some utilities
require "XMLParser_Utilities.pm";
require "XMLParser_ForLoop.pm";
require "XMLParser_Environment.pm";

return;

$write_it = 1;
#$xmlfile = "/Users/davidcox/Desktop/NCR_MW_BUG/RSVP_motoGoBUG.xml";

# make sure the XML file was defined before we start.
if(!$xmlfile) {
    
	mwarning($MonkeyScript::M_PARSER_MESSAGE_DOMAIN,
			 "XMLRangeReplicatorExpander.pl xmlfile not defined.");
    die;
}

# Read the file
$file = new FileHandle($xmlfile);
@lines = <$file>;
$xmlstring = join('', @lines);
$file->close;

# do some preprocessing
$xmlstring = preprocessxml($xmlstring);

# expand any forloops that will expand "media" names
# We have to enumerate a lot of options because twig doesn't
# support full XPath syntax (which is a drag)

$handler = {	"$FORLOOP_STRING" => \&forloop,
				"$FOREACH_STRING" => \&foreachloop }; 

$xmlstring = parsexml($xmlstring, $handler);

# write the file
if($write_it){
	$file = new FileHandle(">$xmlfile");
	print $file $xmlstring;
	$file->close();
}
