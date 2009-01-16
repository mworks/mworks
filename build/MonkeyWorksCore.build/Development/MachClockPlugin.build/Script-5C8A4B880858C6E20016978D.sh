#!/bin/tcsh
rm -rf $INSTALL_PATH/MachClockPlugin.bundle
cp -R "$BUILT_PRODUCTS_DIR/MachClockPlugin.bundle" $INSTALL_PATH
