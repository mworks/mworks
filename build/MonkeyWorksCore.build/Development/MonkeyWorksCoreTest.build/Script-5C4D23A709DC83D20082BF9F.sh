#!/bin/sh
mkdir -p /Library/MonkeyWorks/tests
mkdir -p /Library/MonkeyWorks/tests/XML
rm -Rf /Library/MonkeyWorks/tests/MonkeyWorksCoreTest.framework
cp -Rf "$BUILT_PRODUCTS_DIR/MonkeyWorksCoreTest.framework" /Library/MonkeyWorks/tests/
exit 0

