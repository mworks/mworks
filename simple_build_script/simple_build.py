#!/usr/bin/env python
import os

def build_xcode_project(base_path, relative_path, configuration, target):
    path = os.path.join(base_path, relative_path)
    print "Moving to %s" % path
    os.chdir(path)
    
    os.system("xcodebuild clean -alltargets -configuration %s" % (configuration))
    os.system("xcodebuild build -target %s -configuration %s" % (target, configuration))


# expects to be called from a path containing all of the other repositories
base_path = os.path.abspath(os.curdir)
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
            "coxlab/coxlab_mwcore_plugins/Phidgets"
            ]
             
for p in projects:
    build_xcode_project(base_path, p, standard_config, standard_target)
    