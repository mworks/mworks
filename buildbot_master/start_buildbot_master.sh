#!/bin/sh

# start up an instance of the buildbot "master" application, 
# which will run in the background and coordinate build activities
# on networked build "slave" instances

MASTER_DIRECTORY=~/.buildbot_master

rm -rf $MASTER_DIRECTORY
mkdir -p $MASTER_DIRECTORY
touch $MASTER_DIRECTORY/twistd.log
buildbot create-master $MASTER_DIRECTORY
cp ./master.cfg $MASTER_DIRECTORY
cp -r ./plugins $MASTER_DIRECTORY/
cp ./buildbot.css $MASTER_DIRECTORY/public_html/

buildbot start $MASTER_DIRECTORY
