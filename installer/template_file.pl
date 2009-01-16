#!/usr/bin/perl
use FileHandle;

$template_filename = $ARGV[0];
$template_string =  $ARGV[1];
$replacement_string = $ARGV[2];
$out_filename = $ARGV[3];



my $file = new FileHandle("$template_filename");
@lines = <$file>;
my $outstring = join('', @lines);

$outstring =~ s{$template_string}{$replacement_string}xgs;

$outfile = new FileHandle("> $out_filename");
print "Writing to file $out_filename\n";
print $outfile $outstring;

$file->close();
$outfile->close();

