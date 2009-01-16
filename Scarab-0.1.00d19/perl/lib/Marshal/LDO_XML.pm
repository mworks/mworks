#
# Copyright (C) 1998 The Casbah Project
# Marshal::LDO_XML.pm is free software; you can redistribute it and/or
# modify it under the same terms as Perl itself.
#
# $Id: LDO_XML.pm,v 1.2 2000/03/02 23:53:52 kmacleod Exp $
#

=head1 NAME

Marshal::LDO_XML -- marshal Perl objects in LDO XML format

=head1 SYNOPSIS

  use Marshal::LDO_XML;

  $m = Marshal::LDO_XML->new( stream => $stream );

  $m->freeze(OBJECT);
  (OBJECT) = $m->thaw;

=head1 DESCRIPTION

see <http://152.2.205.21/casbah.270> and
<http://bitsko.slc.ut.us/~ken/casbah/bin-serialization/>

=cut

package Marshal::LDO_XML;

use strict;
use XML::Parser;
use MIME::Base64;

###
### Constants
###

sub MAJOR ()      { 0 }
sub MINOR ()      { 0 }

use vars qw{%char_entities $list_close $list_close_ref $dict_close $dict_close_ref};

%char_entities = (
    '&' => '&amp;',
    '<' => '&lt;',
    '>' => '&gt;',
    '"' => '&quot;',
);

# these objects act as markers, as objects they can't be mistaken for
# any other object on the system.
$list_close_ref = 'Marshal::LDO_XML::List::Close';
$list_close = bless {}, $list_close_ref;
$dict_close_ref = 'Marshal::LDO_XML::Dict::Close';
$dict_close = bless {}, $dict_close_ref;

# FIXME actually, it really is possible to marshal objects in those
# classes, but Perl doesn't support comparing object pointers
# efficiently as it is.  So we just compare the class (package) names
# and hope that is enough.  The second step, after comparing the class
# name, would have been to compare the result of evaluating the object
# in a double-quoted string, for example:
#
#     if (ref($item) eq $list_close_ref
#         && "$item" eq "$list_close") { ... }

sub new {
    my $type = shift;

    my $self = bless { @_ }, $type;

    return $self;
}

sub stream {
    my $self = shift;

    if ($#_ == -1) {
	return $self->{'stream'};
    } else {
	$self->{'stream'} = shift;

	return $self;
    }
}

sub flush {
    my $self = shift;

    $self->{'stream'}->flush;

    return $self;
}

sub _log_info {
    my $self = shift;
    my $info = shift;

    warn $info . "\n"
	if $self->{'debug'};
    return $self;
}

sub _die {
    my $self = shift;
    my $error = shift;

    die $error . "\n";
}

######################################################################
###
### Encoding
###

sub freeze {
    my $self = shift;

    $self->{'stream'}->print('<?xml version="1.0"?>');
    my $item;
    foreach $item (@_) {
	$self->_encode_object($item);
    }

    $self->{'stream'}->print("\n--seperator~\n");
	
    return $self;
}

sub _encode_object {
    my $self = shift; my $object = shift;

    my $ref = ref($object);
    if (!$ref) {
	$self->_encode_scalar($object, '');
    } else {
	$ref = "$object";
	if ($ref =~ /^((.*)=)?SCALAR\(/) {
	    $self->_encode_scalar($$object, $2);
	} elsif ($ref =~ /^((.*)=)?ARRAY\(/) {
	    $self->_encode_list($object, $2);
	} elsif ($ref =~ /^((.*)=)?HASH\(/) {
	    $self->_encode_dictionary($object, $2);
	} else {
	    $self->_die("can't freeze \`$object'");
	}
    }
}

sub _encode_scalar {
    my $self = shift; my $scalar = shift; my $type = shift;

    my $attributes = "";
    if (defined $type && $type ne '') {
	$attributes = " type='$type'";
	$scalar = $$scalar;
    }

    if ($scalar =~ /[\x00-\x1f\x7f-\xff]/) {
	$scalar = encode_base64($scalar);
	$attributes .= " encoding='base64'";
    } else {
	$scalar =~ s/([&<>\"])/$char_entities{$1}/ge;
    }
    $self->{'stream'}->print('<atom' . $attributes . '>' . $scalar . '</atom>');
}

sub _encode_list {
    my $self = shift; my $array = shift; my $type = shift;

    my $attributes = "";
    if (defined $type && $type ne '') {
	$attributes = " type='$type'";
    }

    $self->{'stream'}->print('<list' . $attributes . '>');

    my $arg;
    foreach $arg (@$array) {
	$self->_encode_object($arg);
    }

    $self->{'stream'}->print('</list>');
}

sub _encode_dictionary {
    my $self = shift; my $hash = shift; my $type = shift;

    my $attributes = "";
    # FIXME the Perl package name is very likely not a good Casbah class
    if (defined $type && $type ne '') {
	$attributes = " type='$type'";
    }

    $self->{'stream'}->print('<dictionary' . $attributes . '>');

    my ($key, $value);
    while (($key, $value) = each %$hash) {
	$self->_encode_object($key);
	$self->_encode_object($value);
    }

    $self->{'stream'}->print('</dictionary>');
}

######################################################################
###
### Decoding
###

# `thaw' decodes one object and returns, if the caller expects more
# than one object, they need to call thaw again on the stream.

sub thaw {
    my $self = shift;

    #return (XML::Parser->new(Style => ref($self))->parsefile($self->{'stream'}));

    my $string = "";
    my $line;
    while (($line = $self->{'stream'}->getline)
	   && $line !~ /^--seperator~\r?\n$/s) {
	$string .= $line;
    }
    if ($string eq '') {
	die("zero length read from stream (socket close by remote host?): $!");
    }
    return (XML::Parser->new(Style => ref($self))->parsestring($string));
}

sub init {
    my $self = shift;

    $self->{'parse_value'} = [];
    $self->{'parse_type'} = undef;
    $self->{'parse_value_stack'} = [];
    $self->{'parse_type_stack'} = [];
    $self->{'parse_keep_text'} = 0;
    $self->{'parse_base64'} = 0;
}

sub start {
    my ($self, $tag, %attrs) = @_;

    push(@{ $self->{'parse_value_stack'} }, $self->{'parse_value'});
    push(@{ $self->{'parse_type_stack'} }, $self->{'parse_type'});

    $self->{'parse_value'} = undef;
    $self->{'parse_type'} = $attrs{'type'};

    if ($tag eq 'atom') {
	$self->{'parse_keep_text'} = 1;
	$self->{'parse_value'} = "";
	if (defined $attrs{'encoding'} && $attrs{'encoding'} eq 'base64') {
	    $self->{'parse_base64'} = 1;
	}
    } elsif ($tag eq 'list' || $tag eq 'dictionary') {
	$self->{'parse_value'} = [];
    } elsif ($tag eq 'ref') {
	die("undefined reference \`$attrs{'ref'}'")
	    if (!exists($self->{'references'}{$attrs{'ref'}}));
	$self->{'parse_saved_item'} = $self->{'references'}{$attrs{'ref'}};
    }
}

sub end {
    my $self = shift; my $tag = shift;
    my $item;

    if ($tag eq 'atom') {
	$item = $self->{'parse_value'};
	$self->{'parse_keep_text'} = 0;
	if ($self->{'parse_base64'}) {
	    $self->{'parse_base64'} = 0;
	    $item = decode_base64($item);
	}
    } elsif ($tag eq 'list') {
	$item = $self->{'parse_value'};
    } elsif ($tag eq 'dictionary') {
	$item = { @{$self->{'parse_value'}} };
    } elsif ($tag eq 'ref') {
	$item = $self->{'parse_saved_item'};
    }

    if (defined $self->{'parse_type'} && $self->{'parse_type'} ne '') {
	if (ref($item)) {
	    bless $item, $self->{'parse_type'};
	} else {
	    $item = bless \$item, $self->{'parse_type'};
	}
    }
    $self->{'parse_value'} = pop(@{$self->{'parse_value_stack'}});
    $self->{'parse_type'} = pop(@{$self->{'parse_type_stack'}});
    push (@{$self->{'parse_value'}}, $item);
}

sub char {
    my $self = shift; my $text = shift;

    if ($self->{'parse_keep_text'}) {
	$self->{'parse_value'} .= $text;
    }
}

sub proc {
}

sub final {
    my $self = shift;

    if ($#{$self->{'parse_value'}} > -1) {
	return $self->{'parse_value'}[0];
    } else {
	return undef;
    }
}

1;
