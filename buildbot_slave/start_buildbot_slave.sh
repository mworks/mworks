#!/bin/sh

# start up an instance of the buildbot "slave" application, 
# which will run in the background and build the parts of the
# MW suite at the command of a buildbot master

SLAVE_DIRECTORY=~/.buildbot_slave
HOST=monkeyworks.coxlab.org
PORT="7355"
ARCHITECTURE="i386-OSX-10.5"

# ----------------------------------------------------------------------------------------------
# TODO: These must be edited to include the username and password assigned to your build slave
USER=$ARCHITECTURE
PASS=yastapimrb
echo "-----------------------------------------------------------------------------------"
echo "You must edit the buildbot slave username and password before you can use this file"
echo "-----------------------------------------------------------------------------------"
#exit
# ----------------------------------------------------------------------------------------------

ADMIN_INFO="Dave <cox@rowland.harvard.edu>"
MACHINE_DESCRIPTION="MacBook Pro"

mkdir -p $SLAVE_DIRECTORY
rm -rf $SLAVE_DIRECTORY
mkdir -p $SLAVE_DIRECTORY
touch $SLAVE_DIRECTORY/twistd.log
buildbot create-slave $SLAVE_DIRECTORY $HOST:$PORT $USER $PASS
echo $ADMIN_INFO > $SLAVE_DIRECTORY/info/admin
echo $HOST" - "$MACHINE_DESCRIPTION" - "$ARCHITECTURE  > $SLAVE_DIRECTORY/info/host
buildbot start $SLAVE_DIRECTORY

