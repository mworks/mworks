#!/usr/bin/env python3

import argparse
import socket
import threading

import zmq


remote_address = None
remote_outgoing_port = 19989

local_address = '*'
local_outgoing_port = None


def process_args():
    global remote_address, remote_outgoing_port
    global local_address, local_outgoing_port

    ap = argparse.ArgumentParser()
    ap.add_argument('remote_address',
                    help = 'Listening address of upstream MWServer instance')
    ap.add_argument('--remote-port',
                    default = remote_outgoing_port,
                    help = ('Listening port of upstream MWServer instance '
                            '(default: %d)' % remote_outgoing_port))
    ap.add_argument('local_address',
                    nargs = '?',
                    default = local_address,
                    help = ('Listening address of proxy.  If omitted, bind to '
                            'all available interfaces.'))
    ap.add_argument('--local-port',
                    help = ('Listening port of proxy.  If omitted, use '
                            'REMOTE_PORT.'))

    args = ap.parse_args()
    remote_address = args.remote_address
    remote_outgoing_port = int(args.remote_port)
    local_address = args.local_address
    local_outgoing_port = int(args.local_port or remote_outgoing_port)

    if local_address != '*':
        # Resolve local_address to numeric form, as required by zmq_bind
        local_address = socket.getaddrinfo(local_address,
                                           None,
                                           socket.AF_UNSPEC,
                                           socket.SOCK_STREAM,
                                           socket.IPPROTO_TCP,
                                           socket.AI_DEFAULT)[0][4][0]


def format_endpoint(hostname, port):
    return 'tcp://%s:%d' % (hostname, port)


def connect(local_sock, local_port, remote_sock, remote_port):
    local_sock.bind(format_endpoint(local_address, local_port))
    remote_sock.connect(format_endpoint(remote_address, remote_port))


def incoming_proxy(ctx):
    with ctx.socket(zmq.PULL) as local, ctx.socket(zmq.PUSH) as remote:
        # Block sends until connection completes
        remote.set(zmq.IMMEDIATE, True)

        connect(local, local_outgoing_port+1, remote, remote_outgoing_port+1)
        zmq.proxy(local, remote)


def outgoing_proxy(ctx):
    with ctx.socket(zmq.XPUB) as local, ctx.socket(zmq.XSUB) as remote:
        # No limit on number of outstanding outgoing messages
        local.set(zmq.SNDHWM, 0)

        # No limit on number of outstanding incoming messages
        remote.set(zmq.RCVHWM, 0)

        connect(local, local_outgoing_port, remote, remote_outgoing_port)
        zmq.proxy(remote, local)


def main():
    process_args()

    with zmq.Context() as ctx:
        ctx.setsockopt(zmq.IPV6, True)
        ctx.setsockopt(zmq.LINGER, False)

        # Handle incoming events on another thread
        def incoming_thread():
            try:
                incoming_proxy(ctx)
            except zmq.ContextTerminated:
                pass
        threading.Thread(target=incoming_thread).start()

        # Handle outgoing events on this thread
        try:
            outgoing_proxy(ctx)
        except KeyboardInterrupt:
            print()  # Add final newline


if __name__ == '__main__':
    main()
