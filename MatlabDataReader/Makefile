REAL_MW_HOME = "/Library/Application Support/MonkeyWorks"
MW_HOME = MonkeyWorks
MW_DEVELOPER = $(MW_HOME)/Developer/gcc40
MW_TOOLS = $(MW_HOME)/Tools
INSTALL_DIR = $(MW_HOME)/Scripting/Matlab
MATLABHOME = /Applications/MATLAB
MEX        = $(MATLABHOME)/bin/mex
MEXSUFFIX  = mexmaci
CXX        = g++
CFLAGS     = -arch i386 -fno-common -no-cpp-precomp -fexceptions
LIBS      = -L$(MW_DEVELOPER)/lib -L$(MW_TOOLS)/lib -lscarab -ldfindex -lMonkeyWorksStreamUtilities -lboost_serialization-mw -lboost_system-mw -lboost_filesystem-mw
INCLUDE   = -I$(MATLABHOME)/extern/include -I$(MW_DEVELOPER)/include -I$(MW_TOOLS)/include
MEXFLAGS  = -cxx CC='$(CXX)' CXX='$(CXX)' LD='$(CXX)'

# Prevent mex from building 64-bit binaries
export MACI64 := 0

SRCS = getCodecs.cpp getEvents.cpp
OBJS = $(SRCS:.cpp=.o)
EXES = $(OBJS:.o=.$(MEXSUFFIX))

all: $(EXES)

$(EXES): %.$(MEXSUFFIX): %.o
	$(MEX) $(MEXFLAGS) $(LIBS) -output $@ $<

$(OBJS): %.o: %.cpp $(MW_HOME)
	$(CXX) $(CFLAGS) $(INCLUDE) -c $<

$(MW_HOME):
	ln -fs $(REAL_MW_HOME) $(MW_HOME)

clean:
	rm -f $(EXES) $(OBJS) $(MW_HOME)

install: all
	mkdir -p $(INSTALL_DIR)
	cp $(EXES) $(INSTALL_DIR)
