#!/bin/sh

# start up an instance of the buildbot "slave" application, 
# which will run in the background and build the parts of the
# MW suite at the command of a buildbot master

SLAVE_DIRECTORY=~/.buildbot_slave
HOST="localhost"
PORT="7355"
ARCHITECTURE="x86_64-OSX-10.6"
USER=`whoami`
ADMIN_INFO="Dave <cox@rowland.harvard.edu>"
MACHINE_DESCRIPTION="MacBook Pro"

mkdir -p $SLAVE_DIRECTORY
rm -rf $SLAVE_DIRECTORY
mkdir -p $SLAVE_DIRECTORY
touch $SLAVE_DIRECTORY/twistd.log
buildbot create-slave $SLAVE_DIRECTORY $HOST:$PORT $ARCHITECTURE $USER
echo $ADMIN_INFO > $SLAVE_DIRECTORY/info/admin
echo $HOST" - "$MACHINE_DESCRIPTION" - "$ARCHITECTURE  > $SLAVE_DIRECTORY/info/host
buildbot start $SLAVE_DIRECTORY

