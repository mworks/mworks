# MW Build Infrastructure #

The mw_build repository contains tools to ease the process of building the full MonkeyWorks application suite, which consists of three applications, two frameworks, a flotilla of third-party supporting libraries, and a gaggle of plugins.  It also contains tools to make it easier to checkout everything, if you want to play with the code yourself.

The repository is organized into the following subsections:


### bootstrap ###

This directory contains bootstrap.py, which allows you to clone or pull all of the repositories contained in the MW project.  If you download or clone just this repository, you can easily bootstrap everything else with one command.

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

You need both a build "master" and at least one build "slave" to build the projects.  These are typically meant to run on separate machines, with the "master" being on a publicly available web server, and one or more slaves doing the actual heavy lifting.  To start a build master, you just need to run
	buildbot_master/start_buildbot_master.sh
Likewise for the slave, you'd run
	buildbot_slave/start_buildbot_slave.sh
	
If you get complaints about there being no command called "buildbot", you just need to install it by running:
	sudo easy_install buildbot

To stop any running buildbots on your machine:
	buildbot_master/stop_all_buildbot.sh

To start a local (e.g. running on localhost) buildbot pair (e.g. for testing), run:
	buildbot_master/start_all_buildbot_local.sh

The buildbot master server will then be running at [http://localhost:7349](http://localhost:7349).  You can examine build progress, manually kickstart builds, and more.

### installer ###

The scripts in this directory handle the automatic building of the MW installer. To build an installer, simply run:

	./build_installer_package.py VERSION_STRING
	
The resulting installer will be saved to:
	/tmp/mw_installer/MonkeyWorksInstaller_VERSION_STRING
