SHELL = /bin/bash

SDK_ROOT = $(realpath /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform)
MACOSX_VERSION_QUOTED = $(shell sw_vers -productVersion | grep -Eo '^[0-9]+(\.[0-9]+)?' | sed 's/\./\\\./')

INSTALL_DIR = "$(APP_SUPPORT_DIR)/Scripting/Matlab"
MEX        = $(MATLAB_DIR)/bin/mex
CXX        = /usr/bin/g++
CFLAGS     = -arch i386 -arch x86_64 -fno-common -no-cpp-precomp -fexceptions
LIBS      = -L\"$(MW_DEVELOPER_DIR)/lib\" -lscarab -ldfindex -lMWorksMATLABTools -lboost_serialization -lboost_system -lboost_filesystem
INCLUDE   = -I$(MATLAB_DIR)/extern/include -I"$(MW_DEVELOPER_DIR)"/include
MEXFLAGS  = -cxx LD='$(CXX)' LDFLAGS='`echo "$$LDFLAGS" | sed "s/10\.5/$(MACOSX_VERSION_QUOTED)/g" | sed "s%-syslibroot,%-syslibroot,$(SDK_ROOT)%g"`'

SRCS = getCodecs.cpp getEvents.cpp
OBJS = $(SRCS:.cpp=.o)
EXES_32 = $(OBJS:.o=.mexmaci)
EXES_64 = $(OBJS:.o=.mexmaci64)
EXES = $(EXES_32) $(EXES_64)

all: clean_exes install

$(EXES_32): %.mexmaci: %.o
	MACI64=0 $(MEX) -maci $(MEXFLAGS) $(LIBS) -output $@ $<

$(EXES_64): %.mexmaci64: %.o
	MACI64=1 $(MEX) -maci64 $(MEXFLAGS) $(LIBS) -output $@ $<

$(OBJS): %.o: %.cpp MWorksMATLABTools.h MEXUtils.h
	$(CXX) $(CFLAGS) $(INCLUDE) -c $<

build: $(EXES)

install: build
	/bin/mkdir -p $(INSTALL_DIR)
	/bin/cp $(EXES) $(INSTALL_DIR)

clean_objs:
	/bin/rm -f $(OBJS)

clean_exes:
	/bin/rm -f $(EXES)

clean: clean_objs clean_exes
