#
# Copyright (C) 1998, 1999, 2000 The Casbah Project
# RPC::Scarab::Manager.pm is free software; you can redistribute it
# and/or modify it under the same terms as Perl itself.
#
# $Id: Manager.pm,v 1.1 2000/03/04 19:38:36 kmacleod Exp $
#

use strict;

package RPC::Scarab::Manager;

use IO::Select;
use IO::Socket;
use RPC::Scarab::Connection;
use RPC::Scarab::Proxy;
use Marshal::LDO_Binary;

use vars qw{$default_connection $VERSION};

$VERSION = '0.00';

$default_connection = undef;

sub new {
    my $type = shift;
    my $self = { @_ };

    bless $self, $type;

    delete $self->{'error'};

    $self->{'error_handler'} = $self
	if (!defined($self->{'error_handler'}));

    if (!defined ($default_connection)) {
	if (defined $self->{'receive_port_port'}) {
	    $default_connection = IO::Socket::INET->new(Listen   => 5,
							LocalPort => $self->{'receive_port_port'},
							Proto    => 'tcp');
	} else {
	    $default_connection = IO::Socket::INET->new(Listen   => 5,
							Proto    => 'tcp');
	}
	die "can't create socket: $!\n"
	    if !defined $default_connection;
    }

    $self->{'receive_port'} = $default_connection;

    return $self;
}

sub new_rootproxy {
    my $type = shift;
    my %args = @_;

    $args{'marshal'} = 'binary'
	if !defined $args{'marshal'};

    if (!ref($args{'marshal'})) {
	if ($args{'marshal'} eq 'xml') {
	    if (!defined $Marshal::LDO_XML::) {
		require Marshal::LDO_XML;
		import Marshal::LDO_XML;
	    }
	    $args{'marshal'} = Marshal::LDO_XML->new();
	} else {
	    $args{'marshal'} = Marshal::LDO_Binary->new();
	}
    }

    my $stream = IO::Socket::INET->new(%{ $args{'connect_to'} })
	or die "can't create socket: $!\n";

    my $connection = RPC::Scarab::Connection->new(%args, stream => $stream);

    return RPC::Scarab::Proxy->new (connection => $connection, object => 'root');
}

sub receive_port {
    my $self = shift;
    return ($self->{'receive_port'});
}

sub run_loop {
    my $self = shift;

    $self->{'mode'} = 'internal';

    my $sel = $self->{'selector'} = IO::Select->new;
    $self->_add_connection ($self, $self->{'receive_port'});

    my ($read, $write, $excp);
    while (($read, $write, $excp) = IO::Select->select($sel, undef, $sel)) {
	my $stream;
	foreach $stream (@$read) {
	    $self->{'connections'}{$stream}->accept;
	}
	if ($#$write != -1 || $#$excp != -1) {
	    die "ack! we have writables and exceptions in run_loop!\n";
	}
    }
}

sub tk_fileevent {
    my $self = shift;
    my $mw = shift;

    $self->{'mode'} = 'tk';
    $self->{'mw'} = $mw;
    $self->_add_connection ($self, $self->{'receive_port'});
}

sub _add_connection {
    my $self = shift;
    my $connection = shift;
    my $handle = shift;

    if ($self->{'mode'} eq 'internal') {
	$self->{'connections'}{"$handle"} = $connection;
	$self->{'selector'}->add($handle);
    } elsif ($self->{'mode'} eq 'tk') {
	$self->{'mw'}->fileevent($handle,
				 'readable' => sub { $connection->accept });
    } else {
	die "_add_connection: not initialized with \`run_loop' or \`tk_fileevent'\n";
    }
}

sub _delete_connection {
    my $self = shift;
    my $connection = shift;
    my $handle = shift;

    if ($self->{'mode'} eq 'internal') {
	delete $self->{'connections'}{"$handle"};
	$self->{'selector'}->remove($handle);
    } elsif ($self->{'mode'} eq 'tk') {
	$self->{'mw'}->fileevent($handle,
				 'readable' => '');
    } else {
	die "_delete_connection: not initialized with \`run_loop' or \`tk_fileevent'\n";
    }
}

sub accept {
    my $self = shift;

    $self->log_info("accepting request");
    my $sock = $self->{'receive_port'}->accept;
    my @args;
    if (defined $self->{'debug'}) {
	@args = (debug => $self->{'debug'});
    }
    my $new_connection = RPC::Scarab::Connection->new(stream => $sock,
						      fd => $sock->fileno,
						      manager => $self,
						      @args);
    $self->_add_connection($new_connection, $sock);
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
	    $self->{'debug'}->print($info . "\n");
	} else {
	    warn $info . "\n";
	}
    }
    return $self;
}

sub close {
    my $self = shift;

    die "close not implemented on default Connection\n";
}

sub root {
    my $self = shift;

    if (@_ == -1) {
	return $self->{'root'};
    } else {
	$self->{'root'} = $_[0];
	return $self;
    }
}

1;
