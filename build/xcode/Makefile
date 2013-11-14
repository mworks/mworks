CP_P = /bin/cp -p
MKDIR_P = /bin/mkdir -p
RM_F = /bin/rm -f

MW_DEVELOPER_DIR = /Library/Application\ Support/MWorks/Developer
MW_XCODE_DIR = $(MW_DEVELOPER_DIR)/Xcode

XCCONFIG_FILES = *.xcconfig WARNING.txt

all: install

install: install-xcconfig

install-xcconfig: uninstall-xcconfig
	$(CP_P) $(XCCONFIG_FILES) $(MW_XCODE_DIR)

uninstall: uninstall-xcconfig

uninstall-xcconfig:
	$(MKDIR_P) $(MW_XCODE_DIR)
	cd $(MW_XCODE_DIR) && $(RM_F) $(XCCONFIG_FILES)
