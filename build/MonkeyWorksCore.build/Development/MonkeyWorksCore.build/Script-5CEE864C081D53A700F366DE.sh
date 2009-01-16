#!/bin/tcsh
rm -Rf $INSTALL_PATH/MonkeyWorksCore.framework
cp -Rf "$BUILT_PRODUCTS_DIR/MonkeyWorksCore.framework" $INSTALL_PATH

mkdir -p /Library/MonkeyWorks/ExampleSetupVariables/Macbook
cp -Rf $BUILT_PRODUCTS_DIR/../../Core/SetupVariables/setup_variables_macbook.xml /Library/MonkeyWorks/ExampleSetupVariables/Macbook/setup_variables.xml
mkdir -p /Library/MonkeyWorks/ExampleSetupVariables/TwoMonitors
cp -Rf $BUILT_PRODUCTS_DIR/../../Core/SetupVariables/setup_variables_two_monitors.xml /Library/MonkeyWorks/ExampleSetupVariables/TwoMonitors/setup_variables.xml

exit 0
