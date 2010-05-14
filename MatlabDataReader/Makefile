SHELL = /bin/bash

REAL_MW_HOME = "/Library/Application Support/MWorks"
MW_HOME = MWorks
MW_DEVELOPER = $(MW_HOME)/Developer
INSTALL_DIR = $(MW_HOME)/Scripting/Matlab
MATLABHOME = /Applications/MATLAB
MEX        = $(MATLABHOME)/bin/mex
CXX        = /usr/bin/g++-4.2
CFLAGS     = -arch i386 -arch x86_64 -fno-common -no-cpp-precomp -fexceptions
LIBS      = -L$(MW_DEVELOPER)/lib -lscarab -ldfindex -lMWorksStreamUtilities -lboost_serialization-mw -lboost_system-mw -lboost_filesystem-mw
INCLUDE   = -I$(MATLABHOME)/extern/include -I$(MW_DEVELOPER)/include
MEXFLAGS  = -cxx LD='$(CXX)' LDFLAGS='`echo "$$LDFLAGS" | sed "s/10\.5/10\.6/g"`'

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
