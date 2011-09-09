#!/usr/bin/perl
use XML::Schematron::XPath;


my $schema_file = $ARGV[0];
my $xml_file    = $ARGV[1];

die "Usage: perl schematron.pl schemafile XMLfile.\n"
    unless defined $schema_file and defined $xml_file;

print $schema_file . ", " . $xml_file . "\n";

my $tron = XML::Schematron::XPath->new(schema => $schema_file);

if(!defined($tron)){
	print "no schematron created\n";
}

my $ret = $tron->verify($xml_file);

if(defined($ret)){
	print $ret . "\n";
} else {
	print "no answer\n";
}