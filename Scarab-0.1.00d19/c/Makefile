MEM_MODEL=std
OS_TYPE=unix
STREAMS=file tcpip
ENCODERS=ldo

CC=gcc
CFLAGS=-g -I.

AR=ar

LIBS=-lm

ENCODERS:=$(for i in "$(ENCODERS)"; do echo encoding_$i.o; done)
STREAMS:=$(for i in "$(STREAMS)"; do echo stream_$i.o; done)

HEADERS=\
	scarab.h \
	scarab_datatypes.h \
	scarab_dict.h \
	scarab_list.h \
	scarab_mem.h \
	scarab_os.h

OBJS=\
	os_$(OS_TYPE).o \
	mem_$(MEM_MODEL).o \
	comp.o \
	data.o \
	hashtable.o \
	list.o \
	session.o \
	encoding_ldo.o \
	stream_file.o \
	stream_tcpip.o

.c.o:
	$(CC) $(CFLAGS) -c $<

all: libscarab.O

scarab-test: scarab-test.o libscarab.O
	$(CC) -o scarab-test scarab-test.o libscarab.O $(LIBS)

libscarab.O: $(OBJS)
	ld -r -o libscarab.O $(OBJS)

libscarab.a: $(OBJS)
	$(AR) cr libscarab.a $(OBJS)

clean:
	rm -f *.o
	rm -f libscarab.O
	rm -f libscarab.a
	rm -f scarab-test


####################################################

# Test Code
scarab-test.o: scarab-test.c $(HEADERS)

# Generic Library
comp.o: comp.c $(HEADERS)
data.o: data.c $(HEADERS)
hashtable.o: hashtable.c $(HEADERS)
list.o: list.c $(HEADERS)
session.o: session.c $(HEADERS)

# Encoders
encoding_ldo.o: encoding_ldo.c encoding_ldo.h $(HEADERS)

# Operating Systems
os_unix.o: os_unix.c $(HEADERS)

# Memory Models
mem_coptic.o: mem_coptic.c $(HEADERS)
mem_nt.o: mem_nt.c $(HEADERS)
mem_std.o: mem_std.c $(HEADERS)

# Streams
stream_file.o: stream_file.c $(HEADERS)
stream_tcpip.o: stream_tcpip.c $(HEADERS)
