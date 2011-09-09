XCCONFIG_NAME = Development
include $(XCCONFIG_NAME).xcconfig

XCCONFIG_DIR = /Library/Application\ Support/MWorks/Developer/Xcode
XCCONFIG_FILES = *.xcconfig WARNING.txt
BIN_FILES = mw_xcodebuild

all: install

install: install-xcconfig install-bin

install-xcconfig: uninstall-xcconfig
	cp -p $(XCCONFIG_FILES) $(XCCONFIG_DIR)

install-bin: uninstall-bin
	cp -p $(BIN_FILES) $(MW_BIN_DIR)

uninstall: uninstall-xcconfig uninstall-bin

uninstall-xcconfig:
	mkdir -p $(XCCONFIG_DIR)
	cd $(XCCONFIG_DIR) && rm -f $(XCCONFIG_FILES)

uninstall-bin:
	mkdir -p $(MW_BIN_DIR)
	cd $(MW_BIN_DIR) && rm -f $(BIN_FILES)
