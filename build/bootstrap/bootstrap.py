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
    print("To tag all repositories with a given tag type:")
    print("\t%s tag <tag_id>" % sys.argv[0])
    sys.exit()

command = sys.argv[1]

repositories = ("mw_build",
                "mw_client",
                "mw_cocoa",
                "mw_core",
                "mw_datatools",
                "mw_editor",
                "mw_examples",
                "mw_scarab",
                "mw_server",
                "mw_supporting",
                "mw_client_plugins",
                "mw_core_plugins",
                "mw_xcode_templates")

github_url = "git://github.com/mworks-project"

home_directory = os.path.expanduser('~')
repository_path = "%s/Repositories/MWorks" % home_directory

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

    if command == 'tag':
        if(len(sys.argv) < 3):
            print("To tag all repositories with a given tag type:")
            print("\t%s tag <tag_id>" % sys.argv[0])
            sys.exit()
        
        tag_id = sys.argv[2]
        os.chdir("%s/%s" % (repository_path, repo))
        command_string = "git tag %s; git push --tags" % tag_id
        print("%s (%s): \n>>> %s" % (repo, os.getcwd(), command_string))
        os.system(command_string)
        print("\n")
