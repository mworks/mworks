#!/Library/MonkeyWorks/bin/perl

# XMLWriteVar.pl
#
# Description: Creates and modifies the XML for persistant varaibles
#
# History:
# Ben Kennedy 07/26/06 - Created
#
# Copyright 2006 MIT. All rights reserved.
#


use XML::Twig;		# Parser Library
use MonkeyScript;

require "XMLParser_Utilities.pm";
require "XMLParser_Environment.pm";

if(!defined($xmlfile)){
    die "\$xmlfile is not defined";
}

if(!defined($var_name)) {
    die "\$var_name is not defined";
}


unless (-e $xmlfile){ 
    # create a boilerplate XML file
    create_XML($xmlfile);
}


my $curr_variable = get_raw_pointer($MonkeyScript::GlobalVariableRegistry)->getVariable(new_string($var_name));
my $data = get_raw_pointer($curr_variable)->getValue();

my $var_elt = XML::Twig::Elt->new($VARIABLE_ASSIGNMENT_STRING);

my $tag_elt = XML::Twig::Elt->new($TAG_STRING);
$tag_elt->set_text($var_name);
$tag_elt->paste($FIRST_CHILD_STRING, $var_elt);

my $data_elt = XML::Twig::Elt->new($DATA_STRING);
$data_elt->paste($LAST_CHILD_STRING, $var_elt);

my $elt = get_recursive_elt($data);
$elt->paste($LAST_CHILD_STRING, $data_elt);



my $twig = new XML::Twig(twig_roots => {variable_assignment => 1});
#my $twig = new XML::Twig(twig_roots => {assign_variable => 1});
$twig->set_pretty_print($INDENTED_STRING);

$twig->parsefile($xmlfile);
my $root = $twig->root;
my @var_assigns = $root->children;


# loop through and remove an existing entry for $var_name
foreach my $var_assign ($root->children) {
    foreach my $child ($var_assign->children) {
	my $value_type = $child->gi;

	if($value_type eq $TAG_STRING) {	    
	    my $name = $child->text;

	    if($name eq $var_name) {		
		$var_assign->cut;
	    }   
	}
    }
}

$var_elt->paste($FIRST_CHILD_STRING, $root);

$crap = $twig->sprint;

open(XMLFILE, ">$xmlfile") or die "can't open xmlfile = $xmlfile: $!";

print XMLFILE $twig->sprint;
close XMLFILE;


sub create_XML {    
    my $xmlfile = shift;
    
    open(XMLFILE, ">$xmlfile") or die "can't open xmlfile = $xmlfile: $!";
    
    print XMLFILE "<\?xml version=\"1.0\"\?>\n<saved_variable_assignments></saved_variable_assignments>";
    
    close XMLFILE;
}


sub get_recursive_elt {
    my ($data) = @_;

    my $elt = XML::Twig::Elt->new();

    if($data->getDataType() == $MonkeyScript::M_INTEGER) {
		my $value = XML::Twig::Elt->new($VALUE_STRING);
		$elt->set_gi($INTEGER_STRING);
		$value->set_text($data->getInteger);
		$value->paste($LAST_CHILD_STRING, $elt);
    } elsif($data->getDataType() == $MonkeyScript::M_BOOLEAN) {
		my $value = XML::Twig::Elt->new($VALUE_STRING);
		$elt->set_gi($BOOLEAN_STRING);
		$value->set_text($data->getBool);
		$value->paste($LAST_CHILD_STRING, $elt);
    } elsif($data->getDataType() == $MonkeyScript::M_FLOAT) {
		my $value = XML::Twig::Elt->new($VALUE_STRING);
		$elt->set_gi($FLOAT_STRING);
		$value->set_text($data->getFloat);
		$value->paste($LAST_CHILD_STRING, $elt);
    } elsif($data->getDataType() == $MonkeyScript::M_STRING) {
		my $value = XML::Twig::Elt->new($VALUE_STRING);
		$elt->set_gi($STRING_STRING);
		$value->set_text($data->getString);
		$value->paste($LAST_CHILD_STRING, $elt);
    } elsif($data->getDataType() == $MonkeyScript::M_DICTIONARY) {
		my $dict_element = XML::Twig::Elt->new($DICT_ELEMENT_STRING);

		$elt->set_gi($DICTIONARY_STRING);
	
		my $i = 0;
	
		while($i < $data->getMaxElements()) {
			my $key = $data->getKey($i);
			
			unless($key->isUndefined()) {

				my $keyString = MonkeyScript::const_char_string($key->toString());
				my $keyData = new MonkeyScript::mData();
				$keyData->setString($keyString, length($keyString)+1);
				$keyData->DISOWN;
				
				if($data->hasKey($keyData)) {
					my $subdata = $data->getElement($keyData);

					unless($subdata->isUndefined()) {
						my $dict_elt = XML::Twig::Elt->new($DICT_ELEMENT_STRING);



						my $key_elt = XML::Twig::Elt->new($KEY_STRING);
						$key_elt->set_text($keyData->getString());
						$key_elt->paste($FIRST_CHILD_STRING, $dict_elt);

						$sub_elt = get_recursive_elt($subdata);
						$sub_elt->paste($LAST_CHILD_STRING, $dict_elt);

						$dict_elt->paste($LAST_CHILD_STRING, $elt);
					}
				}
			}
			$i++;
		}
	
    } elsif($data->getDataType() == $MonkeyScript::M_LIST) {

		my $typename = $LIST_STRING;
		$elt->set_gi($typename);

	
		my $i = 0;
		while($i < $data->getMaxElements()) {
			my $subdata = $data->getElement($i);

			unless($subdata->isUndefined()) {
				my $list_element = XML::Twig::Elt->new($typename . $ELEMENT_SUFFIX_STRING);
				my $index_element = XML::Twig::Elt->new($INDEX_STRING);
		
				$sub_elt = get_recursive_elt($subdata);
				
				$index_element->set_text($i);
				$index_element->paste($FIRST_CHILD_STRING, $list_element);
				$sub_elt->paste($LAST_CHILD_STRING, $list_element);
				$list_element->paste($LAST_CHILD_STRING, $elt);
			}	    
			$i++;
		}
    } else {
		die "bad data type: $data->getDataType()";
    }
    return $elt;
}

