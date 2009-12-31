#!/usr/bin/env python

# Builds an installer package (e.g. *.pkg) based on the currently installed version on the system
# This could be used to "replicate" a given development workstation's environment, or it could
# be used by a nightly buildbot to package the results of a fresh build
#
# TODO: add a mechanism for dynamically including plugin installers (which would be "checkable" options
# in the meta-installer)
#
# TODO: remove shell commands and replace with Python os calls
import os
import sys

if(len(sys.argv) != 3):
    raise Exception('This script takes a two arguments, specifying the installer name and version string')

installer_name = sys.argv[1]
version_string = sys.argv[2]

# definitions
package_maker = "/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker"
project_id = "edu.harvard.rowland"

def quote(str):
    return "\"" + str + "\""

def build_package(root, version, target_path, output_path):
    print("Building: %s" % pkg)    
    cmd = "%s --root %s/ --id %s --version %s --install-to %s --out %s" % (package_maker, root, project_id, version, target_path, output_path)
    print(cmd)
    os.system(cmd)


def build_metapackage(doc, version, output_path, output_name):
    print("Building: %s" % pkg)
    filename =  output_name + "_" + version + ".pkg"
    full_output_path = output_path + "/" + filename
    cmd = "%s --doc %s --id %s --version %s  --out %s" % (package_maker, doc, project_id, version, full_output_path)
    print(cmd)
    os.system(cmd)
    os.system("cd %s; zip -r %s %s" % (output_path, filename +".zip", filename))

# directories
install_root="/tmp/mw_installer"

# Important directories in the live system
mw_applications_dir = "/Applications"
mw_application_support_dir = "/Library/Application Support/MonkeyWorks"
mw_frameworks_dir = "/Library/Frameworks"
mw_configuration_dir = mw_application_support_dir + "/Configuration"
mw_developer_dir = mw_application_support_dir + "/Developer"
mw_documents_dir = "/Documents/MonkeyWorks"

mw_installer_info_file = "mw_installer.info"
mw_installer_info_path = mw_configuration_dir + "/" + mw_installer_info_file

mw_subcomponent_package_path = install_root + "/subcomponents"

# Package roots for various sub-project installers
applications_package_root = install_root + "/applications"
application_support_package_root = install_root + "/application_support"
frameworks_package_root = install_root + "/frameworks"
developer_package_root = install_root + "/developer"
documents_package_root = install_root + "/documents"

package_roots = [applications_package_root,
                 application_support_package_root,
                 frameworks_package_root,
                 developer_package_root,
                 documents_package_root]

for root in package_roots:
    os.system("chmod -R 777 %s" % root)

# Package names
packages = ["applications",
            "application_support",
            "frameworks",
            "developer",
            "documents"]


# Applications
applications = ["MWEditor.app", "MWServer.app", "MWClient.app"]

# Frameworks
frameworks = ["MonkeyWorksCocoa.framework", "MonkeyWorksCore.framework", "Scarab.framework"]


# Remove the current install root
os.system("rm -rf %s" % quote(install_root))

# Create the package root directories
for pkg_root in package_roots:
    os.system("mkdir -p %s" % quote(pkg_root))


# Copy stuff into the appropriate directories

# applications
applications_install_dir = applications_package_root + mw_applications_dir
os.system("mkdir -p %s" % quote(applications_install_dir))
for app in applications:
    os.system("cp -rf %s %s" % (quote(mw_applications_dir + "/" + app), quote(applications_install_dir)))

# application support
application_support_install_dir = application_support_package_root + mw_application_support_dir
os.system("mkdir -p %s" % quote(application_support_install_dir))
os.system("rsync -a --exclude Developer --exclude setup_variables.xml %s %s" % (quote(mw_application_support_dir + "/"), 
                                                   quote(application_support_install_dir)))
os.system("mkdir -p %s" % quote(application_support_install_dir + "/Experiment Cache"))  # make sure this directory exists

# frameworks
frameworks_install_dir = frameworks_package_root + mw_frameworks_dir
os.system("mkdir -p %s" % quote(frameworks_install_dir))
for fw in frameworks:
    os.system("cp -rf %s %s" % (quote(mw_frameworks_dir + "/" + fw), quote(frameworks_install_dir)))

# developer support
developer_install_dir = developer_package_root + mw_developer_dir
os.system("mkdir -p %s" % quote(developer_install_dir))
print "cp -rf %s %s" % (quote(mw_developer_dir + "/"), quote(developer_install_dir))
os.system("cp -rf %s %s" % (quote(mw_developer_dir + "/"), quote(developer_install_dir)))

# documents
documents_install_dir = documents_package_root + mw_documents_dir
os.system("mkdir -p %s" % quote(documents_install_dir))
print "cp -rf %s %s" % (quote(mw_documents_dir + "/"), quote(documents_install_dir))
os.system("cp -rf %s %s" % (quote(mw_documents_dir + "/"), quote(documents_install_dir)))

os.system("mkdir -p %s" % quote(mw_subcomponent_package_path))

mw_version = version_string

print "Automatic package building is currently disabled.  Please use the PackageMaker GUI to build the full installer"
os.system("open /Developer/Applications/Utilities/PackageMaker.app")
# for pkg in packages:
#     build_package(install_root + "/" + pkg , mw_version, "/", "%s.pkg" % (pkg))
#     os.system("mv %s.pkg %s/" % (pkg, quote(mw_subcomponent_package_path + "/")))
# 
# build_metapackage(os.path.dirname(__file__) + "/mw_installer.pmdoc", mw_version, install_root, installer_name)
