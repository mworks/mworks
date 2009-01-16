#
# Copyright (C) 1998, 1999, 2000 The Casbah Project
# RPC::Scarab::Connection.pm is free software; you can redistribute it
# and/or modify it under the same terms as Perl itself.
#
# $Id: Connection.pm,v 1.1 2000/03/04 19:38:36 kmacleod Exp $
#

package RPC::Scarab::Connection;
use strict;

use Marshal::LDO_Binary;

sub new {
    my $type = shift;
    my $self = bless { @_ }, $type;

    if (!defined($self->{'error_handler'})) {
	$self->{'error_handler'} = $self;
    }

    if (defined ($self->{'marshal'})) {
	$self->{'marshal'}->stream($self->{'stream'});
    }

    return $self;
}

sub is_valid {
    my $self = shift;

    return !defined($self->{'error'});
}

sub log_error {
    my $self = shift;
    my $error = shift;

    $self->{'error'} = $error;
    warn $error . "\n";
    return $self;
}

sub log_info {
    my $self = shift;
    my $info = shift;

    if (defined $self->{'debug'}) {
	if (ref $self->{'debug'}) {
	    $self->{'debug'}->print ($info . "\n");
	} else {
	    warn $info . "\n";
	}
    }
    return $self;
}

sub close {
    my $self = shift;

    # we effectively delete ourselves out of existence
    $self->log_info("closing connection");
    my $stream = $self->{'stream'};
    if (defined $self->{'manager'}) {
	$self->{'manager'}->_delete_connection($self, $stream);
    }
    $stream->close;
}

# Read one request and process it
sub accept {
    my $self = shift;
    my $error = 0;

    if (!defined $self->{'marshal'}) {
	my $stream = RPC::Scarab::BufferedFile->new(stream => $self->{'stream'});
	my $head;
	eval { $head = $stream->head(4) };
	if ($@) {
	    # btw, `zero length read' is really an EOF than an error
	    $self->log_info($@);
	    $self->{'error'} = $@;
	    $self->close;
	    return;
	}

	if (substr($head, 0, 4) eq "\x89CBF") {
	    $self->log_info("accepting binary connection");
	    $self->{'marshal'} = Marshal::LDO_Binary->new(stream => $stream);
	} else {
	    if (!defined $Marshal::LDO_XML::) {
		eval {
		    require Marshal::LDO_XML;
		    import Marshal::LDO_XML;
		};
		if ($@) {
		    # FIXME send hardcoded XML error fragment
		    # btw, `zero length read' is really an EOF than an error
		    $self->log_info($@);
		    $self->{'error'} = $@;
		    $self->close;
		    return;
		}
	    }
	    $self->log_info("accepting xml connection");
	    $self->{'marshal'} = Marshal::LDO_XML->new(stream => $stream);
	}
    }
    $self->log_info("reading message");
    my $ref;
    eval { $ref = $self->{'marshal'}->thaw };
    if ($@) {
	# FIXME send hardcoded XML error fragment
	# btw, `zero length read' is really an EOF than an error
	$self->log_info($@);
	$self->{'error'} = $@;
	$self->close;
	return;
    }

    if (ref($ref) ne 'HASH') {
	$self->log_error("Error while reading client request: Expected hash");
	$error = 1;
	goto return_error;
    }

    if (!defined($ref->{'object'})
	|| !defined($ref->{'method'})
	|| !defined($ref->{'args'})) {
	$self->log_error("Error while reading client request: missing object reference, method, or args");
	$error = 1;
	goto return_error;
    }

    if (!defined($self->{'manager'}->{$ref->{'object'}})) {
	$self->log_error("No such object registered: $ref->{'object'}");
	$error = 1;
	goto return_error;
    }

    my $method = $ref->{'method'};
    my (@retval, $result);

  return_error:
    if ($error) {
	$result = { error => $self->{'error'} };
    } else {
	eval { @retval = $self->{'manager'}->{$ref->{'object'}}->$method(@{ $ref->{'args'} }) };
	if ($@) {
	    $self->log_error($@);
	    $result = { error => $@ , result => [ @retval ] };
	} else {
	    $result = { result => [ @retval ] };
	}
    }

    $self->log_info("writing reply");
    $self->{'marshal'}->freeze($result);
    $self->{'marshal'}->flush;
}

sub call {
    my $self = shift;
    my $object = shift;
    my $method = shift;

    # Write method call
    $self->{'marshal'}->freeze({ object => $object, method => $method, args => [ @_ ] });
    $self->{'marshal'}->flush;

    my $ref = $self->{'marshal'}->thaw;
    if (ref($ref) ne 'HASH') {
	$self->log_error("Error while reading server reply: Expected dictionary");
	return undef;
    }

    if (defined($ref->{'error'})) {
	$self->{'error'} = $ref->{'error'};
	die $self->{'error'} . "\n";
    } elsif (!defined($ref->{'result'})) {
	$self->log_error("Error while reading server reply: Expected `result' parameter");
	die $self->{'error'} . "\n";
    } elsif (ref($ref->{'result'}) ne 'ARRAY') {
	$self->log_error("Error while reading server reply: Expected `result' to be an array");
	die $self->{'error'} . "\n";
    } else {
	delete $self->{'error'};
    }

    return $ref->{'result'}->[0];
}

package RPC::Scarab::BufferedFile;

sub new {
    my $type = shift;

    return bless { @_ }, $type;
}

sub head {
    my $self = shift;
    my $length = shift;

    my $stream = $self->{'stream'};
    my $string = '';
    while ($length > 0) {
	my $retval = $stream->read($string, $length, length($string));
	die("error reading $length bytes from stream: $!")
	    if (!defined $retval || $retval < 0);
	die("zero length read from stream (socket close by remote host?): $!")
	    if ($retval == 0);
	$length -= $retval;
    }

    if ($string =~ /\n/s) {
	die "RPC::Scarab::BufferedStream: doesn't handle newlines in the header!";
    }

    $self->{'head'} = $string;

    return $string;
}

sub getline {
    my $self = shift;

    if (defined $self->{'head'}) {
	my $string = $self->{'head'} . $self->{'stream'}->getline;
	delete $self->{'head'};
	return $string;
    } else {
	return $self->{'stream'}->getline;
    }
}

sub flush {
    my $self = shift;

    return $self->{'stream'}->flush;
}

# this sub is really wierd because IO::Handle's `read' relies
# on Perl's argument-by-reference on subs
sub read {
    my $self = shift;

    if (defined $self->{'head'}) {
	if ($_[1] < length($self->{'head'})) {
	    substr($_[0], $_[2]) = substr($self->{'head'}, 0, $_[1]);
	    substr($self->{'head'}, 0, $_[1]) = "";
	    return $_[1];
	} elsif ($_[1] == length($self->{'head'})) {
	    substr($_[0], $_[2]) = $self->{'head'};
	    delete $self->{'head'};
	    return $_[1];
	} else {
	    my $string = $self->{'head'};
	    delete $self->{'head'};
	    my $length = $_[1] - length($string);
	    my $read_length = $self->{'stream'}->read($string, $length, length($string));
	    substr($_[0], $_[2]) = $string;
	    return $length + $read_length;
	}
    } else {
	return $self->{'stream'}->read($_[0], $_[1], $_[2]);
    }
}

sub print {
    my $self = shift;

    return $self->{'stream'}->print(@_);
}

1;
