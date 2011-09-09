#
# Copyright (C) 1998 The Casbah Project
# Marshal::LDO_Binary.pm is free software; you can redistribute it and/or
# modify it under the same terms as Perl itself.
#
# $Id: LDO_Binary.pm,v 1.1 1998/10/25 17:58:25 kmacleod Exp $
#

=head1 NAME

Marshal::LDO_Binary -- marshal Perl objects in LDO binary format

=head1 SYNOPSIS

  use Marshal::LDO_Binary;

  $m = Marshal::LDO_Binary->new( stream => $stream );

  $m->freeze(OBJECTS);
  (OBJECTS) = $m->thaw;

=head1 DESCRIPTION

see <http://152.2.205.21/casbah.270> and
<http://bitsko.slc.ut.us/~ken/casbah/bin-serialization/>

=cut

use strict;

package Marshal::LDO_Binary;

###
### Constants
###

sub MAGIC ()      { "\x89CBF" }
sub MAJOR ()      { 0 }
sub MINOR ()      { 0 }

sub VERSION ()      { "\x01" }

sub INTEGER_N ()    { "\x02" }
sub INTEGER_P ()    { "\x03" }
sub FLOAT_NN ()     { "\x04" }
sub FLOAT_NP ()     { "\x05" }
sub FLOAT_PN ()     { "\x06" }
sub FLOAT_PP ()     { "\x07" }
sub FLOAT_INF ()    { "\x08" }
sub FLOAT_NAN ()    { "\x09" }
sub OPAQUE ()       { "\x0A" }
sub NULL ()         { "\x0B" }
sub LIST_OPEN ()    { "\x0C" }
sub LIST_CLOSE ()   { "\x0D" }
sub DICTIONARY_OPEN ()  { "\x0E" }
sub DICTIONARY_CLOSE () { "\x0F" }

sub DEFINE_REFERENCE () { "\x10" }
sub REFERENCE ()    { "\x11" }

sub ATTRIBUTES ()   { "\x12" }

sub CB_CLASS ()   { '__cb_class__' }

use vars qw{$list_close $list_close_ref $dict_close $dict_close_ref};

# these objects act as markers, as objects they can't be mistaken for
# any other object on the system.
$list_close_ref = 'Marshal::LDO_Binary::List::Close';
$list_close = bless {}, $list_close_ref;
$dict_close_ref = 'Marshal::LDO_Binary::Dict::Close';
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

    $self->{'written_stream_header'} = 0;
    $self->{'read_stream_header'} = 0;

    return $self;
}

sub stream {
    my $self = shift;

    if ($#_ == -1) {
	return $self->{'stream'};
    } else {
	$self->{'stream'} = shift;

	$self->{'written_stream_header'} = 0;
	$self->{'read_stream_header'} = 0;

	return $self;
    }
}

sub flush {
    my $self = shift;

    $self->{'stream'}->flush;

    return $self;
}

sub _read {
    my $self = shift;
    my $length = shift;

    my $stream = $self->{'stream'};
    my $string = '';
    while ($length > 0) {
	my $retval = $stream->read($string, $length, length($string));
	$self->_die("error reading $length bytes from stream: $!")
	    if (!defined $retval || $retval < 0);
	$self->_die("zero length read from stream (socket close by remote host?): $!")
	    if ($retval == 0);
	$length -= $retval;
    }

    return $string;
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

    $self->_encode_stream_header
	if (!$self->{'written_stream_header'});

    my $item;
    foreach $item (@_) {
	$self->_encode_object($item);
    }

    return $self;
}

sub _encode_stream_header {
    my $self = shift;

    $self->{'written_stream_header'} = 1;

    $self->_encode_magic;
    $self->_encode_version;
}


sub _encode_magic {
    my $self = shift;

    $self->{'stream'}->print(MAGIC);
}

sub _encode_version {
    my $self = shift;

    $self->{'stream'}->print(VERSION);
    $self->_encode_ber(MAJOR);
    $self->_encode_ber(MINOR);
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

    # FIXME the Perl package name is very likely not a good Casbah class
    if ($type ne '') {
	$self->{'stream'}->print(ATTRIBUTES);
	$self->{'stream'}->print(DICTIONARY_OPEN);
	$self->_encode_object(CB_CLASS);
	$self->_encode_object($type);
	$self->{'stream'}->print(DICTIONARY_CLOSE);
    }

    # always write scalars as a string.  `Storable.xs' refers to
    # `Dump.pm' for how to find out if there's an int or float part to
    # a scalar.  The `perlfaq4' route seems a waste.
    $self->{'stream'}->print(OPAQUE);
    $self->_encode_ber(length($scalar));
    $self->{'stream'}->print($scalar);
}

sub _encode_list {
    my $self = shift; my $array = shift; my $type = shift;

    # FIXME the Perl package name is very likely not a good Casbah class
    if ($type ne '') {
	$self->{'stream'}->print(ATTRIBUTES);
	$self->{'stream'}->print(DICTIONARY_OPEN);
	$self->_encode_object(CB_CLASS);
	$self->_encode_object($type);
	$self->{'stream'}->print(DICTIONARY_CLOSE);
    }

    $self->{'stream'}->print(LIST_OPEN);

    my $arg;
    foreach $arg (@$array) {
	$self->_encode_object($arg);
    }

    $self->{'stream'}->print(LIST_CLOSE);
}

sub _encode_dictionary {
    my $self = shift; my $hash = shift; my $type = shift;

    # FIXME the Perl package name is very likely not a good Casbah class
    if ($type ne '') {
	$self->{'stream'}->print(ATTRIBUTES);
	$self->{'stream'}->print(DICTIONARY_OPEN);
	$self->_encode_object(CB_CLASS);
	$self->_encode_object($type);
	$self->{'stream'}->print(DICTIONARY_CLOSE);
    }

    $self->{'stream'}->print(DICTIONARY_OPEN);

    my ($key, $value);
    while (($key, $value) = each %$hash) {
	$self->_encode_object($key);
	$self->_encode_object($value);
    }

    $self->{'stream'}->print(DICTIONARY_CLOSE);
}

sub _encode_ber {
    my $self = shift; my $value = shift;

    $self->{'stream'}->print(pack('w', $value));
}

######################################################################
###
### Decoding
###


# `thaw' decodes one object and returns, if the caller expects more
# than one object, they need to call thaw again on the stream.

sub thaw {
    my $self = shift;

    $self->_decode_stream_header
	if (!$self->{'read_stream_header'});

    return ($self->_decode_object);
}

sub _decode_stream_header {
    my $self = shift;

    $self->{'read_stream_header'} = 1;

    $self->_decode_magic;
    $self->_decode_version;
}

sub _decode_magic {
    my $self = shift;

    $self->{'magic'} = $self->_read(length(MAGIC));
}

sub _decode_version {
    my $self = shift;

    $self->_die("expected version")
	if ($self->_read(1) ne VERSION);
    $self->{'major'} = $self->_decode_ber;
    $self->{'minor'} = $self->_decode_ber;
    $self->_die("incompatible version")
	if ($self->{'major'} != 0 && $self->{'minor'} != 0);
}

sub _decode_object {
    my $self = shift;

    my $item_type = $self->_read(1);

    my $id = undef;
    if ($item_type eq DEFINE_REFERENCE) {
	$id = $self->_decode_ber;
	$item_type = $self->_read(1);
    }

    # most attributes are not used in Perl and are silently discarded,
    # class is used below to bless the object
    my ($attributes, $class);
    if ($item_type eq ATTRIBUTES) {
	my $attributes = $self->_decode_object;
	die "attributes is not a dictionary"
	    if (ref($attributes) ne 'HASH');
	$class = $attributes->{CB_CLASS()};
	$item_type = $self->_read(1);
    }

    my $item;
    if ($item_type eq INTEGER_N) {
	$item = -$self->_decode_ber;
    } elsif ($item_type eq INTEGER_P) {
	$item = $self->_decode_ber;
    } elsif ($item_type eq FLOAT_NN) {
	my $mantissa = -$self->_decode_ber;
	my $exponent = -$self->_decode_ber;
	$item = "-${mantissa}E-$exponent";
    } elsif ($item_type eq FLOAT_NP) {
	my $mantissa = -$self->_decode_ber;
	my $exponent = $self->_decode_ber;
	$item = "${mantissa}E$exponent";
    } elsif ($item_type eq FLOAT_PN) {
	my $mantissa = $self->_decode_ber;
	my $exponent = -$self->_decode_ber;
	$item = "${mantissa}E$exponent";
    } elsif ($item_type eq FLOAT_PP) {
	my $mantissa = $self->_decode_ber;
	my $exponent = $self->_decode_ber;
	$item = "${mantissa}E$exponent";
    } elsif ($item_type eq FLOAT_INF) {
	$self->_die("FLOAT_INF not supported in Perl\n");
    } elsif ($item_type eq FLOAT_NAN) {
	$self->_die("FLOAT_NAN not supported in Perl\n");
    } elsif ($item_type eq OPAQUE) {
	my $length = $self->_decode_ber;
	# FIXME a sanity check, or ulimit, would probably be nice here
	$item = $self->_read($length);
    } elsif ($item_type eq REFERENCE) {
	my $reference = $self->_decode_ber;
	$self->_die("undefined reference \`$reference'")
	    if ($reference > $#{ $self->{'references'} });
	$item = $self->{'references'}[$reference];
    } elsif ($item_type eq NULL) {
	$item = undef;
    } elsif ($item_type eq LIST_OPEN) {
	$item = $self->_decode_list;
    } elsif ($item_type eq LIST_CLOSE) {
        $item = $list_close;
    } elsif ($item_type eq DICTIONARY_OPEN) {
	$item = $self->_decode_dictionary;
    } elsif ($item_type eq DICTIONARY_CLOSE) {
	$item = $dict_close;
    } else {
	my $type_str = $item_type;
	$type_str =~ s/([^\40-\176]+)/"\\0x" . unpack("H*", $1)/ge;
	$self->_die("unknown item type \`$type_str'");
    }

    if (defined $id) {
	$self->{'references'}[$id] = $item;
    }

    # FIXME the Perl package name is very likely not a good Casbah class
    # FIXME must be a reference to take a class
    if (defined $class) {
	if (ref($item)) {
	    bless $item, $class;
	} else {
	    $item = bless \$item, $class;
	}
    }

    return ($item);
}

sub _decode_list {
    my $self = shift;

    my @list;
    my $item = $self->_decode_object;
    while (ref($item) ne $list_close_ref && ref($item) ne $dict_close_ref) {
	push @list, $item;
	$item = $self->_decode_object;
    }
    $self->_die("received DICTIONARY-CLOSE as terminator for a list")
	if (ref($item) eq $dict_close_ref);

    return ([ @list ]);
}

sub _decode_dictionary {
    my $self = shift;

    my @list;
    my $item = $self->_decode_object;
    while (ref($item) ne $list_close_ref && ref($item) ne $dict_close_ref) {
	$self->_die("non-scalar dictionary keys not supported in Perl: $item")
	    if (ref($item));
	push @list, $item;
	$item = $self->_decode_object;

	$self->_die("received LIST-CLOSE as terminator for a dictionary")
	    if (ref($item) eq $list_close_ref);
	$self->_die("odd number of items thawed for a dictionary")
	    if (ref($item) eq $dict_close_ref);
	push @list, $item;
	$item = $self->_decode_object;
    }
    $self->_die("received LIST-CLOSE as terminator for a dictionary")
	if (ref($item) eq $list_close_ref);

    $item = { @list };

    return ($item);
}

sub _decode_ber {
    my $self = shift;

    my $ber = '';
    my $char;
    do {
	$ber .= $char = $self->_read(1);
    } while (ord($char) > 127);
	
    return (unpack('w', $ber));
}

1;
