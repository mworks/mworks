#!/bin/sh

# start up an instance of the buildbot "master" application, 
# which will run in the background and coordinate build activities
# on networked build "slave" instances

MASTER_DIRECTORY=~/.buildbot_master
SCRIPT_DIRECTORY=`dirname $0`

if ps x | grep "\.buildbot_master" | grep -v "grep"; # NB: this signature must match MASTER_DIRECTORY, above
then
    echo "Buildbot master already started"
    exit
fi

for config_file in local_config.py slave_pass.py; do
    if ! [ -f "$SCRIPT_DIRECTORY/${config_file}" ]; then
	echo "ERROR: ${config_file} not found"
	exit 1
    fi
done

rm -rf $MASTER_DIRECTORY
mkdir -p $MASTER_DIRECTORY
touch $MASTER_DIRECTORY/twistd.log
buildbot create-master $MASTER_DIRECTORY
cp $SCRIPT_DIRECTORY/master.cfg $MASTER_DIRECTORY
cp $SCRIPT_DIRECTORY/local_config.py $MASTER_DIRECTORY
cp $SCRIPT_DIRECTORY/slave_pass.py $MASTER_DIRECTORY
cp -r $SCRIPT_DIRECTORY/plugins $MASTER_DIRECTORY/
mkdir -p $MASTER_DIRECTORY/public_html
cp $SCRIPT_DIRECTORY/*.css $MASTER_DIRECTORY/public_html/
cp $SCRIPT_DIRECTORY/*.html $MASTER_DIRECTORY/public_html/

buildbot start $MASTER_DIRECTORY
