#!/Library/MonkeyWorks/bin/perl

use MonkeyScript;   # MonkeyWorks C++ (swig) interface
use XML::Twig;          # Parser Library
use FileHandle;

require "XMLParser_MonkeyWorks.pm";
require "XMLParser_Environment.pm";
require "XMLParser_Utilities.pm";
require "XMLParser_AssignVariables.pm";

$VERBOSE = 3;

#  -----------------------------------------------------------
#  STEP 2:  Read and parse the XML file
#  -----------------------------------------------------------
if(-e $xmlfile) {
    $file = new FileHandle($xmlfile);
    @lines = <$file>;
    $xmlstring = join('', @lines);
    
    $xmlstring = preprocessxml($xmlstring);

    $xmlstring = parsexml($xmlstring, {$VARIABLE_ASSIGNMENT_STRING => \&AssignVariable});
}
