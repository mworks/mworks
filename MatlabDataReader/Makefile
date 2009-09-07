INSTALL_DIR = "/Library/Application Support/Matlab"
MEXSUFFIX  = mexmaci
MATLABHOME = /Applications/MATLAB74
MW_HOME = /Library/MonkeyWorks
MEX        = $(MATLABHOME)/bin/mex
CXX        = g++
CFLAGS     = -fno-common -no-cpp-precomp -fexceptions
LIBS      = -lscarab -ldfindex -L$(MW_HOME)/lib -lMonkeyWorksStreamUtilities -lboost_serialization-mt -lboost_system-mt -lboost_filesystem-mt
INCLUDE   = -I$(MATLABHOME)/extern/include -I$(MW_HOME)/include 
MEXFLAGS  = -cxx CC='$(CXX)' CXX='$(CXX)' LD='$(CXX)'

all: getEvents.$(MEXSUFFIX) getCodecs.$(MEXSUFFIX)
	mkdir -p $INSTALL_DIR;
	cp getEvents.$(MEXSUFFIX) $INSTALL_DIR/getEvents.$(MEXSUFFIX);
	cp getCodecs.$(MEXSUFFIX) $INSTALL_DIR/Matlab/getCodecs.$(MEXSUFFIX);

clean:
	rm -f *.o \
	*.$(MEXSUFFIX) \
	/Library/MonkeyWorks/Matlab/*.$(MEXSUFFIX)

getEvents.$(MEXSUFFIX): getEvents.o
	$(MEX) $(MEXFLAGS) $(LIBS) -output getEvents $^

getEvents.o: getEvents.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $^

getCodecs.$(MEXSUFFIX): getCodecs.o
	$(MEX) $(MEXFLAGS) $(LIBS) -output getCodecs $^

getCodecs.o: getCodecs.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $^

