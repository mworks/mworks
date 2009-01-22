#!/bin/zsh
export FULL_NAME=${PRODUCT_NAME}.${WRAPPER_EXTENSION}

rm -Rf "${INSTALL_PATH}/${FULL_NAME}"
cp -Rf "${BUILT_PRODUCTS_DIR}/${FULL_NAME}"  "${INSTALL_PATH}"

#mkdir -p /Library/MonkeyWorks/ExampleSetupVariables/Macbook
#cp -Rf $BUILT_PRODUCTS_DIR/../../Core/SetupVariables/setup_variables_macbook.xml /Library/#MonkeyWorks/ExampleSetupVariables/Macbook/setup_variables.xml
#mkdir -p /Library/MonkeyWorks/ExampleSetupVariables/TwoMonitors
#cp -Rf $BUILT_PRODUCTS_DIR/../../Core/SetupVariables/setup_variables_two_monitors.xml /Library/#MonkeyWorks/ExampleSetupVariables/TwoMonitors/setup_variables.xml

exit 0
