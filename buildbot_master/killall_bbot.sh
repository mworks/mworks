#!/bin/sh
ps -ef | grep buildbot | grep -v grep  | awk '{ print $2 }' | xargs kill
