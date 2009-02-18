# mw_build #

The mw_build repository contains tools to ease the process of building the full MonkeyWorks application suite, which consists of three applications, two frameworks, a flotilla of third-party supporting libraries, and a gaggle of plugins.  It also contains tools to make it easier to checkout everything, if you want to play with the code yourself.

The repository is organized into the following subsections:


### bootstrap ###

This directory contains bootstrap.py, which allows you to clone or pull all of the repositories contained in the MW project.

To clone all projects, type:
	./bootstrap.py clone
	
To pull from all projects (assuming you've already cloned):
	./bootstrap.py pull
	
The script, by default, puts everything in the following directory:
	~/Repositories/monkeyworks/*
If you'd like to put stuff somewhere else, the script is easily edited.

### buildbot_master & buildbot_slave ###

These directories contain the necessary parts to run the MW buildbot setup.  Buildbot is an automated build system that coordinates the building of complicated projects like MW.  When installed properly, it automatically builds and tests MW every night, and will also build after new commits are made, to ensure that things haven't been broken by an error in a given commit.  For more info on BuildBot, see [this site](http://buildbot.net).

The Cox Lab maintains a public build master for MW [here](http://monkeyworks.coxlab.org).  If you'd like to setup a build slave, please contact monkeyworks@coxlab.org.

### installer ###

The scripts in this directory handle the automatic building of the MW installer.