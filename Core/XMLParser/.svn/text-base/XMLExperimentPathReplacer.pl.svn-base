#!/Library/MonkeyWorks/bin/perl

# gonna need some of our C++ objects. 
use MonkeyScript;
# using Twig to parse the XML file.
use XML::Twig;

# need some utilities
require "XMLParser_Utilities.pm";
require "XMLParser_ForLoop.pm";
require "XMLParser_Environment.pm";





# make sure the XML file was defined before we start.
if(!$xmlfile) {
    mwarning($MonkeyScript::M_PARSER_MESSAGE_DOMAIN,
			 "XMLExperimentPathReplacer.pl xmlfile not defined.");
    die;
}

if(!$localExperimentPath) {
    mwarning($MonkeyScript::M_PARSER_MESSAGE_DOMAIN,
			 "XMLExperimentPathReplacer.pl localExperimentPath not defined.");
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

#pprint("-------------------\n\n\n\n$xmlstring\n\n\n\n-------------");

$xmlstring = parsexml($xmlstring, { '_all_' => \&foundMedia });

# write the file
$file = new FileHandle("> $xmlfile");
print $file $xmlstring;
$file->close();

#open(FD,">$xmlfile");
#print FD "$xmlstring";
#close(FD);


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

sub foundMedia {
    my($locTwig, $element) = @_;
    # gets the media type ex: Stimulus
    my $type = $element->gi;
    # search for the tag in the media list.
    my $found = "";
    foreach (($SOUND_STRING, $STIMULUS_STRING, $IMPORT_STRING)) {
        if($type eq $_) {
            $found = $type;
        }
    }
    if($found eq "") {
        return;
    } else {
        # get the attribute tag i am looking for
        my $attTag = $PATH_STRING;
        # get the path from element
        my $path = $element->att($attTag);
		# DDC 6/05: not all "media tags" will include a path field
		#			It shouldn't crash in those cases
		# DDC 4/06: wait, maybe they should... this media manager thing is total
		#			shit
		if($path eq "") {
			return;
		}
		
        # edit the XML file to replace the path with the modified path.
		my $oldpath = $path;
        
		$path = $localExperimentPath . "/" . squashPath($path);
		#$path = $localExperimentPath . "/" . trimPath($path);
#pprint("Old path = $oldpath; New path = $path");
        $element->set_att($attTag, $path);
    }
	
	return $element;
}