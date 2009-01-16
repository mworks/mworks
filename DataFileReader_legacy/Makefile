DEBUG_FLAG=-g
#DEBUG_FLAG=-O3

LD_FLAGS_ML=-lmx -lmat -L/Applications/MATLAB74/bin/maci 
LD_FLAGS=-lscarab -L/Library/MonkeyWorks/lib
CC_FLAGS=-F/Library/Frameworks -framework MonkeyWorksCore -I/Library/MonkeyWorks/include
CC_FLAGS_ML=-I/Applications/MATLAB74/extern/include


all: mwdfr mw2ml
	mkdir -p /Library/MonkeyWorks/tools;
	cp mwdfr /Library/MonkeyWorks/tools/mwdfr;
	cp mw2ml /Library/MonkeyWorks/tools/mw2ml;


mw2ml:	mw2ml.cpp monkeyWorksStreamUtilities.o
	g++ ${DEBUG_FLAG} ${CC_FLAGS} ${CC_FLAGS_ML} ${LD_FLAGS_ML} ${LD_FLAGS} \
	monkeyWorksStreamUtilities.o -o mw2ml mw2ml.cpp 

mwdfr:	mwdfr.cpp 
	g++ ${DEBUG_FLAG} ${CC_FLAGS} ${LD_FLAGS} \
	-o mwdfr mwdfr.cpp

monkeyWorksStreamUtilities.o:	monkeyWorksStreamUtilities.h \
				monkeyWorksStreamUtilities.cpp 
				g++ ${DEBUG_FLAG} ${CC_FLAGS} ${CC_FLAGS_ML} ${LD_FLAGS} \
				-c monkeyWorksStreamUtilities.cpp

clean:
	rm -f \
	mw2ml \
	mwdfr \
	*~ \
	*.o \
	*# \
	gdb_history \
	/Library/MonkeyWorks/tools/mwdfr \
	/Library/MonkeyWorks/tools/mw2ml  
