#
# Copyright (C) 1998, 1999, 2000 The Casbah Project
# RPC::Scarab::Proxy.pm is free software; you can redistribute it
# and/or modify it under the same terms as Perl itself.
#
# $Id: Proxy.pm,v 1.1 2000/03/04 19:38:36 kmacleod Exp $
#

package RPC::Scarab::Proxy;

use vars qw{$AUTOLOAD};

sub new {
    my $type = shift;
    return bless { @_ }, $type;
}

sub _connection_for_proxy {
    my $self = shift;
    return $self->{'connection'};
}

sub AUTOLOAD {
    my $self = shift;

    my $method = $AUTOLOAD;
    $method =~ s/.*:://;
    return if $method eq 'DESTROY';

    my $connection = $self->{'connection'};
    # FIXME die
    return undef if !$connection->is_valid;

    return $connection->call($self->{'object'}, $method, @_);
}

1;
