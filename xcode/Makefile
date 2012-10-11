RM_RF = /bin/rm -Rf

MW_APP_SUPPORT_DIR = /Library/Application\ Support/MWorks
MW_PLUGINS_DIR = $(MW_APP_SUPPORT_DIR)/Plugins/Core
XCODE_TEMPLATES_DIR = /Library/Application\ Support/Developer/Shared/Xcode/Project\ Templates

MW_XCODEBUILD = $(MW_APP_SUPPORT_DIR)/Developer/bin/mw_xcodebuild

all: install

install: clean
	$(RM_RF) $(XCODE_TEMPLATES_DIR)/MWorks
	/bin/mkdir -p $(XCODE_TEMPLATES_DIR)
	/usr/bin/rsync -a \
	  --exclude='*.pbxuser' \
	  --exclude='*.perspectivev3' \
	  --exclude='xcuserdata' \
	  --exclude='build' \
	  MWorks $(XCODE_TEMPLATES_DIR)

clean:
	$(RM_RF) $(MW_PLUGINS_DIR)/___PROJECTNAME___.bundle

test:
	cd "MWorks/MWorks Core Plugin" && $(MW_XCODEBUILD) Everything Development
