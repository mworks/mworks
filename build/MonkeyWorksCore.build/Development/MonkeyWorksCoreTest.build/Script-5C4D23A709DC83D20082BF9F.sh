#!/bin/zsh
export FULL_NAME=${PRODUCT_NAME}.${WRAPPER_EXTENSION}

echo making dir ${TESTS_DIR}/XML
mkdir -p "${TESTS_DIR}/XML"
rm -Rf "${TESTS_DIR}/${FULL_NAME}"
echo Copying files from ${BUILT_PRODUCTS_DIR}/${FULL_NAME} to ${TESTS_DIR}/
cp -Rf "${BUILT_PRODUCTS_DIR}/${FULL_NAME}" "${TESTS_DIR}/"
exit 0

