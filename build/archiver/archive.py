#!/usr/bin/python
import time
import os
import shutil


now = time.gmtime()
archive_date = "%d-%d-%d" % (now[0], now[1], now[2])

archive_target_root = "/Users/%s/Desktop/MonkeyWorks Archive (%s)" % (os.getlogin(), archive_date)
os.makedirs(archive_target_root)


paths_to_archive = [# New style locations 
                    "/Library/Application Support/MonkeyWorks",  
                    "/Documents/MonkeyWorks",
                    "/Library/Frameworks/MonkeyWorksCore.framework",
                    "/Library/Frameworks/MonkeyWorksCocoa.framework",
                    # Assorted old-style locations
                    "/Library/MonkeyWorks",                     
                    "/Library/Frameworks/Scarab.framework",
                    "/Library/Application Support/NewClient",
                    "/Library/Application Support/MWClient",
                    "/Library/Application Support/NewEditor",
                    "/Library/Application Support/MWEditor"]

for src in paths_to_archive:
    dst = archive_target_root + src
    try:
        print ">> Copying contents of %s to %s" % (src, dst)
        shutil.copytree(src, dst)
    except:
        print ">> Did not find anything in %s" % src

print("A complete backup of your current MW install has been place in the directory: %s" % archive_target_root)