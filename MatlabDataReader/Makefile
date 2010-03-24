SHELL = /bin/bash

REAL_MW_HOME = "/Library/Application Support/MonkeyWorks"
MW_HOME = MonkeyWorks
MW_DEVELOPER = $(MW_HOME)/Developer/gcc40
MW_TOOLS = $(MW_HOME)/Tools
INSTALL_DIR = $(MW_HOME)/Scripting/Matlab
MATLABHOME = /Applications/MATLAB
MEX        = $(MATLABHOME)/bin/mex
CXX        = g++
CFLAGS     = -arch i386 -arch x86_64 -fno-common -no-cpp-precomp -fexceptions
LIBS      = -L$(MW_DEVELOPER)/lib -L$(MW_TOOLS)/lib -lscarab -ldfindex -lMonkeyWorksStreamUtilities -lboost_serialization-mw -lboost_system-mw -lboost_filesystem-mw
INCLUDE   = -I$(MATLABHOME)/extern/include -I$(MW_DEVELOPER)/include -I$(MW_TOOLS)/include
MEXFLAGS  = -cxx CC='$(CXX)' CXX='$(CXX)' LD='$(CXX)'

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

$(OBJS): %.o: %.cpp $(MW_HOME)
	$(CXX) $(CFLAGS) $(INCLUDE) -c $<

$(MW_HOME):
	ln -fs $(REAL_MW_HOME) $(MW_HOME)

clean:
	rm -f $(EXES) $(OBJS) $(MW_HOME)

install: all
	mkdir -p $(INSTALL_DIR)
	cp $(EXES) $(INSTALL_DIR)
