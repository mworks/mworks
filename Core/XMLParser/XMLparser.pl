#!/Library/MonkeyWorks/bin/perl

# XMLparser.pl
#
# Description: An XML parser written in Perl.
# (using the swig-generated MonkeyScript Perl API) 
#
#
# History:
# Dave Cox on ??/??/?? - Created.
# Paul Jankunas on 05/16/05 - Added support for a name attribute field
#                           for state elements ex: Protocols Blocks.  Also
#                           added support for description elements within
#                           states so they can be reported to users.
# Paul Jankunas on 05/19/05 - Added base support for list,vector,dictionary
#                       mData types in variable function even though they are
#                       not yet in the system.  Made corrections to variable
#                       function to create proper mInterfaceSettting objects.
# Dave Cox	on 01/19/06  - Restructured the parser so that the paradigm 
#						components are built up in two passes, one that 
#						instantiates the objects, and a second pass that 
#						connects them up.  Also added support for referenced
#						paradigm components and for loops
# Jim DiCarlo 02/21/06  Added calibrator parsing (mirroring Filter)
#
# Copyright 2005 MIT. All rights reserved.
#


use MonkeyScript;   # MonkeyWorks C++ (swig) interface
use XML::Twig;		# Parser Library
use FileHandle;
use Switch;
use XML::Schematron;
use XML::Schematron::XPath;
#use Tree::Nary;	# validation trees.

# Parser modules
require "XMLParser_MonkeyWorks.pm";		# if we're inside an embedded interp.
require "XMLParser_Environment.pm";		# strings and globals (a lot here)
require "XMLParser_Utilities.pm";
require "XMLParser_Stimuli.pm";
require "XMLParser_Sounds.pm";
require "XMLParser_Variables.pm";
require "XMLParser_Experiments.pm";
require "XMLParser_States.pm";
require "XMLParser_Transitions.pm";
require "XMLParser_Actions.pm";
require "XMLParser_IODevices.pm";
require "XMLParser_Triggers.pm";
require "XMLParser_Filters.pm";
require "XMLParser_Calibrators.pm";
require "XMLParser_ForLoop.pm";
require "XMLParser_Import.pm";
require "XMLParser_AdaptiveOptimizers.pm";

$VERBOSE = 3;

$PARSER_ERROR_FLAG = 0;

#mprintf("Blah! Fie on you! (1)");
#merror($MonkeyScript::M_PARSER_MESSAGE_DOMAIN, "Blah! Fie on you! (2)");



my %object_pool = {};


#  -----------------------------------------------------------
#  STEP 1:  Instantiate Global Objects if they are not already
#  -----------------------------------------------------------

if(!($MonkeyScript::GlobalIODeviceManager)){
	$MonkeyScript::GlobalIODeviceManager = new MonkeyScript::mIODeviceManager();
}

if(!($MonkeyScript::GlobalVariableRegistry)){
	$MonkeyScript::GlobalVariableRegistry = new MonkeyScript::mVariableRegistry();
}

if(!($MonkeyScript::GlobalFilterManager)){
	$MonkeyScript::GlobalFilterManager = new MonkeyScript::mFilterManager();
#pprint("Creating global filter manager ...");
}

if(!($MonkeyScript::GlobalOpenALContextManager)){
	$MonkeyScript::GlobalOpenALContextManager = 
							new MonkeyScript::mOpenALContextManager();
}

if(!($MonkeyScript::GlobalCollectionPoint)){
	$MonkeyScript::GlobalCollectionPoint = 
							new MonkeyScript::mCollectionPoint();
}

#if(!($MonkeyScript::GlobalOpenGLContextManager)){
#	$MonkeyScript::GlobalOpenGLContextManager = 
#							new MonkeyScript::mOpenGLContextManager();
#}

# TODO -- need global calibrator manager

$variable_registry = $MonkeyScript::GlobalVariableRegistry;


#  -----------------------------------------------------------
#  STEP 2a:  Read the XML file
#  -----------------------------------------------------------

$file = new FileHandle($xmlfile);
@lines = <$file>;
$xmlstring = join('', @lines);


# do some preprocessing
$xmlstring = preprocessxml($xmlstring);


#  -----------------------------------------------------------
#  STEP 2b:  Validate the XML using a Schematron
#  -----------------------------------------------------------
$schematron_file = "/Library/MonkeyWorks/Scripting/Perl/MonkeyWorksSchematron.xml";
$schematron = new XML::Schematron::XPath(schema => $schematron_file);
$| = 1;
#print "verifying XML";
#@messages = $schematron->verify($xmlstring);
@messages = ();
if(@messages){
	parser_error(0,
				 "There were syntax errors in the XML file ($xmlfile):");
	foreach $message (@messages){
		parser_error(0,
			   "$message");
	}
	
	exit(0);
}


#  -----------------------------------------------------------
#  STEP 3:  Parse the XML file
#  -----------------------------------------------------------


# Handle imports; these files will simply be pasted in
# Fancier import facilities may be added as necessary
$xmlstring = parsexml($xmlstring, {$IMPORT_STRING => \&ParseImport});


# Do all of the "lexical expansion" parser passes.  These will transform
# the XML into a different form.  Replicators (here, "forloops") are the
# primary example of this
$handler = {	$FORLOOP_STRING => \&forloop,
				$FOREACH_STRING => \&foreachloop };

$xmlstring = parsexml($xmlstring, $handler);


# strip out folders
$xmlstring =~ s{<\s*folder.*?\/?>}{}igs;
$xmlstring =~ s{<\/\s*folder.*?\/?>}{}igs;


# Parse the Experiment object (but don't connect anything to it yet)
$xmlstring = parsexml($xmlstring, {$EXPERIMENT_STRING => \&Experiment});

# Parse Variables
# Other objects will need these, so do them up front
$xmlstring = parsexml($xmlstring, {$VARIABLE_STRING => \&Variable});
$xmlstring = parsexml($xmlstring, {$SELECTION_VARIABLE_STRING => 
													\&SelectionVariable});



# Parse things that might generate new variables 
my $handler = { $IODEVICE_STRING => \&createIODevices,
			 $TRIGGER_STRING  => \&Trigger,
			 $FILTER_STRING   => \&Filter,
			 $CALIBRATOR_STRING   => \&Calibrator,
			 $STAIRCASE_OPTIMIZE_STRING => \&StaircaseOptimizer };

$xmlstring = parsexml($xmlstring, $handler);


$xmlstring = parsexml($xmlstring, {$IODEVICE_STRING => \&mapIODevices});


# Rebuild the experiment's variable contexts in case there are new variables
# built in the last step
# TODO: do this more elegantly?
$MonkeyScript::GlobalCurrentExperiment->createVariableContexts();


# prepare the stimulus display (this is a hack to have it here; must fix)
prepareStimulusDisplay();


# Parse stimuli
my $handler = { $STIMULUS_STRING => \&Stimulus,
				$STIMULUS_GROUP_STRING => \&stimulus_group,
				$SOUND_STRING => \&Sound };

$xmlstring = parsexml($xmlstring, $handler);


# Pre-parse stimulus nodes defined in actions
$handler = { $ACTION_STRING => \&StimulusNodeDefinedInAction };
$xmlstring = parsexml($xmlstring, $handler);


# Parse Paradigm Components

# Do an instantiation pass; the objects themselves are created, but none of 
# the child/parent relationships are set up at this point
$handler = { $TASK_SYSTEM_STATE_STRING => \&TaskSystemState_Instantiate,
			 $TASK_SYSTEM_STRING	   => \&TaskSystem_Instantiate,
			 $TRIAL_STRING			   => \&Trial_Instantiate,
			 $LIST_STRING			   => \&List_Instantiate,
			 $BLOCK_STRING			   => \&Block_Instantiate,
			 $TRIAL_STRING			   => \&Trial_Instantiate,
			 $PROTOCOL_STRING		   => \&Protocol_Instantiate};
$xmlstring = parsexml($xmlstring, $handler);



# Do a referencing pass; We want to be dealing with "references" to the objects
# rather than the objects themselves.  Here a paradigm component "reference"
# means a shallow copy + it's own unique variable context
# This allows each reference to the object to carry its own variable_assignment
# "payload"
$handler = { #$TASK_SYSTEM_STATE_STRING => \&createInstance,
			 $TASK_SYSTEM_STRING	   => \&createInstance,
			 $TRIAL_STRING			   => \&createInstance,
			 $LIST_STRING			   => \&createInstance,
			 $BLOCK_STRING			   => \&createInstance,
			 $TRIAL_STRING			   => \&createInstance,
			 $PROTOCOL_STRING		   => \&createInstance};
$xmlstring = parsexml($xmlstring, $handler);


# Do a connection pass; Now we actually connect children to parents
# For the most part, we are attaching paradigm "references" to the actual
# object.  Dig down into the connect calls for more details.
$handler = {	$BLOCK_STRING				=>	\&Block_Connect,
				$LIST_STRING				=>  \&List_Connect,
				$TRIAL_STRING				=>  \&Trial_Connect,
				$PROTOCOL_STRING			=>	\&Protocol_Connect,
				$EXPERIMENT_STRING			=>	\&Experiment_Connect,
				$TASK_SYSTEM_STATE_STRING	=>	\&TaskSystemState_Connect,
				$TASK_SYSTEM_STRING			=>	\&TaskSystem_Connect};
$xmlstring = parsexml($xmlstring, $handler);


# Connect up "spring-loaded" actions that are subordinate to variables
# This is done as a separate pass because variables need to done first, but
# the actions can refer to just about anything else in the system
$xmlstring = parsexml($xmlstring, 
						{$VARIABLE_STRING => \&SpringLoadedVariableAction });




#  -----------------------------------------------------------
#  STEP 4:  Clean-up, and make sure that everything went OK
#  -----------------------------------------------------------

if ($PARSER_ERROR_FLAG) {  # something went wrong

	top_level_parser_error( 
				"Errors were encountered while parsing this experiment. " .
				"Please consult the error logs for details");
	
	unloadExperiment(); # kill everything!
	#$MonkeyScript::GlobalCurrentExperiment = 0; # this signals that the
												# experiment did not load
												# correctly
		
	return;
}

#if($VERBOSE > 0){
#	pprint("Finished parsing XML.\n");
#}

# I think that the following code is vestigial, but I'll keep it around for
# a little bit, just in case
#@all_defined_tags = keys(%object_pool);
#@defined_experiment_tags = grep {$_ =~ m"Experiment_"} @all_defined_tags;

#if(length(@defined_experiment_tags) == 0){
#	mwarning("No valid experiment objects were defined...");
#} else {
#	if($VERBOSE > 2){
#		mprintf("$object_pool{$defined_experiment_tags[0]}");
#	}
#}










