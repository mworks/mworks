#!/usr/bin/env python
# boostrap.py <command>
# clone:
# 	clone all repositories to the current directory
#
# pull:
#	pull for all repositories

import os
import sys

if(len(sys.argv) < 2):
	print("Usage: to setup for the first time (i.e. to clone the repositories) type:")
	print("\t%s clone" % sys.argv[0])
	print("To pull changes from the central repository to your computer type:")
	print("\t%s pull" % sys.argv[0])
	sys.exit()

command = sys.argv[1]

repositories = ("acqboard",
				"backplane",
				"cnetevent",
				"crc-hw",
				"doctools",
				"dspboard",
				"eproc",
				"hdltools",
				"libsomanetwork",
				"logging",
				"network",
				"pynet",
				"serial-deviceio",
				"software-common",
				"tspikes")
				
github_url = "git://github.com/somaproject"

home_directory = os.path.expanduser('~')
repository_path = "%s/Repositories/monkeyworks" % home_directory 

if not os.path.exists(repository_path):
	os.system("mkdir -p " + repository_path)

os.chdir("%s" % repository_path)

    

for repo in repositories:
	if command == "clone":
		command_string = "git clone %s/%s.git" % (github_url, repo)
		print("%s (current directory = %s): \n>>> %s" % (repo, os.getcwd(), command_string))
		os.system(command_string)
		print("\n")
		
	if command == "pull":
		os.chdir("%s/%s" % (repository_path, repo))
		command_string = "git pull"
		print("%s (%s): \n>>> %s" % (repo, os.getcwd(), command_string))
		os.system(command_string)
		print("\n")

	if command == "status":
		os.chdir("%s/%s" % (repository_path, repo))
		command_string = "git status"
		print("%s (%s): \n>>> %s" % (repo, os.getcwd(), command_string))
		os.system(command_string)
		print("\n")