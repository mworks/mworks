REAL_MW_HOME = "/Library/Application Support/MonkeyWorks"
MW_HOME = MonkeyWorks
MW_DEVELOPER = $(MW_HOME)/Developer/gcc40
MW_TOOLS = $(MW_HOME)/Tools
INSTALL_DIR = $(MW_HOME)/Scripting/Matlab
MATLABHOME = $(INSTALL_DIR)/MATLAB
export MACI64 := 0
MEX        = $(MATLABHOME)/bin/mex
MEXSUFFIX  = mexmaci
CXX        = g++
CFLAGS     = -arch i386 -fno-common -no-cpp-precomp -fexceptions
LIBS      = -L$(MW_DEVELOPER)/lib -L$(MW_TOOLS)/lib -lscarab -ldfindex -lMonkeyWorksStreamUtilities -lboost_serialization-mw -lboost_system-mw -lboost_filesystem-mw
INCLUDE   = -I$(MATLABHOME)/extern/include -I$(MW_DEVELOPER)/include -I$(MW_TOOLS)/include
MEXFLAGS  = -cxx CC='$(CXX)' CXX='$(CXX)' LD='$(CXX)'

all: getEvents.$(MEXSUFFIX) getCodecs.$(MEXSUFFIX)
	mkdir -p $(INSTALL_DIR)
	cp getEvents.$(MEXSUFFIX) $(INSTALL_DIR)
	cp getCodecs.$(MEXSUFFIX) $(INSTALL_DIR)

clean:
	rm -f *.o *.$(MEXSUFFIX) $(INSTALL_DIR)/*.$(MEXSUFFIX) $(MW_HOME)

getEvents.$(MEXSUFFIX): mw-link getEvents.o
	$(MEX) $(MEXFLAGS) $(LIBS) -output getEvents getEvents.o

getEvents.o: getEvents.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $^

getCodecs.$(MEXSUFFIX): mw-link getCodecs.o
	$(MEX) $(MEXFLAGS) $(LIBS) -output getCodecs getCodecs.o

getCodecs.o: getCodecs.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $^

mw-link:
	ln -fs $(REAL_MW_HOME) $(MW_HOME)
