XCODE_CLEAN = xcodebuild clean -alltargets -configuration $(1)
XCODE_BUILD = xcodebuild build -target $(1) -configuration $(2)

all: df-indexer df-reader matlab-reader mw-stream-utils

df-indexer:
	cd DataFileIndexer && \
	$(call XCODE_CLEAN,"Development (10.5 Compatible)") && \
	$(call XCODE_BUILD,dfindex,"Development (10.5 Compatible)") && \
	$(call XCODE_BUILD,"Python Bindings","Development (10.5 Compatible)")

df-reader:
	cd DataFileReader && \
	$(call XCODE_CLEAN,Debug) && \
	$(call XCODE_BUILD,mwdfr,Debug)

matlab-reader: mw-stream-utils
	cd MatlabDataReader && \
	make clean && \
	make install

mw-stream-utils:
	cd MWorksStreamUtilities && \
	$(call XCODE_CLEAN,Debug) && \
	$(call XCODE_BUILD,libMWorksStreamUtilities,Debug)
