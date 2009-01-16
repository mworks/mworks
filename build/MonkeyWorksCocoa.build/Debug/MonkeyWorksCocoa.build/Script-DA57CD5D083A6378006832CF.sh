#!/bin/tcsh
# copy the framework to the proper directory
rm -dfR "/Library/Frameworks/MonkeyWorksCocoa.framework"
cp -Rf "$BUILT_PRODUCTS_DIR/MonkeyWorksCocoa.framework" "/Library/Frameworks"

