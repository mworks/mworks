CP_P = /bin/cp -p
MKDIR_P = /bin/mkdir -p
RM_F = /bin/rm -f

MW_DEVELOPER_DIR = /Library/Application\ Support/MWorks/Developer
MW_XCCONFIG_DIR = $(MW_DEVELOPER_DIR)/Xcode
MW_BIN_DIR = $(MW_DEVELOPER_DIR)/bin

XCCONFIG_FILES = *.xcconfig WARNING.txt
BIN_FILES = mw_xcodebuild

all: install

install: install-xcconfig install-bin

install-xcconfig: uninstall-xcconfig
	$(CP_P) $(XCCONFIG_FILES) $(MW_XCCONFIG_DIR)

install-bin: uninstall-bin
	$(CP_P) $(BIN_FILES) $(MW_BIN_DIR)

uninstall: uninstall-xcconfig uninstall-bin

uninstall-xcconfig:
	$(MKDIR_P) $(MW_XCCONFIG_DIR)
	cd $(MW_XCCONFIG_DIR) && $(RM_F) $(XCCONFIG_FILES)

uninstall-bin:
	$(MKDIR_P) $(MW_BIN_DIR)
	cd $(MW_BIN_DIR) && $(RM_F) $(BIN_FILES)
