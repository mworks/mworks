#!/usr/bin/env python
# This is a simple script that builds some or all of the MW projects.  
# It is not meant to be a proper build system, just a quicky for building
# all or a subset of the MW suite.  For a proper build system, see the buildbot
# build system one directory up.

import os
import sys


print("Warning: this is not a proper build system, just a quicky script for doing a few automated builds")
print("You will need to customize it for your purposes if you intend to use it.")

if len(sys.argv) != 2:
    raise Exception("This script takes one argument, specifying the base directory where all of the MW repositories can be found")

def build_xcode_project(base_path, relative_path, configuration, target):
    path = os.path.join(base_path, relative_path)
    print "Moving to %s" % path
    os.chdir(path)
    
    os.system("xcodebuild clean -alltargets -configuration %s" % (configuration))
    os.system("xcodebuild build -target %s -configuration %s" % (target, configuration))



base_path = sys.argv[1]
standard_config = "\"Development (10.5 Compatible)\""
standard_target = "Everything"



projects = [ 
            "mw_scarab",
            "mw_core",
            "mw_cocoa",
            "mw_server",
            "mw_client",
            "mw_client_plugins/BehavioralWindow",
            "mw_client_plugins/PythonBridgePlugin",
            "mw_client_plugins/VariablesWindow",
            "mw_core_plugins/CircleStimulus",
            "mw_core_plugins/DriftingGratingStimulus",
            "mw_core_plugins/NE500",
            "mw_core_plugins/RectangleStimulus",
            "mw_core_plugins/SidewinderPlugAndPlayGamepadPlugin",
            "mw_editor",
            "coxlab/coxlab_mwclient_plugins/CoxLabRatBehaviorControlPanel",
            "coxlab/coxlab_mwcore_plugins/CobraTracker",
            "coxlab/coxlab_mwcore_plugins/Phidgets",
            "coxlab/mwMaskStimulus"
            ]
             
for p in projects:
    build_xcode_project(base_path, p, standard_config, standard_target)
    