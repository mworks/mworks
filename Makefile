XCCONFIG_PATH = /Library/Application\ Support/MWorks/Developer/Xcode
XCCONFIG_NAME = Development
include $(XCCONFIG_PATH)/$(XCCONFIG_NAME).xcconfig

TEST_XML_DIR = "$(TESTS_DIR)/XML"

all: install

install: install-examples install-tests

install-examples: clean-examples
	mkdir -p $(EXAMPLES_DIR)
	cp -Rp Examples/ Tests/ExampleExperiments/ $(EXAMPLES_DIR)

install-tests: clean-tests
	mkdir -p $(TEST_XML_DIR)
	cp -Rp Tests/ $(TEST_XML_DIR)

clean: clean-examples clean-tests

clean-examples:
	rm -Rf $(EXAMPLES_DIR)

clean-tests:
	rm -Rf $(TEST_XML_DIR)
