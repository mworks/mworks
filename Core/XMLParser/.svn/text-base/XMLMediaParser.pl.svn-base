#!/Library/MonkeyWorks/bin/perl


# TO DO !!!!!
# I hate you Paul you motherfucker!!!!!!!
# Why does this file coexist with XMLExperimentPathReplacer?
# Why does everything you did suck so hard?

# An XML parser used to collect pathnames from elements that reference 
# files that need to be transferred to a remote machine.

# Usage: The loading utilities function that calls this script first sets
# the variable $xmlfile to the path of the XML file we want to parse.  All
# the information we need for parsing is stored in the Perl Media IO Manager.
# It contains the names of the elements that contain media we are interested
# in.  The paths for the files are taken from these elements and stored
# back into the Perl Media IO Manager.  They are used later to load the files
# and send them across the network.  The XML file is altered so that the paths
# of the media are created in a certain place on the remote machine, and
# are able to load when needed.

# gonna need some of our C++ objects. 
use MonkeyScript;
# using Twig to parse the XML file.
use XML::Twig;
# file pathing
use File::Basename;
use File::Spec;
use Cwd;
use FileHandle;


# need some utilities
require "XMLParser_Utilities.pm";
require "XMLParser_ForLoop.pm";
require "XMLParser_Environment.pm";

# make sure the XML file was defined before we start.
if(!$xmlfile) {
    mwarning($MonkeyScript::M_PARSER_MESSAGE_DOMAIN,
			"XMLMediaParser.pl xmlfile not defined.");
    die;
}

our $return = new MonkeyScript::mData($MonkeyScript::M_LIST, 1);
# some variable we will need
my $twig; # twig xml

# Read the file
$file = new FileHandle($xmlfile);
@lines = <$file>;
$xmlstring = join('', @lines);

# do some preprocessing
$xmlstring = preprocessxml($xmlstring);

# expand any forloops that will expand "media" names
# We have to enumerate a lot of options because twig doesn't
# support full XPath syntax (which is a drag)
$handler = {	"$FORLOOP_STRING" => \&forloop,
				"$FOREACH_STRING" => \&foreachloop }; 

$xmlstring = parsexml($xmlstring, $handler);


# parse the twig and at every element call foundMedia
#$twig = new XML::Twig(TwigHandlers => { '_all_' => \&foundMedia } );
#$twig->parsefile($xmlfile);

$xmlstring = parsexml($xmlstring, {'_all_' => \&foundMedia });

$return->DISOWN;
MonkeyScript::setPerlReturn($return);

##############################################
#                   Subroutines
##############################################

# trims all path components from a string just leaving
# the file name and a / i.e /filename
sub trimPath {
    my($locPath) = @_;
    my $lastDir = rindex($locPath, '/') + 1;
    substr($locPath, $lastDir); # the return value is the filename
}

# turn a hierarchical path (e.g. path/to/file) into a "squashed" one 
# (i.e. path_to_file)
sub squashPath {
	my($locPath) = @_;
	$locPath =~ s{\/}{_}sg;
	return $locPath;
}

sub resolvePath {
    my($fpath) = @_;
    my $resolvme;
    # need to get the xmlfile path
    ($xmlbase, $xmlpath, $xmltype) = fileparse($xmlfile);
    SWITCH: {
        # match paths that start with ..
        if($fpath =~ /^\.\./) {
            $resolveme = $xmlpath . $fpath;
            last SWITCH;
        }
        # matches a single dot slash and replaces it with nothing
        if($fpath =~ s/^\.\///) {
            $resolveme = $xmlpath . $fpath;
            last SWITCH;
        }
        # matches a / slash a the begining
        if($fpath =~ /^\//) {
            $resolveme = $fpath;
            last SWITCH;
        }
        # supports \ in paths
        if($fpath =~ /^\\/) {
            $resolveme = $fpath;
            last SWITCH;
        }
        # the file doesnt begin with any of the file path markers
        # so lets just append it to the xml file path and pray
        # note that if there is a tilde (~) in the path then this method
        # will return an empty string which should flag a usr error 
        # down the pipe.
        $resolveme = $xmlpath . $fpath;
    }
#    pprint("resolving -> $resolveme\n");
    return Cwd::realpath($resolveme);
}

sub foundMedia {
	
    my($locTwig, $element) = @_;
    # gets the media type ex: stimulus
    my $type = $element->gi;
    # search for the tag in the media list.
    my $found = "";
    foreach (($STIMULUS_STRING, $SOUND_STRING, $IMPORT_STRING)) {
        if($type eq $_) {
            $found = $type;
        }
    }
    if($found eq "") {
        return;
    } else {
		
        # get the attribute tag i am looking for
        my $attTag = "path";
        # get the path from element
        my $mediaFilePath = $element->att($attTag);
#		pprint("original path -> $mediaFilePath\n");
		
		# DDC 6/05: not all media tags will include a path field
		# It shouldn't crash in those cases
		if($mediaFilePath eq "") {
			return;
		}
        # the path can be in four different forms
        # it could start with a '~' or a '/' or '..' or '.'
        # so we have to fix any paths to be absolute so we can get their
        # data when we move back to C++ land. $path = 
        my $resolved = resolvePath($mediaFilePath);
#pprint("resolved -> $resolved\n");
        # put the path into the media manager
        if(!$resolved) { $resolved = ""; }
		
		my $pathData = new MonkeyScript::mData($MonkeyScript::M_DICTIONARY, 2);

		my $fullPath = new MonkeyScript::mData();
		$fullPath->setString($resolved, length($resolved)+1);
		$fullPath->DISOWN;
		
		my $relPath = new MonkeyScript::mData();
		$relPath->setString($mediaFilePath, length($mediaFilePath)+1);
		$relPath->DISOWN;
		
		my $fullPathString = "full_path";
		my $fullPathKey = new MonkeyScript::mData();
		$fullPathKey->setString($fullPathString, length($fullPathString)+1);
		$fullPathKey->DISOWN;
		
		my $relPathString = "relative_path";
		my $relPathKey = new MonkeyScript::mData();
		$relPathKey->setString($relPathString, length($relPathString)+1);
		$relPathKey->DISOWN;
		
		
		$pathData->addElement($fullPathKey, $fullPath);
		$pathData->addElement($relPathKey, $relPath);
		$pathData->DISOWN;
		
		$return->addElement($pathData);
    }
}