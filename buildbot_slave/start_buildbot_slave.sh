#!/bin/bash

# start up an instance of the buildbot "slave" application, 
# which will run in the background and build the parts of the
# MW suite at the command of a buildbot master

set -u

SLAVE_DIRECTORY=~/.buildbot_slave
MASTER_PORT="7355"
MAC_OSX_VERSION=$(sw_vers -productVersion | grep -Eo '^[0-9]+(\.[0-9]+)?')
SLAVE_NAME="x86_64-OSX-${MAC_OSX_VERSION}"

for config_file in local_config.sh slave_pass.sh; do
    if ! [ -f "$config_file" ]; then
	echo "ERROR: $config_file not found"
	exit 1
    fi
    . "$config_file"
done

if [[ $MASTER_HOST == $(hostname) ]]; then
    MASTER_HOST=localhost
fi

mkdir -p $SLAVE_DIRECTORY
rm -rf $SLAVE_DIRECTORY
mkdir -p $SLAVE_DIRECTORY
touch $SLAVE_DIRECTORY/twistd.log
buildbot create-slave --umask=022 $SLAVE_DIRECTORY $MASTER_HOST:$MASTER_PORT $SLAVE_NAME $SLAVE_PASS
echo $ADMIN_INFO > $SLAVE_DIRECTORY/info/admin
echo $(hostname)" - "$MACHINE_DESCRIPTION" - "$SLAVE_NAME  > $SLAVE_DIRECTORY/info/host
buildbot start $SLAVE_DIRECTORY
