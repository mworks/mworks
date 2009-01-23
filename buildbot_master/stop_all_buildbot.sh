#!/bin/sh

SLAVE_DIR=~/.buildbot_slave
MASTER_DIR=~/.buildbot_master

buildbot stop $MASTER_DIR
buildbot stop $SLAVE_DIR
