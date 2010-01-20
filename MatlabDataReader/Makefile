MW_HOME = "/Library/Application Support/MonkeyWorks"
MW_DEVELOPER = $(MW_HOME)/Developer/gcc40
INSTALL_DIR = $(MW_HOME)/Scripting/Matlab
MATLABHOME = $(INSTALL_DIR)/MATLAB
MEX        = $(MATLABHOME)/bin/mex
MEXSUFFIX  = mexmaci
CXX        = g++
CFLAGS     = -fno-common -no-cpp-precomp -fexceptions
LIBS      = -lscarab -ldfindex -L$(MW_DEVELOPER)/lib -lMonkeyWorksStreamUtilities -lboost_serialization-mt -lboost_system-mt -lboost_filesystem-mt
INCLUDE   = -I$(MATLABHOME)/extern/include -I$(MW_DEVELOPER)/include 
MEXFLAGS  = -cxx CC='$(CXX)' CXX='$(CXX)' LD='$(CXX)'

all: getEvents.$(MEXSUFFIX) getCodecs.$(MEXSUFFIX)
	mkdir -p $(INSTALL_DIR);
	cp getEvents.$(MEXSUFFIX) $(INSTALL_DIR)/getEvents.$(MEXSUFFIX);
	cp getCodecs.$(MEXSUFFIX) $(INSTALL_DIR)/Matlab/getCodecs.$(MEXSUFFIX);

clean:
	rm -f *.o \
	*.$(MEXSUFFIX) \
	$(INSTALL_DIR)/*.$(MEXSUFFIX)

getEvents.$(MEXSUFFIX): getEvents.o
	$(MEX) $(MEXFLAGS) $(LIBS) -output getEvents $^

getEvents.o: getEvents.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $^

getCodecs.$(MEXSUFFIX): getCodecs.o
	$(MEX) $(MEXFLAGS) $(LIBS) -output getCodecs $^

getCodecs.o: getCodecs.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $^

