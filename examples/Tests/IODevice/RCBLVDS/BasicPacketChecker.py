from __future__ import print_function, unicode_literals
from collections import namedtuple
import socket
import struct


MAX_PACKET_SIZE = 1480

header_fields = (
    'magic', 'B',
    'sod', 'B',
    'mac', '6s',
    'sn', 'I',
    '', '4x',  # padding
    'reserved', 'Q',
    'spiBitRate', 'I',
    'chanMask', 'I',
    'auxMask', 'B',
    'auxPhase', 'B',
    'numTs', 'H',
    'vbat', 'H',
    'digIn', 'H',
    )

header = namedtuple('header', ' '.join(header_fields[0::2]))
header_unpacker = struct.Struct('<' + ''.join(header_fields[1::2]))

expected_digital_inputs = [
    0b0100001000010101,
    0b0100001000010100,
    0b0100001000010101,
    0b0100001000010011,
    0b0100001000011011,
    0b0100001111101011,
    0b1011110111101011,
    0b0000000000000000,
    ]

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
try:
    sock.bind(('10.0.1.2', 4407))
    digital_inputs = 0
    current_expected_digital_input = 0
    while True:
        data, addr = sock.recvfrom(MAX_PACKET_SIZE)
        h = header._make(header_unpacker.unpack(data[:header_unpacker.size]))
        if digital_inputs != h.digIn:
            digital_inputs = h.digIn
            expected = expected_digital_inputs[current_expected_digital_input]
            if expected == digital_inputs:
                print('OK: {0:016b}'.format(digital_inputs))
            else:
                print('ERROR:')
                print('  Expected: {0:016b}'.format(expected))
                print('  Received: {0:016b}'.format(digital_inputs))
            current_expected_digital_input += 1
            current_expected_digital_input %= len(expected_digital_inputs)
except KeyboardInterrupt:
    print()  # Add final newline
finally:
    sock.close()
