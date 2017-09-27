from __future__ import print_function, unicode_literals
import socket


in_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
in_sock.bind(('10.0.1.2', 4407))

out_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

while True:
    data, addr = in_sock.recvfrom(1024)
    out_sock.sendto(data, ('192.168.72.130', 4407))
