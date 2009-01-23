#!/bin/sh

cd ../buildbot_master
./start_buildbot_master.sh

cd ../buildbot_slave
./start_buildbot_slave_local.sh

