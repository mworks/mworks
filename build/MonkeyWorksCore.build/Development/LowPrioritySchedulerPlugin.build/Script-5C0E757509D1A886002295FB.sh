#!/bin/tcsh
rm -R -f ${INSTALL_PATH}/LowPrioritySchedulerPlugin.bundle
cp -R -f "${BUILT_PRODUCTS_DIR}/LowPrioritySchedulerPlugin.bundle" ${INSTALL_PATH}

