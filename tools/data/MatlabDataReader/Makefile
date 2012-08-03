XCCONFIG_DIR = /Library/Application\ Support/MWorks/Developer/Xcode
XCCONFIG_NAME = Development
include $(XCCONFIG_DIR)/$(XCCONFIG_NAME).xcconfig

SHELL = /bin/bash

CXX_ROOT = $(realpath /Applications/Xcode.app/Contents)
SDK_ROOT = $(realpath /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform)
MACOSX_VERSION_QUOTED = $(shell sw_vers -productVersion | grep -Eo '^[0-9]+(\.[0-9]+)?' | sed 's/\./\\\./')

INSTALL_DIR = "$(APP_SUPPORT_DIR)/Scripting/Matlab"
MATLABHOME = /Applications/MATLAB
MEX        = $(MATLABHOME)/bin/mex
CXX        = $(CXX_ROOT)/Developer/usr/bin/g++
CFLAGS     = -arch i386 -arch x86_64 -fno-common -no-cpp-precomp -fexceptions
LIBS      = -L\"$(MW_DEVELOPER_DIR)/lib\" -lscarab -ldfindex -lMWorksStreamUtilities -lboost_serialization -lboost_system -lboost_filesystem
INCLUDE   = -I$(MATLABHOME)/extern/include -I"$(MW_DEVELOPER_DIR)"/include
MEXFLAGS  = -cxx LD='$(CXX)' LDFLAGS='`echo "$$LDFLAGS" | sed "s/10\.5/$(MACOSX_VERSION_QUOTED)/g" | sed "s%-syslibroot,%-syslibroot,$(SDK_ROOT)%g"`'

SRCS = getCodecs.cpp getEvents.cpp
OBJS = $(SRCS:.cpp=.o)
EXES_32 = $(OBJS:.o=.mexmaci)
EXES_64 = $(OBJS:.o=.mexmaci64)
EXES = $(EXES_32) $(EXES_64)

all: $(EXES)

$(EXES_32): %.mexmaci: %.o
	MACI64=0 $(MEX) -maci $(MEXFLAGS) $(LIBS) -output $@ $<

$(EXES_64): %.mexmaci64: %.o
	MACI64=1 $(MEX) -maci64 $(MEXFLAGS) $(LIBS) -output $@ $<

$(OBJS): %.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $<

clean:
	rm -f $(EXES) $(OBJS)

install: all
	mkdir -p $(INSTALL_DIR)
	cp $(EXES) $(INSTALL_DIR)
