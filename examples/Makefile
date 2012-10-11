CP_RP = /bin/cp -Rp
MKDIR_P = /bin/mkdir -p
RM_RF = /bin/rm -Rf

MW_APP_SUPPORT_DIR = /Library/Application\ Support/MWorks
EXAMPLES_DIR = $(MW_APP_SUPPORT_DIR)/Examples
TEST_XML_DIR = $(MW_APP_SUPPORT_DIR)/Developer/tests/XML

all: install

install: install-examples install-tests

install-examples: uninstall-examples
	$(MKDIR_P) $(EXAMPLES_DIR)
	$(CP_RP) Examples/ $(EXAMPLES_DIR)

install-tests: uninstall-tests
	$(MKDIR_P) $(TEST_XML_DIR)
	$(CP_RP) Tests/ $(TEST_XML_DIR)

uninstall: uninstall-examples uninstall-tests

uninstall-examples:
	$(RM_RF) $(EXAMPLES_DIR)

uninstall-tests:
	$(RM_RF) $(TEST_XML_DIR)
